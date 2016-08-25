/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/render/shader.h"

HART_OBJECT_TYPE_DECL(hart::render::Shader);

namespace hart {
namespace render {
bool Shader::deserialiseObject(MarshallType const* in_data, hobjfact::SerialiseParams const&) {
    auto const* shader_profiles = in_data->shaderArray();
    for (uint32_t i=0, n=shader_profiles->size(); i < n; ++i) {
        if ((*shader_profiles)[i]->profile() == resource::Profile_Direct3D11) {
            bgfx::Memory const* prog_mem = bgfx::copy((*shader_profiles)[i]->mem()->data(), (*shader_profiles)[i]->mem()->size());
            shaders[resource::Profile_Direct3D11] = bgfx::createShader(prog_mem);
        }
    }

    return true;
}

bool Shader::serialiseObject(MarshallType**, hobjfact::SerialiseParams const&) const {
    return false;
}

}
}