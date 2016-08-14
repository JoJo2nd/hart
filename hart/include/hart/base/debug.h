/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"
#if (HART_PLATFORM == HART_PLATFORM_LINUX)
#   include <signal.h>
#   include <stdlib.h>
#endif

namespace hart {
namespace debug {
/*
outputdebugstring
assertMsgFunc
*/
}
}


#if (HART_PLATFORM == HART_PLATFORM_LINUX)
#   define __noop(...)
#endif

#if HART_DO_ASSERTS

#define hdbprintf(fmt, ...)         ::printf(fmt, ##__VA_ARGS__ ) /*outputdebugstring*/
#define hdbcondprintf(x, y, ...) if (x) { hdbprintf(y, ##__VA_ARGS__ ); }

#define hdbassert( x, y,...)	{ static bool ignore = false; \
                                if (!(x)) {\
                                uint32_t ret = 1;/*assertMsgFunc(ignore, __FILE__"(%u) Assert Failed: ("#x ")\n" #y, __LINE__, ##__VA_ARGS__);*/\
                                if (ret == 0) exit(-1);\
                                if (ret == 1) hdbbreak;\
                                if (ret == 2) ignore = true; }}
#define hdbfatal( y, ... )      hdbassert(false, y, ##__VA_ARGS__)

#if (HART_PLATFORM == HART_PLATFORM_WINDOWS)
#   if HART_64BIT
#       define hdbbreak __debugbreak()
#   else
#       define hdbbreak __asm { int 3 }
#   endif
#elif  (HART_PLATFORM == HART_PLATFORM_LINUX)
#   define hdbbreak raise(SIGTRAP)
#else
#   error "Unkonwn platform"
#endif

#else
//#else

#define hdbprintf(fmt, ...)
#define hdbcondprintf(x, y, ...)
#define hdbassert( x, y,...)
#define hdbfatal( y, ... )
#define hdbbreak 

#endif
