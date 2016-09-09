/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/render/material.h"
#include "hart/render/shader.h"
#include "hart/render/render.h"
#include "hart/core/resourcemanager.h"
#include "hart/base/crt.h"

HART_OBJECT_TYPE_DECL(hart::render::Material);
HART_OBJECT_TYPE_DECL(hart::render::MaterialSetup);

namespace hart {
namespace render {
bool Material::deserialiseObject(MarshallType const* in_data, hobjfact::SerialiseParams const& params) {
    auto const* in_tech = in_data->techniques();
    params.resdata->persistFileData = true;
    techniques.resize(in_tech->size());
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
    //Process inputs
    auto const* in_inputs = in_data->defaultInputs();
    inputs.resize(in_inputs->size());
    for (uint32_t i=0, n=in_inputs->size(); i<n; ++i) {
        resource::MaterialInput const* in_input = (*in_inputs)[i];
        inputs[i].name = in_input->name()->c_str();
        inputs[i].dataType = in_input->data_type();
        if (void const* raw_data = in_input->data()) {
            switch(inputs[i].dataType) {
            case resource::MaterialInputData_Vec3: {
                inputs[i].dataIdx = (uint16_t)inputsData.vec.size();
                hart::resource::Vec3* v = (hart::resource::Vec3*)raw_data;
                inputsData.vec.push_back(hVec4(v->x(), v->y(), v->z(), 0.f));
            } break;
            case resource::MaterialInputData_Vec4: {
                inputs[i].dataIdx = (uint16_t)inputsData.vec.size();
                hart::resource::Vec4* v = (hart::resource::Vec4*)raw_data;
                inputsData.vec.push_back(hVec4(v->x(), v->y(), v->z(), v->w()));
            } break;
            case resource::MaterialInputData_Mat33: {
                inputs[i].dataIdx = (uint16_t)inputsData.matrix.size();
                hart::resource::Mat33* m = (hart::resource::Mat33*)raw_data;
                inputsData.matrix.push_back(hMat44(
                    hVec4(m->row1()->x(), m->row1()->y(), m->row1()->z(), 0.f),
                    hVec4(m->row2()->x(), m->row2()->y(), m->row2()->z(), 0.f),
                    hVec4(m->row3()->x(), m->row3()->y(), m->row3()->z(), 0.f),
                    hVec4(           0.f,            0.f,            0.f, 1.f)
                ));
            } break;
            case resource::MaterialInputData_Mat44: {
                inputs[i].dataIdx = (uint16_t)inputsData.matrix.size();
                hart::resource::Mat44* m = (hart::resource::Mat44*)raw_data;
                inputsData.matrix.push_back(hMat44(
                    hVec4(m->row1()->x(), m->row1()->y(), m->row1()->z(), m->row1()->w()),
                    hVec4(m->row2()->x(), m->row2()->y(), m->row2()->z(), m->row2()->w()),
                    hVec4(m->row3()->x(), m->row3()->y(), m->row3()->z(), m->row3()->w()),
                    hVec4(m->row4()->x(), m->row4()->y(), m->row4()->z(), m->row4()->w())
                ));
            } break;
            case resource::MaterialInputData_Texture2DInput: {
                //inputs[i].dataIdx = (uint16_t)inputsData.texture.size();
                //hart::resource::uuid* u = (hart::resource::uuid*)raw_data;
                //inputsData.texture.push_back(hresmgr::tweakGetResource<Texture2D>(huuid::fromData(*u)));
            } break;
            }
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
    //Process inputs
    auto const* in_inputs = in_data->inputs();
    inputs.resize(in_inputs->size());
    for(uint32_t i=0,n=in_inputs->size(); i<n; ++i) {
        resource::MaterialInput const* in_input = (*in_inputs)[i];
        inputs[i].handle = material->getInputParameterHandle(in_input->name()->c_str());
        hdbassert(inputs[i].handle.isValid(), "Material Setup parameter is defined but doesn't exist in base material\n");
        hdbassert(inputs[i].handle.type == (uint16_t)in_input->data_type(), "Material Setup parameter is defined but doesn't match type in base material\n");
        inputs[i].dataOffset = (uint16_t)inputData.size();
        if(void const* raw_data = in_input->data()) {
            switch(in_input->data_type()) {
            case resource::MaterialInputData_Vec3: {
                inputs[i].dataLen = sizeof(hVec4);
                hart::resource::Vec3* v = (hart::resource::Vec3*)raw_data;
                hVec4 vv(v->x(),v->y(),v->z(),0.f);
                inputData.resize(inputs[i].dataOffset+inputs[i].dataLen);
                hcrt::memcpy(&inputData[inputs[i].dataOffset], &vv, inputs[i].dataLen);
            } break;
            case resource::MaterialInputData_Vec4: {
                inputs[i].dataLen = sizeof(hVec4);
                hart::resource::Vec4* v = (hart::resource::Vec4*)raw_data;
                hVec4 vv(v->x(),v->y(),v->z(),v->w());
                inputData.resize(inputs[i].dataOffset+inputs[i].dataLen);
                hcrt::memcpy(&inputData[inputs[i].dataOffset], &vv, inputs[i].dataLen);
            } break;
            case resource::MaterialInputData_Mat33: {
                inputs[i].dataLen = sizeof(hMat44);
                hart::resource::Mat33* m = (hart::resource::Mat33*)raw_data;
                hMat44 d(
                    hVec4(m->row1()->x(),m->row1()->y(),m->row1()->z(),0.f),
                    hVec4(m->row2()->x(),m->row2()->y(),m->row2()->z(),0.f),
                    hVec4(m->row3()->x(),m->row3()->y(),m->row3()->z(),0.f),
                    hVec4(0.f,0.f,0.f,1.f)
                );
                inputData.resize(inputs[i].dataOffset+inputs[i].dataLen);
                hcrt::memcpy(&inputData[inputs[i].dataOffset], &d, inputs[i].dataLen);
            } break;
            case resource::MaterialInputData_Mat44: {
                inputs[i].dataLen = sizeof(hMat44);
                hart::resource::Mat44* m = (hart::resource::Mat44*)raw_data;
                hMat44 d(
                    hVec4(m->row1()->x(),m->row1()->y(),m->row1()->z(),m->row1()->w()),
                    hVec4(m->row2()->x(),m->row2()->y(),m->row2()->z(),m->row2()->w()),
                    hVec4(m->row3()->x(),m->row3()->y(),m->row3()->z(),m->row3()->w()),
                    hVec4(m->row4()->x(),m->row4()->y(),m->row4()->z(),m->row4()->w())
                );
                inputData.resize(inputs[i].dataOffset+inputs[i].dataLen);
                hcrt::memcpy(&inputData[inputs[i].dataOffset], &d, inputs[i].dataLen);
            } break;
            case resource::MaterialInputData_Texture2DInput: {
                inputs[i].dataLen = sizeof(void*);
                //hart::resource::uuid* u = (hart::resource::uuid*)raw_data;
                //inputsData.texture.push_back(hresmgr::tweakGetResource<Texture2D>(huuid::fromData(*u)));
            } break;
            }
        } else {
            switch(in_input->data_type()) {
            case resource::MaterialInputData_Vec3:
            case resource::MaterialInputData_Vec4: {
                inputs[i].dataLen = sizeof(hMat44); 
                inputData.resize(inputs[i].dataOffset+inputs[i].dataLen); 
            } break;
            case resource::MaterialInputData_Mat33:
            case resource::MaterialInputData_Mat44: {
                inputs[i].dataLen = sizeof(hMat44);
                inputData.resize(inputs[i].dataOffset+inputs[i].dataLen);
            } break;
            case resource::MaterialInputData_Texture2DInput: {
                inputs[i].dataLen = sizeof(void*);
                inputData.resize(inputs[i].dataOffset+inputs[i].dataLen);
            } break;
            }
        }
    }
    return true;
}

bool MaterialSetup::serialiseObject(MarshallType**, hobjfact::SerialiseParams const&) const {
    return false;
}

}
}
