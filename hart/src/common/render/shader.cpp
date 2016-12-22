/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/render/shader.h"
#include "hart/base/crt.h"

HART_OBJECT_TYPE_DECL(hart::render::Shader);

namespace hart {
namespace render {
bool Shader::deserialiseObject(MarshallType const* in_data, hobjfact::SerialiseParams const&) {
    resource::Profile active_profile = resource::Profile_Direct3D11;

    for (auto& i : shaders)
        BGFX_HANDLE_SET_INVALID(i);

    auto const* shader_profiles = in_data->shaderArray();
    for (uint32_t i=0, n=shader_profiles->size(); i < n; ++i) {
        if ((*shader_profiles)[i]->profile() == active_profile) {
            bgfx::Memory const* prog_mem = bgfx::copy((*shader_profiles)[i]->mem()->data(), (*shader_profiles)[i]->mem()->size());
            shaders[active_profile] = bgfx::createShader(prog_mem);
        }
    }

    return true;
}

Shader::~Shader() {
    for (uint32_t i=0, n=resource::Profile_MAX+1; i<n; ++i) {
        if (bgfx::isValid(shaders[i])) {
            bgfx::destroyShader(shaders[i]);
        }
    }
}

}
}