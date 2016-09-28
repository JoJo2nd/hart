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

    struct Handle {

        bool loaded();
        template<typename t_ty>
        t_ty* getData() {
            return (t_ty*)getData(t_ty::getTypeCC());
        }
        void* getData(uint32_t expected_typecc) {
            hdbassert(data, "Asset is not loaded yet. Check with call to loaded() first.");
            return (expected_typecc == typecc) ? data : nullptr;
        }
        bool isValid() {
            return !huuid::isNull(id);
        }

    private:
        friend Handle loadResource(resid_t res_id);
        friend void unloadResource(Handle res_hdl);

        resid_t     id;
        uint32_t    typecc = 0;
        Resource const* info = nullptr;
        void* data = nullptr;
    };

    bool initialise();
    void update();
    void shutdown();
    bool checkResourceLoaded(resid_t res_id);
    Handle loadResource(resid_t res_id);
    void unloadResource(Handle res_hdl);
    // Grabs a weak reference to loaded data. Can't be depended on to stay loaded 
    // UNLESS resource is a marked as a prerequisite of the resource requesting it.
    void* weakGetResource(resid_t res_id, uint32_t typecc);
    template<typename t_ty>
    t_ty* tweakGetResource(resid_t res_id) {
        return (t_ty*)weakGetResource(res_id, t_ty::getTypeCC());
    }

    class Collection {
        HART_OBJECT_TYPE(HART_MAKE_FOURCC('r','s','c','t'), fb::ResourceCollection)
    };

}
}

namespace hresmgr = hart::resourcemanager;
