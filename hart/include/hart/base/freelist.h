/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"
#include "hart/base/std.h"
#include "hart/base/debug.h"
#include <algorithm>

namespace hart {

template<typename t_ty, uintptr_t pageSize = 4096>
class Freelist {

    static const size_t NodePadSize = sizeof(t_ty) - sizeof(void*) - sizeof(uint32_t);
    static const uintptr_t PtrMask = ~(pageSize - 1);
    static const uint32_t EmptyBlockCount = pageSize / sizeof(t_ty);
    static_assert(pageSize >= sizeof(t_ty), "pageSize is smaller than type size");
    static_assert((pageSize & (pageSize - 1)) == 0, "pageSize must be a power of 2");

    struct Node {
        Node* next;
        uint32_t nodeCount; // number of nodes after this that are free for allocation, inclusive of this node too.
        uint8_t pad[NodePadSize];
    };
    struct Page {
        hstd::unique_ptr<uint8_t> pageMem;
        uint32_t freeCount = EmptyBlockCount;
        Node* freelist = nullptr;
    };

    hstd::vector<Page> freelistPages;

    Page* getPage() {
        for (size_t i = 0, n = freelistPages.size(); i < n; ++i) {
            if (freelistPages[i].freeCount) {
                uint32_t j = 0;
                while (i - j > 0 && (freelistPages[i].freeCount-1) < freelistPages[i - j].freeCount) ++j;
                if (j) std::swap(freelistPages[i], freelistPages[i - j]);
                return &freelistPages[i];
            }
        }
        Page new_page;
        new_page.pageMem.reset(new uint8_t[pageSize]);
        new_page.freelist = (Node*)new_page.pageMem.get();
        new_page.freelist->nodeCount = new_page.freeCount;
        new_page.freelist->next = nullptr;
        freelistPages.insert(freelistPages.begin(), std::move(new_page));
        return &freelistPages[0];
    }
    Page* findPage(void* ptr) {
        for (size_t i = 0, n = freelistPages.size(); i < n; ++i) {
            if (ptr >= freelistPages[i].pageMem.get() && ptr < (freelistPages[i].pageMem.get()+pageSize)) {
                uint32_t j = 0;
                while (i+j < (n-1) && (freelistPages[i].freeCount+1) > freelistPages[i+j+1].freeCount) ++j;
                if (j) std::swap(freelistPages[i], freelistPages[i+j]);
                return &freelistPages[i+j];
            }
        }
        hdbfatal("Unable to find page for freelist node ptr 0x%p", ptr);
        return nullptr;
    }
public:
    Freelist() = default;
    Freelist(Freelist const& rhs) = delete;
    Freelist& operator = (Freelist const& rhs) = delete;

    t_ty* allocate() {
        auto* page = getPage();
        --page->freeCount;
        auto* ptr = (t_ty*)page->freelist;
        --page->freelist->nodeCount;
        if (page->freelist->nodeCount == 0) {
            page->freelist = page->freelist->next;
        } else {
            (page->freelist+1)->nodeCount = page->freelist->nodeCount;
            (page->freelist + 1)->next = page->freelist->next;
            page->freelist = page->freelist + 1;
        }

#if HART_VERIFY_FREELIST
        for (Node* i = page->freelist; i; i = i->next) {
            hdbassert(i < i->next || i->next == nullptr, "Freelist allocator unallocated list is not address sorted");
            hdbassert(i + i->nodeCount != i->next || i->next == nullptr, "Freelist allocator unallocated list is not merged correctly");
        }
        for (size_t i = 0, n = freelistPages.size() - 1; i < n; ++i) {
            hdbassert(freelistPages[i].freeCount >= freelistPages[i + 1].freeCount, "Freelist bin pages aren't sorted correctly");
        }
#endif

        return ptr;
    }
    void release(void* ptr) {
        if (!ptr) return;
        // insertion sort pointer back in, merging into freeblock, if required.
        auto* page = findPage(ptr);
        hdbassert(ptr >= page->pageMem.get() && ptr < page->pageMem.get()+pageSize, "Pointer is outsize owning page range.");
        ++page->freeCount;
        auto* fl_new_ptr = (Node*)ptr;
        if (!page->freelist) {
            page->freelist = fl_new_ptr;
            fl_new_ptr->nodeCount = 1;
            fl_new_ptr->next = nullptr;
            return;
        }
        
        auto* prev = (Node*)nullptr;
        auto* node = page->freelist;
        while(node) {
            if (fl_new_ptr > node) {
                if (prev) prev->next = fl_new_ptr;
                fl_new_ptr->next = node;
                fl_new_ptr->nodeCount = 1;

                //check for merge forward
                if (fl_new_ptr+1 == fl_new_ptr->next) {
                    if (prev) prev->next = fl_new_ptr;
                    fl_new_ptr->nodeCount += fl_new_ptr->next->nodeCount;
                    fl_new_ptr->next = fl_new_ptr->next;
                }
                //check for merge backward
                if (prev && prev+prev->nodeCount == fl_new_ptr) {
                    prev->nodeCount += fl_new_ptr->nodeCount;
                    prev->next = fl_new_ptr->next;
                }
                break;
            }
            prev = node;
            node = node->next;
        }

#if HART_VERIFY_FREELIST
        for (Node* i=page->freelist; i; i=i->next) {
            hdbassert(i < i->next || i->next == nullptr, "Freelist allocator unallocated list is not address sorted");
            hdbassert(i+i->nodeCount != i->next || i->next == nullptr, "Freelist allocator unallocated list is not merged correctly");
        }
        for (size_t i=0, n=freelistPages.size()-1; i<n; ++i) {
            hdbassert(freelistPages[i].freeCount <= freelistPages[i+1].freeCount, "Freelist bin pages aren't sorted correctly");
        }
#endif
        if (page->freeCount == EmptyBlockCount) {
            freelistPages.erase(freelistPages.begin() + (page - freelistPages.data()));
        }
    }
};

}

