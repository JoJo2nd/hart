/********************************************************************vo
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
 
#include "hart/base/scopestack.h"
#include "hart/base/threadlocalstorage.h"

namespace hart {
namespace tls {

static const uint32_t allocationSize = 8*1024*1024; // 8MB
static size_t tlsKey;

static void HART_API TLSKeyDestructor(void* key_value) {
    if (key_value) {
        memory::LinearAllocator* la = (memory::LinearAllocator*)key_value;
        delete la;
    }
}

void initTLSAllocaScratchPad() {
    tlsKey = createKey(TLSKeyDestructor);
}

memory::LinearAllocator* getTLSLinearAllocator() {
    memory::LinearAllocator* la = (memory::LinearAllocator*)getKeyValue(tlsKey);
    if (!la) {
        la = new memory::LinearAllocator(allocationSize);
        setKeyValue(tlsKey, la);
    }
    return la;
}

}
}