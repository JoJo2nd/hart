/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#pragma once

//valid platforms
#define HART_PLATFORM_WINDOWS   (0x1001)
#define HART_PLATFORM_LINUX     (0x1002)

#define HART_PLATFORM           (0) // defined to whatever platform we are building for
#define HART_DEBUG              (0) // 1 for a debug build
#define HERT_RELEASE            (0) // 1 for a release build
#define HART_DEBUG_INFO         (0) // enable debug features and aids
#define HART_ENABLE_PROFILE     (1) // enable Remotery profiling and logging
#define HART_DO_ASSERTS         (0) // enable asserts
#define HART_ENABLE_STDIO       (0) // enable output to stdio
#define HART_64BIT              (0) // 1 for a 64bit build
#define HART_32BIT              (0) // 1 for a 32bit build
#define HART_API                    // calling convention for callbacks
#define HART_VERIFY_FREELIST    (1)
#define HART_DEFAULT_WND_WIDTH  (1280)
#define HART_DEFAULT_WND_HEIGHT (720)
#define HART_MAX_PATH           (1024)


#if defined (_WIN32) || defined (_WIN64)
#   undef HART_PLATFORM
#   define HART_PLATFORM (HART_PLATFORM_WINDOWS)
#else
#   error "Unable to determine platform"
#endif

#if defined(__x86_64__)    || \
    defined(_M_X64)        || \
    defined(__aarch64__)   || \
    defined(__64BIT__)     || \
    defined(__mips64)      || \
    defined(__powerpc64__) || \
    defined(__ppc64__)
#   undef  HART_64BIT
#   define HART_64BIT 64
#else
#   undef  HART_32BIT
#   define HART_32BIT 32
#endif //

#if defined (_DEBUG) || defined (DEBUG) || defined (CMAKE_DEBUG_BUILD)
#   undef HART_DEBUG
#   define HART_DEBUG (1)
#else
#   undef HART_RELEASE
#   define HART_RELEASE (1)
#endif

#if defined (HART_DEBUG) || defined (CMAKE_RELWITHDEBINFO_BUILD)
#   undef HART_DEBUG_INFO
#	define HART_DEBUG_INFO (1)
#endif

#if HART_DEBUG_INFO
#   undef HART_DO_ASSERTS
#   define HART_DO_ASSERTS (1)
#   undef HART_ENABLE_STDIO
#   define HART_ENABLE_STDIO (1)
#endif

#if HART_DEBUG_INFO
#   define HART_DEBUG_TASK_ORDER (0)
#endif

#if !HART_DEBUG_INFO
#   undef HART_VERIFY_FREELIST
#   define HART_VERIFY_FREELIST (0)
#endif

#if (HART_PLATFORM == HART_PLATFORM_WINDOWS)
#   undef HART_API
#   define HART_API   __cdecl
#elif (HART_PLATFORM == HART_PLATFORM_LINUX)
#	if HART_32BIT
#       undef HART_API
#	    define HART_API   __attribute__((__cdecl__))
#	endif
#else
#   error "Platform not supported"
#endif

//TODO: detect endian

/*
    FVN-1a
    hash = offset_basis
    for each octet_of_data to be hashed
            hash = hash xor octet_of_data
            hash = hash * FNV_prime
    return hash

    32 bit FNV_prime = 224 + 28 + 0x93 = 16777619
    64 bit FNV_prime = 240 + 28 + 0xb3 = 1099511628211

    32 bit offset_basis = 2166136261
    64 bit offset_basis = 14695981039346656037
*/
#if HART_64BIT
#   define HART_FVN_OFFSET_BASIS (14695981039346656037)
#   define HART_FVN_PRIME (1099511628211)
#elif HART_32BIT
#   define HART_FVN_OFFSET_BASIS (2166136261)
#   define HART_FVN_PRIME (16777619)
#endif

#if (HART_PLATFORM == HART_PLATFORM_WINDOWS)
#   define SDL_MAIN_HANDLED
#endif

#define BGFX_HANDLE_SET_INVALID(x) (x.idx = bgfx::invalidHandle)

#include <stdint.h>
