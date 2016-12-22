/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"
#include <atomic>

namespace hart {
namespace atomic {

typedef std::atomic<int32_t>  aint32_t;
typedef std::atomic<uint32_t> auint32_t;
typedef std::atomic<int64_t>  aint64_t;
typedef std::atomic<uint64_t> auint64_t;

int32_t increment(aint32_t& i);
int32_t decrement(aint32_t& i);
int32_t compareAndSwap(aint32_t& val, int32_t compare, int32_t newVal);
int32_t atomicSet(aint32_t& i, int32_t val);
int32_t atomicGet(const aint32_t& i);
int32_t atomicAdd(aint32_t& i, int32_t amount);
int32_t atomicAddWithPrev(aint32_t& i, int32_t amount, int32_t* prev);
void liteMemoryBarrier();
void heavyMemoryBarrier();
}
}

namespace hatomic = hart::atomic;
