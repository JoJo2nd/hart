/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "hart/base/atomic.h"

#if (HART_PLATFORM == HART_PLATFORM_WINDOWS)
#include <winsock2.h>
#include <windows.h>
#define mem_barrier() MemoryBarrier()
#elif (HART_PLATFORM == HART_PLATFORM_LINUX)
#define mem_barrier() __sync_synchronize()
#else
#error("Platform not supported")
#endif

namespace hart {
namespace atomic {
int32_t increment(aint32_t& i) {
  return ++i;
}

int32_t decrement(aint32_t& i) {
  return --i;
}

int32_t compareAndSwap(aint32_t& val, int32_t compare, int32_t newVal) {
  int32_t l_compare = compare;
  int32_t l_new_val = newVal;
  return val.compare_exchange_strong(l_compare, l_new_val) ? compare : newVal;
}

void liteMemoryBarrier() {
  mem_barrier();
}

void heavyMemoryBarrier() {
  mem_barrier();
}

int32_t atomicSet(aint32_t& i, int32_t val) {
  i.store(val);
  return val;
}

int32_t atomicGet(const aint32_t& i) {
  return i.load();
}

int32_t atomicAdd(aint32_t& i, int32_t amount) {
  return i.fetch_add(amount) + amount;
}
}
}