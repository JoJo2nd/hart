/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/render/state.h"

HART_OBJECT_TYPE_DECL(hart::render::State);

namespace hart {
namespace render {
    
bool State::deserialiseObject(MarshallType const* in_data, hobjfact::SerialiseParams const&) {
    return true;
}

bool State::serialiseObject(MarshallType**, hobjfact::SerialiseParams const&) const {
    return false;
}

}
}
