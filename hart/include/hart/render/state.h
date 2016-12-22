/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#pragma once

#include "hart/config.h"
#include "hart/base/util.h"
#include "hart/core/objectfactory.h"
#include "hart/fbs/renderstate_generated.h"

namespace hart {
namespace render {

class State {
    HART_OBJECT_TYPE(HART_MAKE_FOURCC('r','s','t', 'e'), resource::RenderState)
    public:
        State() = default;

        uint64_t stateMask = 0;    
    
};

}
}