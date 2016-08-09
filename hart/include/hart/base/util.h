#pragma once
namespace hart {
namespace util {

template <typename t_ty, size_t n> 
constexpr size_t arraySize(const t_ty (&)[n]) { return n; }

}
}
namespace hutil = hart::util;

#define HART_ARRAYSIZE(x) hutil::arraySize(x)
#define HART_MAKE_FOURCC(a,b,c,d) ((uint32_t)(uint8_t)(a) | ((uint32_t)(uint8_t)(b) << 8) | ((uint32_t)(uint8_t)(c) << 16) | ((uint32_t)(uint8_t)(d) << 24 ))