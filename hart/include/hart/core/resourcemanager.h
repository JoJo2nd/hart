/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"
#include "hart/base/uuid.h"
#include "hart/base/debug.h"
#include "hart/base/util.h"
#include "hart/fbs/resourcecollection_generated.h"
#include "hart/core/objectfactory.h"

namespace hart {
namespace resourcemanager {

    typedef huuid::uuid_t resid_t;
    struct Resource;

#if HART_DEBUG_INFO
    enum ReloadEvent {
        ResourceReloaded,
    };
    typedef hstd::function<void(resid_t id, ReloadEvent evt, void* new_ptr, void* old_ptr)> ReloadEventProc;
    struct ReloadHandle {
        resid_t res_id;
        size_t i;
    };
#endif

    struct ResourceLoadData {
        bool persistFileData = false;
        char const* friendlyName = nullptr;
    };

#if HART_DEBUG_INFO
    struct HandleCopyable {

        HandleCopyable() {
            unlink();
        }
        HandleCopyable(HandleCopyable& rhs) {
            rhs.link(this);
        }
        HandleCopyable& operator = (HandleCopyable& rhs) {
            rhs.link(this);
            return *this;
        }
        ~HandleCopyable() {
            unlink();
        }

        virtual void onResourceEvent(resid_t id, ReloadEvent evt, void* new_ptr, void* old_ptr) {}

        void link(HandleCopyable* rhs) {
            next = rhs->next;
            prev = rhs;
            rhs->next->prev = this;
            rhs->next = this;
        }
        void unlink() {
            next->prev = prev;
            prev->next = next;
            next = prev = this;
        }

    protected:
        friend void updateResourceHotSwap();

        mutable HandleCopyable* prev;
        mutable HandleCopyable* next;

        void* getResourceDataPtr(resid_t res_id, uint32_t* o_typecc);
    };

    struct HandleNonCopyable : public HandleCopyable {
        HandleNonCopyable() = default;
        ~HandleNonCopyable() = default;
        HandleNonCopyable(HandleNonCopyable const& rhs) = delete;
        HandleNonCopyable& operator = (HandleNonCopyable const& rhs) = delete;

        HandleNonCopyable(HandleNonCopyable&& rhs) {
            std::swap(prev, rhs.prev);
            std::swap(next, rhs.next);
            std::swap(listeners, rhs.listeners);
        }
        HandleNonCopyable& operator = (HandleNonCopyable&& rhs) {
            std::swap(prev, rhs.prev);
            std::swap(next, rhs.next);
            std::swap(listeners, rhs.listeners);
            return *this;
        }

        virtual void onResourceEvent(resid_t id, ReloadEvent evt, void* new_ptr, void* old_ptr) override {
            //TODO: call resource listeners
            for (auto const& l : listeners) {
                l(id, evt, new_ptr, old_ptr);
            }
        }
        void addReloadListener(ReloadEventProc p) {
            listeners.push_back(p);
        }
        hstd::vector<ReloadEventProc> listeners;
    };

    template< typename t_copy_policy >
    struct HandleListenerBase : public t_copy_policy {

    };
#endif    

    template<typename t_copy_policy>
    struct HandleBase
#if HART_DEBUG_INFO
        : public HandleListenerBase<t_copy_policy>
#endif    
     {
        bool loaded(){
            if (data)
                return true;

            data = getResourceDataPtr(id, &typecc);
            return !!data;
        }
        void* getDataRaw(uint32_t expected_typecc) {
            hdbassert(data, "Asset is not loaded yet. Check with call to loaded() first.");
            return (expected_typecc == typecc) ? data : nullptr;
        }
        bool isValid() {
            return !huuid::isNull(id);
        }

        virtual void onResourceEvent(resid_t id, ReloadEvent evt, void* new_ptr, void* old_ptr) override {
            //TODO: update pointers first
            data = new_ptr;
            HandleListenerBase<t_copy_policy>::onResourceEvent(id, evt, new_ptr, old_ptr);
        }

    protected:
        friend void loadResource(resid_t res_id, HandleBase<t_copy_policy>* hdl);
        friend void unloadResource(HandleBase<t_copy_policy>* hdl);

        resid_t     id;
        Resource const* info = nullptr;
        void* data = nullptr;
        uint32_t    typecc = 0;
    };

    template<typename t_ty, typename t_copy_policy>
    struct THandle : public HandleBase<t_copy_policy> {
        t_ty* getData() {
            return (t_ty*)getDataRaw(t_ty::getTypeCC());
        }

        t_ty* operator -> () {
            return getData();
        }
    };

    template<typename t_copy_policy>
    struct WeakHandleBase
#if HART_DEBUG_INFO
        : public HandleListenerBase<t_copy_policy>
#endif    
    {

        bool valid() const {
            return !!data;
        }

        explicit operator bool () const {
            return valid();
        }

        virtual void onResourceEvent(resid_t id, ReloadEvent evt, void* new_ptr, void* old_ptr) override {
            data = new_ptr;
            HandleListenerBase<t_copy_policy>::onResourceEvent(id, evt, new_ptr, old_ptr);
        }

    protected:
        friend void weakGetResource(resid_t res_id, WeakHandleBase<t_copy_policy>* hdl);

        void* data = nullptr;
#if HART_DEBUG_INFO
        uint32_t typecc = 0;
#endif
    };

    template<typename t_ty, typename t_copy_policy>
    struct TWeakHandle : public WeakHandleBase<t_copy_policy> {
        t_ty* getData() {
#if HART_DEBUG_INFO
            hdbassert((t_ty::getTypeCC() == typecc), "WeakHandle is cast to an incorrect type");
#endif
            return (t_ty*)data;
        }

        t_ty* operator -> () {
            return getData();
        }
    };  

    bool initialise();
    void update();
    void shutdown();
    bool checkResourceLoaded(resid_t res_id);
    void loadResource(resid_t res_id, HandleBase<HandleCopyable>* hdl);
    void loadResource(resid_t res_id, HandleBase<HandleNonCopyable>* hdl);
    void unloadResource(HandleBase<HandleCopyable>* res_hdl);
    void unloadResource(HandleBase<HandleNonCopyable>* res_hdl);
    // Grabs a weak reference to loaded data. Can't be depended on to stay loaded 
    // Use when it is know that the resource will not unload due to some other dependency 
    void weakGetResource(resid_t res_id, WeakHandleBase<HandleCopyable>* hdl);
    void weakGetResource(resid_t res_id, WeakHandleBase<HandleNonCopyable>* hdl);

#if HART_DEBUG_INFO
    void updateResourceHotSwap();
#endif

    class Collection {
        HART_OBJECT_TYPE(HART_MAKE_FOURCC('r','s','c','t'), fb::ResourceCollection)
    };

    typedef THandle<Collection, HandleNonCopyable> ResourceCollection;
}
}

namespace hresmgr = hart::resourcemanager;
