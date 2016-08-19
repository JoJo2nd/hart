/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"
#include "hart/base/uuid.h"
#include "hart/base/debug.h"

namespace hart {
namespace resourcemanager {
    typedef huuid::uuid_t resid_t;

    struct Handle {

        bool loaded();
        void const* getData(uint32_t expected_typecc) {
            hdbassert(data, "Asset is not loaded yet. Check with call to loaded() first.");
            return (expected_typecc == typecc) ? data : nullptr;
        }

    private:
        friend Handle loadResource(resid_t res_id);
        friend void unloadResource(Handle res_hdl);

        resid_t     id;
        uint32_t    typecc;
        void const* data;
    };

    bool initialise();
    void update();
    void shutdown();
    Handle loadResource(resid_t res_id);
    void unloadResource(Handle res_hdl);
    //void* getResourceDataPtr(Handle res_hdl);

}
}

namespace hresmgr = hart::resourcemanager;
