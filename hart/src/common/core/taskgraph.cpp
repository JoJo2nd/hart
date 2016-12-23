/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/core/taskgraph.h"
#include "hart/base/std.h"
#include "hart/base/crt.h"
#include "hart/lfds/lfds.h"
#include <algorithm>


namespace hart {
namespace tasks {

hstd::vector<lfds_queue_state*>         workerInputQueues;
hstd::vector<hstd::unique_ptr<hThread>> workerThreads;
lfds_queue_state*                       graphInputQueue;
hSemaphore                              schedulerSemphore;
hSemaphore                              schedulerKillSemphore;
hSemaphore                              workerSemphore;
hThread                                 schedulerThread;
hatomic::aint32_t                       taskToComplete;

uint32_t TaskHandle::postWaitingCompleted() {
  return owner->internalNotifyWaitingJob(*this);
}

TaskHandle Graph::addTask(char const* name, TaskProc const& proc) {
  hdbassert(!isRunning(), "Cannot add task while a task graph is running.");
  tasks.emplace_back(this, name, proc);
  TaskHandle handle;
  handle.owner = this;
  handle.firstTaskIndex = (uint32_t)tasks.size() - 1;
  handle.lastTaskIndex = (uint32_t)tasks.size() - 1;
  return handle;
}

void Graph::addTaskInput(TaskHandle handle, void* in_taskinput) {
  hdbassert(handle.owner == this && handle.firstTaskIndex < tasks.size(), "Task does not belong to this task graph");
  tasks[handle.firstTaskIndex].taskInputs.push_back(in_taskinput);
  hatomic::liteMemoryBarrier();
}

TaskHandle Graph::findTaskByName(char const* task_name) {
  TaskHandle handle;
  handle.owner = this;
  for (size_t i = 0, n = tasks.size(); i < n; ++i) {
    if (tasks[i].taskName == task_name) {
      handle.firstTaskIndex = (uint32_t)i;
      handle.lastTaskIndex = (uint32_t)i;
      break;
    }
  }
  return handle;
}

void Graph::clearTaskInputs(TaskHandle handle) {
  tasks[handle.firstTaskIndex].taskInputs.clear();
}

void Graph::createTaskDependency(TaskHandle first, TaskHandle second) {
  hdbassert(first.owner == this && first.firstTaskIndex < tasks.size(),
            "First task does not belong to this task graph");
  hdbassert(second.owner == this && second.firstTaskIndex < tasks.size(),
            "Second task does not belong to this task graph");
  if (std::find(tasks[first.firstTaskIndex].dependentTasks.begin(), tasks[first.firstTaskIndex].dependentTasks.end(),
                second) == tasks[first.firstTaskIndex].dependentTasks.end()) {
    tasks[first.firstTaskIndex].dependentTasks.push_back(second);
    ++tasks[second.firstTaskIndex].initialWaitingTaskCount;
  }
}

void Graph::clear() {
  tasks.clear();
}

void Graph::kick() {
  for (auto& i : tasks) {
    i.completed = &jobsWaiting;
    hatomic::atomicSet(i.currentWaitingTaskCount, i.initialWaitingTaskCount);
    hatomic::atomicSet(i.started, 0);
    hatomic::atomicSet(i.finished, 0);
    if (i.initialWaitingTaskCount == 0)
      hatomic::atomicSet(i.toSend, hutil::tmax((int32_t)i.taskInputs.size(), 1));
    else // As this task waits on others, set this value to something that'll ensure it's never sent to a worker. Allows
         // for
      hatomic::atomicSet(i.toSend, -1); // an earlier task to set up tasks for a later task.
    i.inFlight = 0;
  }
  hatomic::atomicSet(running, 1);
  hatomic::atomicSet(jobsWaiting, (uint32_t)tasks.size());
  lfds_queue_enqueue(graphInputQueue, this);
  schedulerSemphore.Post();
}

Task* Graph::getNextAvaibleJob() {
  for (size_t i = 0, n = tasks.size(); i < n; ++i) {
    if (hatomic::atomicGet(tasks[i].currentWaitingTaskCount) == 0) {
      int32_t toSend = hatomic::atomicGet(tasks[i].toSend);
      hdbassert((int32_t)tasks[i].inFlight <= toSend || toSend < 0, "Pending jobs count is too high.");
      if ((int32_t)tasks[i].inFlight < toSend) {
        ++tasks[i].inFlight;
        return &tasks[i];
      }
    }
  }
  return nullptr;
}

// There are probably better ways to handle this but it'll do for now
static uint32_t schedulerProcess(void* null) {
  std::vector<Graph*> graphsToProcess;
  std::vector<Task*>  tasksToQueue;
  hatomic::atomicSet(taskToComplete, 0);
  lfds_queue_use(graphInputQueue);
  lfds_queue_use(workerInputQueues[0]);
  while (!schedulerKillSemphore.poll()) {
    schedulerSemphore.Wait();
    // Check the input queue for new graphs to process. (i.e. Graph.kick() called)
    Graph* graph_ptr;
    while (lfds_queue_dequeue(graphInputQueue, (void**)&graph_ptr)) {
      for (auto& i : graphsToProcess) {
        if (!i) {
          i = graph_ptr;
          graph_ptr = nullptr;
          break;
        }
      }
      if (graph_ptr) graphsToProcess.push_back(graph_ptr);
    }

    // if (!graphsToProcess.empty() && !hatomic::atomicGet(taskToComplete)) {
    //    hatomic::atomicSet(taskToComplete, 1);
    //}

    for (auto& i : graphsToProcess) {
      if (i && i->internalAllJobsDone()) {
        i->internalPostComplete();
        i = nullptr;
      }
    }

    // if (graphsToProcess.empty() && hatomic::atomicGet(taskToComplete)) {
    //    hatomic::atomicSet(taskToComplete, 0);
    //}

    for (auto& i : graphsToProcess) {
      if (!i) continue;
      while (auto* t = i->getNextAvaibleJob()) {
        lfds_queue_enqueue(workerInputQueues[0], (void*)t); // tasksToQueue.push_back(t);
        workerSemphore.Post();
      }
    }

    /*
    for (auto& i : tasksToQueue) {
        for (auto& q : workerInputQueues) {
            //
            lfds_queue_enqueue(q, (void*)i);
        }
    }
    tasksToQueue.clear();
    */
  }
  return 0;
}

static uint32_t workerProcess(void* workerQueuePtr) {
  lfds_queue_state* workerQueue = (lfds_queue_state*)workerQueuePtr;
  lfds_queue_use(workerQueue);
  while (1) {
    workerSemphore.Wait();
    // Grab an task and run it
    Task* task = nullptr;
    if (lfds_queue_dequeue(workerQueue, (void**)&task)) {
      hprofile_start_str(task->taskName.c_str());
      auto task_index = hatomic::increment(task->started);
      hdbassert(task_index <= hatomic::atomicGet(task->toSend),
                "task_index is invalid. To high compared to number of tasks expected to run.");
      Info info;
      info.owningGraph = task->owner;
      info.taskInput = task->taskInputs.empty() ? nullptr : task->taskInputs[task_index - 1];
      if (HART_DEBUG_TASK_ORDER)
        hdbprintf("Starting task %s on Thread [Unknown]", task->taskName.c_str() /*, getCurrentThreadID()*/);
      task->work(&info);
      if (HART_DEBUG_TASK_ORDER)
        hdbprintf("Ending task %s on Thread [Unknown]", task->taskName.c_str() /*, getCurrentThreadID()*/);
      bool wakeScheduler = false;
      auto complete_index = hatomic::increment(task->finished);
      if (complete_index ==
          hatomic::atomicGet(task->toSend)) { // Was this the last task of this type that needed to be run.
        if (HART_DEBUG_TASK_ORDER)
          hdbprintf("Waking dependent tasks for %s on Thread [Unknown]",
                    task->taskName.c_str() /*, getCurrentThreadID()*/);
        for (auto& i : task->dependentTasks) {
          if (i.postWaitingCompleted() == 0) {
            wakeScheduler = true;
          }
        }
        if (hatomic::decrement(*task->completed) == 0) wakeScheduler = true;
      }
      if (wakeScheduler) schedulerSemphore.Post();
      hprofile_end();
    }
  }
  return 0;
}

namespace scheduler {
bool initialise(int32_t worker_count, uint32_t job_queue_size) {
  uint32_t processor_count = worker_count <= 0 ? 4 : worker_count;

  workerInputQueues.resize(1);
  workerThreads.resize(processor_count);
  workerSemphore.Create(0, processor_count);
  schedulerSemphore.Create(0, 128);
  schedulerKillSemphore.Create(0, 1);
  hatomic::atomicSet(taskToComplete, 0);
  lfds_queue_new(&graphInputQueue, job_queue_size);
  lfds_queue_new(&workerInputQueues[0], job_queue_size);
  for (uint32_t i = 0; i < processor_count; ++i) {
    char name[128];
    hcrt::sprintf(name, sizeof(name), "Worker Thread %d", i + 1);
    workerThreads[i].reset(new hThread());
    workerThreads[i]->create(name, 0, workerProcess, workerInputQueues[0]);
  }

  schedulerThread.create("Task Graph Scheduler", 0, schedulerProcess, nullptr);

  return true;
}

void destroy() {
  schedulerKillSemphore.Post();
  schedulerThread.join();
}
}
}
}
