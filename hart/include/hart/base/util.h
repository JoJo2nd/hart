/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once
namespace hart {
namespace util {

template <typename t_ty, size_t n> 
constexpr size_t arraySize(const t_ty (&)[n]) { return n; }

inline uint16_t endianSwap(uint16_t v) {
    return ((v&0x00FF) << 8) | ((v&0xFF00) >> 8);
}

inline uint32_t endianSwap(uint32_t v) {
    return ((uint32_t)endianSwap(uint16_t(v&0x0000FFFF))<< 16) | ((uint32_t)endianSwap(uint16_t((v&0xFFFF0000)>>16)));
}

inline uint64_t endianSwap(uint64_t v) {
    return ((uint64_t)endianSwap(uint32_t(v&0x00000000FFFFFFFF)) << 32) | ((uint64_t)endianSwap(uint32_t((v&0xFFFFFFFF00000000)>>32)));
}

template< typename t_ty >
inline t_ty tmin(t_ty a, t_ty b) {
    return a < b ? a : b;
}

template< typename t_ty >
inline t_ty tmax(t_ty a, t_ty b) {
    return a > b ? a : b;
}

}
}
namespace hutil = hart::util;

#define HART_ARRAYSIZE(x) hutil::arraySize(x)
#define HART_MAKE_FOURCC(a,b,c,d) ((uint32_t)(uint8_t)(a) | ((uint32_t)(uint8_t)(b) << 8) | ((uint32_t)(uint8_t)(c) << 16) | ((uint32_t)(uint8_t)(d) << 24 ))
