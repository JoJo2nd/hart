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

    private: 
        resid_t id;
        uint32_t ptr;
    };

    bool initialise();
    void update();
    void shutdown();
    //void    injectResource(void* ptr);
    //void    removeResource(hStringID res_id);
    Handle loadResource(resid_t res_id);
    void unloadResource(Handle res_hdl);
    void* getResourceDataPtr(Handle res_hdl, uint32_t res_type_id);

}
}

namespace hresmgr = hart::resourcemanager;
