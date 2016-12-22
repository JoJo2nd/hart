/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"
#include "hart/windows.inc"

namespace hart {

class Mutex {
public:
  Mutex() { InitializeCriticalSection(&mutex_); }
  void lock() { EnterCriticalSection(&mutex_); }
  bool tryLock() {
    BOOL ret = TryEnterCriticalSection(&mutex_);
    return !!ret;
  }
  void unlock() { LeaveCriticalSection(&mutex_); }
  ~Mutex() { DeleteCriticalSection(&mutex_); }

  _RTL_CRITICAL_SECTION mutex_;
};

class ScopedMutex {
public:
  ScopedMutex(Mutex* in_mtx) : mtx(in_mtx) { mtx->lock(); }
  ~ScopedMutex() { mtx->unlock(); }

  ScopedMutex& operator=(ScopedMutex const& rhs) = delete;
  ScopedMutex(ScopedMutex const& rhs) = delete;

private:
  Mutex* mtx;
};
}

typedef hart::Mutex       hMutex;
typedef hart::ScopedMutex hScopedMutex;
