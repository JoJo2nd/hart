/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"
#include "hart/base/uuid.h"

namespace hart {
namespace resourcemanager {
    typedef huuid::uuid_t resid_t;

    struct Handle {
        bool loaded();

        void const* data;
    };

    bool initialise();
    void update();
    void shutdown();
    Handle loadResource(resid_t res_id);
    void unloadResource(Handle res_hdl);
    void* getResourceDataPtr(Handle res_hdl);

}
}

namespace hresmgr = hart::resourcemanager;
