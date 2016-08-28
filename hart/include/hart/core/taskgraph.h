/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"
#include "hart/base/mutex.h"
#include "hart/base/thread.h"
#include "hart/base/semaphore.h"
#include "hart/base/std.h"
#include "hart/base/atomic.h"
#include "hart/base/util.h"
namespace hart {
namespace tasks {

namespace scheduler {    
    bool initialise(int32_t worker_count, uint32_t job_queue_size);
    void destroy();
}

    class Graph;
    struct Info {
        // todo... some info
        Graph* owningGraph = nullptr;
        void* taskInput = nullptr;
    };

    typedef hstd::function<void(Info*)> TaskProc;

    class TaskHandle {
        Graph* owner = nullptr;
        uint32_t firstTaskIndex = -1;
        uint32_t lastTaskIndex = -1; // Unused. maybe for future use...
        friend class Graph;
    public:
        uint32_t postWaitingCompleted();
        bool isValid() const { return firstTaskIndex != -1; }
        void reset() {
            firstTaskIndex = -1;
            lastTaskIndex = -1;
        }

        bool operator == (const TaskHandle& rhs) const {
            return owner == rhs.owner && firstTaskIndex == rhs.firstTaskIndex;
        }
    };

    struct Task {
        Task(Graph* in_owner, const char* name, TaskProc const& proc) 
            : owner(in_owner)
            , taskName(name)
            , work(proc) {
        }
        Task(const Task&& rhs) {
            work = std::move(rhs.work);
            taskName = rhs.taskName;
            inFlight = rhs.inFlight;
            initialWaitingTaskCount = rhs.initialWaitingTaskCount;
            hatomic::atomicSet(currentWaitingTaskCount, hatomic::atomicGet(rhs.currentWaitingTaskCount));
            hatomic::atomicSet(started, hatomic::atomicGet(rhs.started));
            hatomic::atomicSet(finished, hatomic::atomicGet(finished));
            hatomic::atomicSet(toSend, hatomic::atomicGet(rhs.toSend));
            completed = rhs.completed;
            dependentTasks = std::move(rhs.dependentTasks);
            taskInputs = std::move(rhs.taskInputs);
            owner = rhs.owner;
        }
        TaskProc work;
        hstd::string taskName;
        uint32_t inFlight = 0; // How many of this task have we queued?
        uint32_t initialWaitingTaskCount = 0; // the number of tasks that need to complete before this can be entered into the task queue.
        hatomic::aint32_t currentWaitingTaskCount; //
        hatomic::aint32_t started; //
        hatomic::aint32_t finished; //
        hatomic::aint32_t toSend; // How many of this task should we queue? normally max(taskInputs.size(), 1), atomic to allow to be update mid graph
        hatomic::aint32_t* completed; // set when a worker finishes the job and has notified the dependent tasks.
        Graph* owner = nullptr;
        hstd::vector<TaskHandle> dependentTasks; // list of tasks waiting on our completion
        hstd::vector<void*> taskInputs; //if not empty task will be run once, otherwise run [taskInputs.size()] times
    };

    class Graph {
        hstd::vector<Task> tasks;
        hSemaphore  graphComplete;
        hatomic::aint32_t  running;
        hatomic::aint32_t  jobsWaiting;
    public:
        Graph() {
            graphComplete.Create(0, 1);
            hatomic::atomicSet(running, 0);
            hatomic::atomicSet(jobsWaiting, 0);
        }
        TaskHandle addTask(const char* task_name, TaskProc const& proc);
        TaskHandle findTaskByName(const char* task_name);
        void addTaskInput(TaskHandle handle, void* in_taskinput);
        void clearTaskInputs(TaskHandle handle);
        void createTaskDependency(TaskHandle first, TaskHandle second);
        void clear();
        void kick();
        void wait() {
            graphComplete.Wait();
            hatomic::atomicSet(running, 0);
        }
        bool isRunning() const { return !!hatomic::atomicGet(running); }
        bool isComplete() const { return !isRunning(); }
        Task* getNextAvaibleJob();
        bool internalAllJobsDone() const { return hatomic::atomicGet(jobsWaiting) == 0; }
        void internalPostComplete() { graphComplete.Post(); hatomic::atomicSet(running, 0); }
        uint32_t internalNotifyWaitingJob(const TaskHandle& handle) {
            auto ret = hatomic::decrement(tasks[handle.firstTaskIndex].currentWaitingTaskCount);
            if (ret == 0) {
                hatomic::atomicSet(tasks[handle.firstTaskIndex].toSend, hutil::tmax((int32_t)tasks[handle.firstTaskIndex].taskInputs.size(), 1));
            }
            return ret;
        }
    };

}
}

namespace htasks = hart::tasks;
