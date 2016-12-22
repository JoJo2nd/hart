/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/base/crt.h"
#include "hart/config.h"
#if (HART_PLATFORM == HART_PLATFORM_LINUX)
#include <signal.h>
#include <stdlib.h>
#endif
#if HART_ENABLE_PROFILE
#include "Remotery.h"
#endif

#if (HART_PLATFORM == HART_PLATFORM_LINUX)
#define __noop(...)
#endif

#if HART_DO_ASSERTS

#define hdbprintf(fmt, ...) ::debug::outputdebugstring(fmt, ##__VA_ARGS__)
#define hdbcondprintf(x, y, ...)                                                                                       \
  if (x) {                                                                                                             \
    hdbprintf(y, ##__VA_ARGS__);                                                                                       \
  }

#define hdbassert(x, y, ...)                                                                                           \
  {                                                                                                                    \
    static bool ignore = false;                                                                                        \
    if (!(x)) {                                                                                                        \
      uint32_t ret = 1; /*assertMsgFunc(ignore, __FILE__"(%u) Assert Failed: \ \                                                                                                                     \
                           ("#x ")\n" #y, __LINE__, ##__VA_ARGS__);*/                                                  \
      hdbprintf(__FILE__ "(%u) Assert Failed: (" #x ")\n" #y, __LINE__, ##__VA_ARGS__);                                \
      hdbbreak;                                                                                                        \
                                /*if (ret == 0) exit(-1);\
                                if (ret == 1) hdbbreak;\
                                if (ret == 2) ignore = true; */}                                                       \
  }
#define hdbfatal(y, ...) hdbassert(false, y, ##__VA_ARGS__)

#if (HART_PLATFORM == HART_PLATFORM_WINDOWS)
#if HART_64BIT
#define hdbbreak __debugbreak()
#else
#define hdbbreak __asm { int 3}
#endif
#elif (HART_PLATFORM == HART_PLATFORM_LINUX)
#define hdbbreak raise(SIGTRAP)
#else
#error "Unkonwn platform"
#endif

#else

#define hdbprintf(fmt, ...) __noop
#define hdbcondprintf(x, y, ...) __noop
#define hdbassert(x, y, ...) __noop
#define hdbfatal(y, ...) __noop
#define hdbbreak __noop

#endif

#if HART_ENABLE_PROFILE

#define hprofile_startup()                                                                                             \
  Remotery* rmt;                                                                                                       \
  rmt_CreateGlobalInstance(&rmt)
#define hprofile_shutdown() rmt_DestroyGlobalInstance(rmt)
#define hprofile_namethread(tname) rmt_SetCurrentThreadName(tname)
#define hprofile_log(str) rmt_LogText(str)
#define hprofile_start(str) rmt_BeginCPUSample(str, 0)
#define hprofile_scope(str) rmt_ScopedCPUSample(str, 0)
#define hprofile_start_str(str) rmt_BeginCPUSampleDynamic(str, 0)
#define hprofile_end() rmt_EndCPUSample()

#else

#define hprofile_startup()
#define hprofile_shutdown()
#define hprofile_namethread(tname)
#define hprofile_log(str)
#define hprofile_start(str)
#define hprofile_scope(str)
#define hprofile_start_str(str)
#define hprofile_end()

#endif

namespace debug {

inline void outputdebugstring(char const* fmt_str, ...) {
  va_list args;
  va_start(args, fmt_str);
#if HART_ENABLE_PROFILE
  char tmp_buffer[1024];
  hcrt::vsprintf(tmp_buffer, 1024, fmt_str, args);
  hprofile_log(tmp_buffer);
#endif
#if HART_ENABLE_STDIO
  ::vprintf(fmt_str, args);
#endif
  va_end(args);
}
/*
assertMsgFunc
*/
}
