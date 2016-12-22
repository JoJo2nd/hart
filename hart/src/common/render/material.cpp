/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/render/material.h"
#include "hart/base/crt.h"
#include "hart/core/resourcemanager.h"
#include "hart/render/render.h"
#include "hart/render/shader.h"
#include "hart/render/texture.h"

HART_OBJECT_TYPE_DECL(hart::render::Material);
HART_OBJECT_TYPE_DECL(hart::render::MaterialSetup);

namespace hart {
namespace render {

struct MaterialTextureSlot {
  Material::TextureResHandle* resource;
  Texture                     texture;
  uint8_t                     slot;
  uint16_t                    flags;
};

static bgfx::UniformType::Enum MaterialInputTypeToUniformType[resource::MaterialInputData_MAX + 1] = {
  bgfx::UniformType::End,  // MaterialInputData_NONE = 0,
  bgfx::UniformType::Vec4, // MaterialInputData_Vec3 = 1,
  bgfx::UniformType::Vec4, // MaterialInputData_Vec4 = 2,
  bgfx::UniformType::Mat4, // MaterialInputData_Mat33 = 3,
  bgfx::UniformType::Mat4, // MaterialInputData_Mat44 = 4,
  bgfx::UniformType::Int1, // MaterialInputData_Texture2DInput = 5,
};


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
      hresmgr::weakGetResource(uuid::fromData(*(*in_pass)[p]->vertex()), &pass->vertex);
      hresmgr::weakGetResource(uuid::fromData(*(*in_pass)[p]->pixel()), &pass->pixel);
      pass->program = createProgram(pass->vertex.getData(), pass->pixel.getData());
    }
  }
  // Process inputs
  auto const* in_inputs = in_data->defaultInputs();
  if (!in_inputs) return true;
  inputs.resize(in_inputs->size());
  uint32_t datalen = 0;
  for (uint32_t i = 0, n = in_inputs->size(); i < n; ++i) {
    resource::MaterialInput const* in_input = (*in_inputs)[i];
    inputs[i].name = in_input->name()->c_str();
    inputs[i].dataType = in_input->data_type();
    inputs[i].dataIdx = Input::Invalid;
    inputs[i].uniform = bgfx::createUniform(inputs[i].name, MaterialInputTypeToUniformType[inputs[i].dataType]);
    if (void const* raw_data = in_input->data()) {
      switch (inputs[i].dataType) {
      case resource::MaterialInputData_Vec3Input: {
        inputs[i].dataIdx = (uint16_t)datalen;
        hdbassert(inputs[i].dataIdx == datalen, "Size overflow\n");
        hart::resource::Vec3* v = (hart::resource::Vec3*)raw_data;
        hVec4                 vv(v->x(), v->y(), v->z(), 0.f);
        inputData.resize(datalen + sizeof(vv));
        hcrt::memcpy(&inputData[datalen], &vv, sizeof(vv));
        datalen += sizeof(vv);
      } break;
      case resource::MaterialInputData_Vec4Input: {
        inputs[i].dataIdx = (uint16_t)datalen;
        hdbassert(inputs[i].dataIdx == datalen, "Size overflow\n");
        hart::resource::Vec4* v = (hart::resource::Vec4*)raw_data;
        hVec4                 vv(v->x(), v->y(), v->z(), v->w());
        inputData.resize(datalen + sizeof(vv));
        hcrt::memcpy(&inputData[datalen], &vv, sizeof(vv));
        datalen += sizeof(vv);
      } break;
      case resource::MaterialInputData_Mat33Input: {
        inputs[i].dataIdx = (uint16_t)datalen;
        hdbassert(inputs[i].dataIdx == datalen, "Size overflow\n");
        hart::resource::Mat33* m = (hart::resource::Mat33*)raw_data;
        hMat33                 mm(hVec3(m->row1().x(), m->row1().y(), m->row1().z()),
                  hVec3(m->row2().x(), m->row2().y(), m->row2().z()),
                  hVec3(m->row3().x(), m->row3().y(), m->row3().z()));
        inputData.resize(datalen + sizeof(mm));
        hcrt::memcpy(&inputData[datalen], &mm, sizeof(mm));
        datalen += sizeof(mm);
      } break;
      case resource::MaterialInputData_Mat44Input: {
        inputs[i].dataIdx = (uint16_t)datalen;
        hdbassert(inputs[i].dataIdx == datalen, "Size overflow\n");
        hart::resource::Mat44* m = (hart::resource::Mat44*)raw_data;
        hMat44                 mm(hVec4(m->row1().x(), m->row1().y(), m->row1().z(), m->row1().w()),
                  hVec4(m->row2().x(), m->row2().y(), m->row2().z(), m->row2().w()),
                  hVec4(m->row3().x(), m->row3().y(), m->row3().z(), m->row3().w()),
                  hVec4(m->row4().x(), m->row4().y(), m->row4().z(), m->row4().w()));
        inputData.resize(datalen + sizeof(mm));
        hcrt::memcpy(&inputData[datalen], &mm, sizeof(mm));
        datalen += sizeof(mm);
      } break;
      case resource::MaterialInputData_Texture2DInput: {
        inputs[i].dataIdx = (uint16_t)datalen;
        hdbassert(inputs[i].dataIdx == datalen, "Size overflow\n");
        resource::Texture2DInput* t = (resource::Texture2DInput*)raw_data;
        Texture                   invalid_tex = BGFX_INVALID_HANDLE;
        uint16_t                  flags = 0;
        if (t->pointsample()) flags |= BGFX_TEXTURE_MIP_POINT | BGFX_TEXTURE_MIN_POINT | BGFX_TEXTURE_MAG_POINT;
        if (t->anisotropic()) flags |= BGFX_TEXTURE_MIN_ANISOTROPIC | BGFX_TEXTURE_MAG_ANISOTROPIC;
        if (t->wrapU() == resource::TextureWrap_Mirror) flags |= BGFX_TEXTURE_U_MIRROR;
        if (t->wrapV() == resource::TextureWrap_Mirror) flags |= BGFX_TEXTURE_V_MIRROR;
        if (t->wrapU() == resource::TextureWrap_Clamp) flags |= BGFX_TEXTURE_U_CLAMP;
        if (t->wrapV() == resource::TextureWrap_Clamp) flags |= BGFX_TEXTURE_V_CLAMP;
        TextureResWeakHandle thdl;
        if (t->resid()) hresmgr::weakGetResource(huuid::fromData(*t->resid()), &thdl);
        MaterialTextureSlot tslot = {nullptr, t->resid() ? thdl->texture : invalid_tex, t->slot(), flags};
        inputData.resize(datalen + sizeof(tslot));
        hcrt::memcpy(&inputData[datalen], &tslot, sizeof(tslot));
        datalen += sizeof(tslot);
      } break;
      }
    }
  }
  return true;
}

Material::~Material() {
  for (auto& tech : techniques) {
    for (auto& pass : tech.passes) {
      hrnd::destroyProgram(pass.program);
      pass.program = getInvalidProgram();
    }
  }
  for (auto& in : inputs) {
    bgfx::destroyUniform(in.uniform);
  }
}

void Material::setParameters(MaterialHandleData const* hrestrict in_data, uint32_t in_data_count,
                             uint8_t const* hrestrict base_add) {
  uint32_t n_inputs = uint32_t(inputs.size());
  for (uint32_t i = 0; i < n_inputs; ++i) {
    inputs[i].set = 0;
  }

  uint8_t const* dafault_add = inputData.data();
  for (uint32_t i = 0, n = in_data_count; i < n; ++i) {
    uint32_t i_idx = in_data[i].handle.idx;
    inputs[i_idx].set = 1;
    if (in_data[i].handle.type >= resource::MaterialInputData_Texture2DInput) {
      MaterialTextureSlot* tslot = (MaterialTextureSlot*)(base_add + in_data[i].dataOffset);
      MaterialTextureSlot* dtslot = (MaterialTextureSlot*)(dafault_add + inputs[i_idx].dataIdx);
      bgfx::setTexture(dtslot->slot, inputs[i_idx].uniform, tslot->texture, dtslot->flags);
    } else {
      bgfx::setUniform(inputs[i_idx].uniform, base_add + in_data[i].dataOffset);
    }
  }

  // Apply any default that weren't set by in_data
  for (uint32_t i = 0; i < n_inputs; ++i) {
    if (inputs[i].set == 0 && inputs[i].dataIdx != Input::Invalid) {
      if (inputs[i].dataType >= resource::MaterialInputData_Texture2DInput) {
        MaterialTextureSlot* tslot = (MaterialTextureSlot*)(dafault_add + inputs[i].dataIdx);
        bgfx::setTexture(tslot->slot, inputs[i].uniform, tslot->texture, tslot->flags);
      } else {
        bgfx::setUniform(inputs[i].uniform, dafault_add + inputs[i].dataIdx);
      }
    }
  }
}

bool MaterialSetup::deserialiseObject(MarshallType const* in_data, hobjfact::SerialiseParams const&) {
  hresmgr::weakGetResource(uuid::fromData(*in_data->material()), &material);
  // Process inputs
  auto const* in_inputs = in_data->inputs();
  if (!in_inputs) return true;
  inputs.resize(in_inputs->size());
  for (uint32_t i = 0, n = in_inputs->size(); i < n; ++i) {
    resource::MaterialInput const* in_input = (*in_inputs)[i];
    inputs[i].handle = material->getInputParameterHandle(in_input->name()->c_str());
    hdbassert(inputs[i].handle.isValid(), "Material Setup parameter is defined but doesn't exist in base material\n");
    hdbassert(inputs[i].handle.type == (uint16_t)in_input->data_type(),
              "Material Setup parameter is defined but doesn't match type in base material\n");
    inputs[i].dataOffset = (uint16_t)inputData.size();
    if (void const* raw_data = in_input->data()) {
      switch (in_input->data_type()) {
      case resource::MaterialInputData_Vec3Input: {
        inputs[i].dataLen = sizeof(hVec4);
        hart::resource::Vec3* v = (hart::resource::Vec3*)raw_data;
        hVec4                 vv(v->x(), v->y(), v->z(), 0.f);
        inputData.resize(inputs[i].dataOffset + inputs[i].dataLen);
        hcrt::memcpy(&inputData[inputs[i].dataOffset], &vv, inputs[i].dataLen);
      } break;
      case resource::MaterialInputData_Vec4Input: {
        inputs[i].dataLen = sizeof(hVec4);
        hart::resource::Vec4* v = (hart::resource::Vec4*)raw_data;
        hVec4                 vv(v->x(), v->y(), v->z(), v->w());
        inputData.resize(inputs[i].dataOffset + inputs[i].dataLen);
        hcrt::memcpy(&inputData[inputs[i].dataOffset], &vv, inputs[i].dataLen);
      } break;
      case resource::MaterialInputData_Mat33Input: {
        inputs[i].dataLen = sizeof(hMat44);
        hart::resource::Mat33* m = (hart::resource::Mat33*)raw_data;
        hMat33 d(hVec3(m->row1().x(), m->row1().y(), m->row1().z()), hVec3(m->row2().x(), m->row2().y(), m->row2().z()),
                 hVec3(m->row3().x(), m->row3().y(), m->row3().z()));
        inputData.resize(inputs[i].dataOffset + inputs[i].dataLen);
        hcrt::memcpy(&inputData[inputs[i].dataOffset], &d, inputs[i].dataLen);
      } break;
      case resource::MaterialInputData_Mat44Input: {
        inputs[i].dataLen = sizeof(hMat44);
        hart::resource::Mat44* m = (hart::resource::Mat44*)raw_data;
        hMat44                 d(hVec4(m->row1().x(), m->row1().y(), m->row1().z(), m->row1().w()),
                 hVec4(m->row2().x(), m->row2().y(), m->row2().z(), m->row2().w()),
                 hVec4(m->row3().x(), m->row3().y(), m->row3().z(), m->row3().w()),
                 hVec4(m->row4().x(), m->row4().y(), m->row4().z(), m->row4().w()));
        inputData.resize(inputs[i].dataOffset + inputs[i].dataLen);
        hcrt::memcpy(&inputData[inputs[i].dataOffset], &d, inputs[i].dataLen);
      } break;
      case resource::MaterialInputData_Texture2DInput: {
        inputs[i].dataLen = sizeof(MaterialTextureSlot);
        resource::Texture2DInput* t = (resource::Texture2DInput*)raw_data;
        uint16_t                  flags = 0;
        if (t->pointsample()) flags |= BGFX_TEXTURE_MIP_POINT | BGFX_TEXTURE_MIN_POINT | BGFX_TEXTURE_MAG_POINT;
        if (t->anisotropic()) flags |= BGFX_TEXTURE_MIN_ANISOTROPIC | BGFX_TEXTURE_MAG_ANISOTROPIC;
        if (t->wrapU() == resource::TextureWrap_Mirror) flags |= BGFX_TEXTURE_U_MIRROR;
        if (t->wrapV() == resource::TextureWrap_Mirror) flags |= BGFX_TEXTURE_V_MIRROR;
        if (t->wrapU() == resource::TextureWrap_Clamp) flags |= BGFX_TEXTURE_U_CLAMP;
        if (t->wrapV() == resource::TextureWrap_Clamp) flags |= BGFX_TEXTURE_V_CLAMP;
        Material::TextureResWeakHandle thdl;
        hresmgr::weakGetResource(huuid::fromData(*t->resid()), &thdl);
        MaterialTextureSlot tslot = {nullptr, thdl->texture, t->slot(), flags};
        inputData.resize(inputs[i].dataOffset + inputs[i].dataLen);
        hcrt::memcpy(&inputData[inputs[i].dataOffset], &tslot, sizeof(tslot));
      } break;
      }
    } else {
      switch (in_input->data_type()) {
      case resource::MaterialInputData_Vec3Input:
      case resource::MaterialInputData_Vec4Input: {
        inputs[i].dataLen = sizeof(hMat44);
        inputData.resize(inputs[i].dataOffset + inputs[i].dataLen);
      } break;
      case resource::MaterialInputData_Mat33Input:
      case resource::MaterialInputData_Mat44Input: {
        inputs[i].dataLen = sizeof(hMat44);
        inputData.resize(inputs[i].dataOffset + inputs[i].dataLen);
      } break;
      case resource::MaterialInputData_Texture2DInput: {
        inputs[i].dataLen = sizeof(MaterialTextureSlot);
        inputData.resize(inputs[i].dataOffset + inputs[i].dataLen);
      } break;
      }
    }
  }

  return true;
}

void MaterialSetup::setParameter(MaterialInputHandle p, Texture const* d) {
  hdbassert(p.isValid() && p.type >= resource::MaterialInputData_Texture2DInput, "Invalid parameter handle!\n");
  MaterialTextureSlot slot = {nullptr, *d, 0};
  hcrt::memcpy(&inputData[inputs[p.idx].dataOffset], &slot, sizeof(slot));
  dirty = true;
}
}
}
