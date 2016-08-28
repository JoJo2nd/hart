/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#include "hart/base/threadlocalstorage.h"
#include "hart/base/mutex.h"
#include "hart/base/std.h"
#include "hart/windows.inc"

namespace hart {
namespace tls {    

struct TLSDestructor{
    TLSDestructor()
        : tlsSlot_(0)
        , tlsDestructor_(nullptr)
    {}

    size_t         tlsSlot_;
    KeyDestructor  tlsDestructor_;
};
hMutex tlsMutex;
hstd::vector< TLSDestructor > tlsDestructorArray;

size_t  createKey(KeyDestructor destructor) {
    tlsMutex.lock();
    size_t tlsSlot = TlsAlloc();
    TLSDestructor dtor;
    dtor.tlsSlot_ = tlsSlot;
    dtor.tlsDestructor_ = destructor;
    tlsDestructorArray.push_back(dtor);
    tlsMutex.unlock();
    return tlsSlot;
}

void  deleteKey(size_t key) {
    tlsMutex.lock();
    for (auto i=tlsDestructorArray.begin(), n=tlsDestructorArray.end(); i!=n; ++i) {
        if (i->tlsSlot_==key) {
            tlsDestructorArray.erase(i);
            break;
        }
    }
    TlsFree((DWORD)key);
    tlsMutex.unlock();
}

void  setKeyValue(size_t key, void* value) {
    TlsSetValue((DWORD)key, value);
}

void*  getKeyValue(size_t key) {
    return TlsGetValue((DWORD)key);
}

void  threadExit() {
    for (auto i=tlsDestructorArray.cbegin(), n=tlsDestructorArray.cend(); i!=n; ++i) {
        if (i->tlsDestructor_) {
            i->tlsDestructor_(getKeyValue(i->tlsSlot_));
        }
    }
}

}
}