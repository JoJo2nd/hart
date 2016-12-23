/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#include "hart/base/thread.h"
#include "hart/base/debug.h"
#include "hart/base/threadlocalstorage.h"

namespace hart {

Thread::Thread() : threadFunc(nullptr) {}

Thread::~Thread() {
  delete threadFunc;
}

void Thread::create(const char* threadName, int32_t priority, Function pFunctor, void* param) {
  memcpy(threadName_, threadName, THREAD_NAME_SIZE);
  threadFunc = new Function(pFunctor);
  pThreadParam_ = param;
  priority_ = priority;
  if (priority_ < -2) {
    priority_ = -2;
  }
  if (priority_ > 2) {
    priority_ = 2;
  }
  ThreadHand_ = CreateThread(NULL, (1024 * 1024) * 2, staticFunc, this, 0, NULL);
}


void Thread::SetThreadName(LPCSTR szThreadName) {
#pragma pack(push, 8)
  typedef struct tagTHREADNAME_INFO {
    DWORD  dwType;     // must be 0x1000
    LPCSTR szName;     // pointer to name (in user addr space)
    DWORD  dwThreadID; // thread ID (-1=caller thread)
    DWORD  dwFlags;    // reserved for future use, must be zero
  } THREADNAME_INFO;
#pragma pack(pop)
  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = szThreadName;
  info.dwThreadID = -1; // caller thread
  info.dwFlags = 0;

  __try {
    RaiseException(0x406D1388, 0, sizeof(info) / sizeof(DWORD), (ULONG_PTR*)&info);
  } __except (EXCEPTION_CONTINUE_EXECUTION) {
  }
}

unsigned long WINAPI Thread::staticFunc(LPVOID pParam) {
  Thread* local_this = (Thread*)pParam;
  SetThreadName(local_this->threadName_);
  hprofile_namethread(local_this->threadName_);
  SetThreadPriority(local_this->ThreadHand_, local_this->priority_);
  local_this->returnCode_ = (*local_this->threadFunc)(local_this->pThreadParam_);
  htls::threadExit();
  return local_this->returnCode_;
}
}
