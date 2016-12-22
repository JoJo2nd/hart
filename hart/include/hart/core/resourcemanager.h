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

    struct ResourceLoadData {
        bool persistFileData = false;
        char const* friendlyName = nullptr;
    }; 

    struct HandleBase {
        bool loaded();
        void* getDataRaw(uint32_t expected_typecc) {
            hdbassert(data, "Asset is not loaded yet. Check with call to loaded() first.");
            return (expected_typecc == typecc) ? data : nullptr;
        }
        bool valid() const {
            return !huuid::isNull(id);
        }
    protected:
        friend void loadResource(resid_t res_id, HandleBase* hdl);
        friend void unloadResource(HandleBase* hdl);

        resid_t     id;
        Resource const* info = nullptr;
        void* data = nullptr;
        uint32_t    typecc = 0;
    };

    template<typename t_ty>
    struct THandle : public HandleBase {
        t_ty* getData() {
            return (t_ty*)getDataRaw(t_ty::getTypeCC());
        }

        t_ty* operator -> () {
            return getData();
        }
    };

    struct WeakHandleBase {
        bool valid() const {
            return !!data;
        }

        explicit operator bool () const {
            return valid();
        }

    protected:
        friend void weakGetResource(resid_t res_id, WeakHandleBase* hdl);

        void* data = nullptr;
#if HART_DEBUG_INFO
        uint32_t typecc = 0;
#endif
    };

    template<typename t_ty>
    struct TWeakHandle : public WeakHandleBase {
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
    void loadResource(resid_t res_id, HandleBase* hdl);
    void unloadResource(HandleBase* res_hdl);
    // Grabs a weak reference to loaded data. Can't be depended on to stay loaded 
    // Use when it is know that the resource will not unload due to some other dependency 
    void weakGetResource(resid_t res_id, WeakHandleBase* hdl);

    class Collection {
        HART_OBJECT_TYPE(HART_MAKE_FOURCC('r','s','c','t'), fb::ResourceCollection)
    };

    typedef THandle<Collection> ResourceCollection;
}
}

namespace hresmgr = hart::resourcemanager;
