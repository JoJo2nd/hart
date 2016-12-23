/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"
#include "hart/base/std.h"
#include "hart/windows.inc"

namespace hart {

class Thread {
public:
  typedef hstd::function<int32_t(void*)> Function;

  Thread();
  Thread(const Thread& rhs) = delete;
  Thread& operator==(const Thread& rhs) = delete;
  ~Thread();

  enum Priority {
    PRIORITY_LOWEST = -2,
    PRIORITY_BELOWNORMAL = -1,
    PRIORITY_NORMAL = 0,
    PRIORITY_ABOVENORMAL = 1,
    PRIORITY_HIGH = 2,
  };

  void create(const char* threadName, int32_t priority, Function pFunctor, void* param);
  int32_t returnCode() { return returnCode_; }
  void    join() { WaitForSingleObject(ThreadHand_, INFINITE); }

private:
  static const int THREAD_NAME_SIZE = 32;

  static void SetThreadName(LPCSTR szThreadName);
  static unsigned long WINAPI staticFunc(LPVOID pParam);

  char      threadName_[THREAD_NAME_SIZE];
  void*     pThreadParam_;
  Function* threadFunc;
  HANDLE    ThreadHand_;
  int32_t   priority_;
  int32_t   returnCode_;
};
}

typedef hart::Thread hThread;
