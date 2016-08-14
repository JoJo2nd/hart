/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

namespace hart {
namespace crt {
    inline void memset(void* dest,uint8_t val,size_t size) {
        ::memset(dest,val,size);
    }

    inline void memcpy(void* dest,const void* src,size_t size) {
        ::memcpy(dest,src,size);
    }

    inline void zeromem(void* dest,size_t size) {
        ::memset(dest,0,size);
    }

    inline void memmove(void* dest,const void* src,size_t size) {
        ::memmove(dest,src,size);
    }

    inline int memcpy(const void* lhs,const void* rhs,size_t size) {
        return ::memcmp(lhs,rhs,size);
    }

    // return zero on success
    inline int strcpy(char * dst, size_t dstsize, char const* src) {
        return ::strcpy_s(dst, dstsize, src);
    }

    inline size_t strlen(const char* s1) {
        return ::strlen(s1);
    }

    inline void strncpy(char* dest, size_t destlen, const char* src) {
        ::strncpy(dest,src,destlen);
    }

    inline void strcat(char* dest, size_t destlen, const char* src) {
        ::strncat(dest, src, destlen);
    }

    inline int strcmp(const char* s1, const char* s2) {
        return ::strcmp(s1,s2);
    }

    inline char const* strchr(char const* s1,char ch) {
        return ::strchr(s1, ch);
    }

    inline const char* strrchr(const char* s1, char ch) {
        return ::strrchr(s1, ch);
    }

    inline int stricmp(const char* s1,const char* s2) {
#if (HART_PLATFORM == HART_PLATFORM_WINDOWS)
        return ::_stricmp(s1,s2);
#elif (HART_PLATFORM == HART_PLATFORM_LINUX)
        return ::strcasecmp(s1,s2);
#else
#       error ("Unknown platform")
#endif
    }

    inline int32_t sprintf(char* dest, size_t destlen, const char* format,...) {
        va_list marker;
        va_start(marker,format);

#if (HART_PLATFORM == HART_PLATFORM_LINUX)
        int32_t r = ::vsprintf(dest,format,marker);
#elif (HART_PLATFORM == HART_PLATFORM_WINDOWS)
        int32_t r = ::vsprintf_s(dest,destlen,format,marker);
#else
#       error ("Unknown platform")
#endif

        va_end(marker);

        return r;
    }

    inline int atoi(const char* str) {
        return ::atoi(str);
    }

    inline float atof(const char* str) {
        return float(::atof(str));
    }

    inline int isspace(char c) {
        return ::isspace(c);
    }

    inline int strncmp(const char* str1,const char* str2,size_t n) {
        return ::strncmp(str1,str2,n);
    }
}
}

namespace hcrt = hart::crt;