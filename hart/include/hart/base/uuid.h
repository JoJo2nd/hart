/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"
#include "hart/base/crt.h"
#include "hart/base/debug.h"
#include <math.h>

namespace hart {
namespace uuid {

struct uuid_t {
    union {
        uint8_t bytes[16];
        uint32_t words[4];
        uint64_t dwords[2];
    };
    bool operator == (const uuid_t& rhs) const;
};

inline bool compareUUID(const uuid_t& lhs, const uuid_t& rhs) {
    return lhs.dwords[0] == rhs.dwords[0] && lhs.dwords[1] == rhs.dwords[1];
}

inline size_t toStringSize(const uuid_t& a) { return 37; }

inline size_t toString(const uuid_t& a, char* out, size_t out_size) {
    hdbassert(out_size >= 37, "UUID to string buffer is too small");
    auto* bytes = (const char*)&a;
    auto* ptr = out;
    for (auto i = 0u; i < 36;) {
        if (i != 8 && i != 13 && i != 18 && i != 23) {
            int val = (*bytes)&0xFF;
            if (hcrt::sprintf(ptr, out_size-i, "%02x", val) != 2) {
                return 0;
            }
            ++bytes;
            ptr+=2;
            i+=2;
        } else {
            *ptr = '-';
            ++ptr;
            ++i;
        }
    }
    return 36;
}

inline uuid_t fromString(const char* out, size_t in_size) {
    uuid_t out_uuid;
    hcrt::zeromem(&out_uuid, sizeof(out_uuid));
    auto len = hcrt::strlen(out);
    for (auto i=0u, dst=0u; i < in_size && i < len; ) {
        if (out[i] == '-') {
            ++i;
            continue;
        } 
        auto* bytes = ((char*)&out_uuid)+dst;
        *bytes = 0;
        for (auto j=0; j < 2; ++j) {
            auto radix = (int)pow(16,1-j);
            if (out[i+j] >= '0' && out[i+j] <= '9') {
                *bytes += (out[i+j] - '0')*radix;
            } else if (out[i+j] >= 'A' && out[i+j] <= 'F') {
                *bytes += (out[i+j] - 'A' + 10)*radix;
            } else if (out[i + j] >= 'a' && out[i + j] <= 'f') {
                *bytes += (out[i + j] - 'a' + 10)*radix;
            } else {
                hdbfatal("Unknown UUID string character %c", out[i+j]);
                hcrt::zeromem(&out_uuid, sizeof(out_uuid));
                return out_uuid;
            }
        }
        i+=2;
        ++dst;
    }
    return out_uuid;
}

inline bool isNull(const uuid_t& a) {
    return a.dwords[0] == 0 && a.dwords[1] == 0;
}

inline uuid_t getInvalid() {
    uuid_t zero = {0};
    return zero;    
}

bool uuid_t::operator == (const uuid_t& rhs) const {
    return compareUUID(*this, rhs);
}

}
}

namespace huuid = hart::uuid;
