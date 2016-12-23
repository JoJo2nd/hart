/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"
#include "hart/base/debug.h"
#include "hart/windows.inc"

namespace hart {
class Semaphore {
public:
  bool Create(uint32_t initCount, uint32_t maxCount) {
    sema = CreateSemaphore(NULL, initCount, maxCount, NULL);
    hdbassert(sema != INVALID_HANDLE_VALUE, "CreateSemaphore Failed");
    return sema != INVALID_HANDLE_VALUE;
  }
  void Wait() { WaitForSingleObject(sema, INFINITE); }
  bool poll() {
    DWORD ret = WaitForSingleObject(sema, 0);
    return ret == WAIT_OBJECT_0 ? true : false;
  }
  void Post() {
    auto r = ReleaseSemaphore(sema, 1, NULL);
    hdbassert(r, "ReleaseSemaphore Failed");
  }
  void Destroy() { CloseHandle(sema); }

private:
  HANDLE sema;
};
}

typedef hart::Semaphore hSemaphore;
