/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/render/material.h"
#include "hart/render/shader.h"
#include "hart/render/render.h"
#include "hart/core/resourcemanager.h"

HART_OBJECT_TYPE_DECL(hart::render::Material);
HART_OBJECT_TYPE_DECL(hart::render::MaterialSetup);

namespace hart {
namespace render {
bool Material::deserialiseObject(MarshallType const* in_data, hobjfact::SerialiseParams const& params) {
    auto const* in_tech = in_data->techniques();
    techniques.resize(in_tech->size());
    auto const* inputs = in_data->defaultInputs();
    if (inputs)
        hdbprintf("default input size\n", inputs->size());
    else
        hdbprintf("Zero default inputs\n");
    for (uint32_t i = 0, n = in_tech->size(); i < n; ++i) {
        Technique* tech = &techniques[i];
        tech->type = (*in_tech)[i]->name();
        auto const* in_pass = (*in_tech)[i]->passes();
        tech->passes.resize(in_pass->size());
        for (uint32_t p = 0, pn = in_pass->size(); p < n; ++p) {
            Pass* pass = &tech->passes[p];
            pass->state.deserialiseObject((*in_pass)[p]->state(), params);
            pass->vertex = hresmgr::tweakGetResource<Shader>(uuid::fromData(*(*in_pass)[p]->vertex()));
            pass->pixel = hresmgr::tweakGetResource<Shader>(uuid::fromData(*(*in_pass)[p]->pixel()));
            pass->program = createProgram(pass->vertex, pass->pixel);
        }
    }
    return true;
}

Material::~Material() {
    for (auto& tech : techniques) {
        for (auto& pass : tech.passes) {
            destroyProgram(pass.program);
            pass.program = nullptr;
        }
    }
}

bool Material::serialiseObject(MarshallType**, hobjfact::SerialiseParams const&) const {
    return false;
}

bool MaterialSetup::deserialiseObject(MarshallType const* in_data, hobjfact::SerialiseParams const&) {
    material = hresmgr::tweakGetResource<Material>(uuid::fromData(*in_data->material()));
    return true;
}

bool MaterialSetup::serialiseObject(MarshallType**, hobjfact::SerialiseParams const&) const {
    return false;
}

}
}