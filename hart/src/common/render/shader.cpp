/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/render/shader.h"

HART_OBJECT_TYPE_DECL(hart::render::Shader)

namespace hart {
namespace render {
bool Shader::deserialiseObject(MarshallType const*, hobjfact::SerialiseParams const&) {
    return false;
}

bool Shader::serialiseObject(MarshallType**, hobjfact::SerialiseParams const&) const {
    return false;
}

bool Shader::linkObject() {
    return false;
}

}
}