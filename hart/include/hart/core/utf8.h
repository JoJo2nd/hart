/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"

namespace hart {
namespace utf8 {

typedef uint16_t Unicode;
/*
    static const int32_t MASKBITS   = 0x3F;
    static const int32_t MASKBYTE   = 0x80;
    static const int32_t MASK2BYTES = 0xC0;
    static const int32_t MASK3BYTES = 0xE0;
    static const int32_t MASK4BYTES = 0xF0;
    static const int32_t MASK5BYTES = 0xF8;
    static const int32_t MASK6BYTES = 0xFC;
    uint32_t encodeFromUnicode(Unicode ucIn, char* utf8Out);
    uint32_t encodeFromUnicodeString(const Unicode* restrict ucin, uint32_t
   limit, char* restrict utf8out, uint32_t tlimit);
    uint32_t bytesRequiredForUTF8(const Unicode& ucin);
    uint32_t decodeToUnicode( const char* restrict uft8In, Unicode& ucOut);
    uint32_t bytesInUTF8Character(const char* uft8In);
*/
size_t utf8_to_uc2(const char* src, Unicode* dst, size_t len);
size_t uc2_to_utf8(Unicode* uc_in, char* utf8_out, size_t buf_size);
}
}

namespace hutf8 = hart::utf8;
