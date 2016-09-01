/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/render/material.h"

HART_OBJECT_TYPE_DECL(hart::render::Material);
HART_OBJECT_TYPE_DECL(hart::render::MaterialSetup);

namespace hart {
namespace render {
bool Material::deserialiseObject(MarshallType const* in_data, hobjfact::SerialiseParams const&) {

    return true;
}

bool Material::serialiseObject(MarshallType**, hobjfact::SerialiseParams const&) const {
    return false;
}

bool MaterialSetup::deserialiseObject(MarshallType const* in_data, hobjfact::SerialiseParams const&) {

    return true;
}

bool MaterialSetup::serialiseObject(MarshallType**, hobjfact::SerialiseParams const&) const {
    return false;
}

}
}