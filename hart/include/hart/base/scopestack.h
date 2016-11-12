/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once
// Simplified scope stack implementation
 
#include "hart/config.h"
#include "hart/base/std.h"
#include "hart/base/debug.h"

namespace hart {
namespace memory {

struct LinearAllocator {
    explicit LinearAllocator(size_t size) 
        : mem(new uint8_t[size])
    {
        basePtr = mem.get();
        endPtr = (uint8_t*)basePtr+size;
        currentPtr = endPtr;
    }

    LinearAllocator(void* ptr, size_t size) 
        : basePtr(ptr)
        , endPtr((uint8_t*)ptr+size)
        , currentPtr((uint8_t*)ptr+size)
    {}


    LinearAllocator(LinearAllocator const& rhs) = delete;
    LinearAllocator& operator = (LinearAllocator const& ) = delete;

    static const uint32_t Alignment = 16;

    hstd::unique_ptr<uint8_t> mem; 
    void* basePtr;
    void* endPtr;
    void* currentPtr; // Like the stack, we grow down.

    static size_t alignedSize(size_t s) { return (s + (Alignment - 1)) & ~(Alignment - 1); }
    size_t getRemaining() const { return (uintptr_t)currentPtr - (uintptr_t)basePtr; }
    size_t getUsed() const { return (uintptr_t)endPtr - (uintptr_t)currentPtr; }
    void* alloc(size_t size) {
        size = alignedSize(size);
        hdbassert(getRemaining() >= size, "Not enough space for allocation.\n");
        currentPtr = ((uint8_t*)currentPtr) - size;
        return currentPtr;
    }

    void rewind(void* ptr) {
        hdbassert(ptr >= currentPtr && ptr <= endPtr, "Pointer was not allocated from here.\n");
        currentPtr = ptr;
    }
};

struct Finalizer {
        void (*fn)(void *ptr);
        Finalizer *chain;
};
 
template <typename T>
void destructorCall(void *ptr) {
        static_cast<T*>(ptr)->~T();
}
 
class ScopeStack {
private:
        LinearAllocator& m_alloc;
        void* m_rewindPoint;
        Finalizer* m_finalizerChain;
 
        static void* objectFromFinalizer(Finalizer *f) {
                return ((uint8_t*)f) + LinearAllocator::alignedSize(sizeof(Finalizer));
        }
 
        Finalizer *allocWithFinalizer(size_t size) {
                return (Finalizer*) m_alloc.alloc(size + LinearAllocator::alignedSize(sizeof(Finalizer)));
        }
 
        template <typename T>
        T* newObject() {
            // Allocate memory for finalizer + object.
            Finalizer* f = allocWithFinalizer(sizeof(T));

            // Placement construct object in space after finalizer. Do this before
            // linking in the finalizer for this object so nested calls will be
            // finalized after this object.
            T* result = new (objectFromFinalizer(f)) T;

            // Link this finalizer onto the chain.
            f->fn = &destructorCall<T>;
            f->chain = m_finalizerChain;
            m_finalizerChain = f;
            return result;
        }
 
        template <typename T>
        T* newPOD() {
            return new (m_alloc.alloc(sizeof(T))) T;
        }

public:
    explicit ScopeStack(LinearAllocator& a)
    :       m_alloc(a)
    ,       m_rewindPoint(a.currentPtr)
    ,       m_finalizerChain(0)
    {}

    ~ScopeStack() {
            for (Finalizer *f = m_finalizerChain; f; f = f->chain) {
                    (*f->fn)(objectFromFinalizer(f));
            }
            m_alloc.rewind(m_rewindPoint);
    }

    ScopeStack(ScopeStack const&) = delete;
    ScopeStack& operator = (ScopeStack const&) = delete;

    template <typename T>
    T* alloc() {
        return hstd::is_pod<T>::value ? newPOD<T>() : newObject<T>();
    }

    // TODO: arrays & perfect forwarding...
};
 
}
namespace tls {

void initTLSAllocaScratchPad();
memory::LinearAllocator* getTLSLinearAllocator();
 
}
}

namespace htls = hart::tls;
namespace hmem = hart::memory;

/*
struct Foo {
        static int count;
        int num;
        Foo() : num(count++) { printf("Foo ctor %d\n", num); }
        ~Foo() { printf("Foo dtor %d\n", num); }
};
int Foo::count;
 
u8 test_mem[65536];
 
int main()
{
        LinearAllocator allocator(test_mem, sizeof(test_mem));
 
        ScopeStack outerScope(allocator);
        Foo* foo0 = outerScope.alloc<Foo>();
 
        {
                ScopeStack innerScope(allocator);
                Foo* foo1 = innerScope.alloc<Foo>();
                Foo* foo2 = innerScope.alloc<Foo>();
        }
 
        return 0;
}
*/
