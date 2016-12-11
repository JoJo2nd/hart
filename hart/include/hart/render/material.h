/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#pragma once

#include "hart/config.h"
#include "hart/base/util.h"
#include "hart/base/std.h"
#include "hart/core/objectfactory.h"
#include "hart/core/resourcemanager.h"
#include "hart/render/state.h"
#include "hart/render/program.h"
#include "hart/render/technique.h"
#include "hart/render/texture.h"
#include "hart/base/vec.h"
#include "hart/base/matrix.h"
#include "hart/fbs/material_generated.h"
#include "hart/fbs/materialsetup_generated.h"

namespace bgfx {
    struct TextureHandle;
}

namespace hart {
namespace render {

typedef resource::MaterialInputData MaterialInputData;
class Shader;
typedef bgfx::TextureHandle Texture;

struct MaterialInputHandle {
	static const uint16_t Invalid = uint16_t(~0ul);
    bool isValid() { return type != Invalid && idx != Invalid; }
    uint16_t type = Invalid;
    uint16_t idx = Invalid;
};

struct MaterialHandleData {
    MaterialInputHandle handle;
    uint16_t dataOffset;
    uint16_t dataLen;
};

struct MaterialTextureSlot;

class Material {
	static const uint32_t InvalidIndex = uint32_t(~0ul);
    HART_OBJECT_TYPE(HART_MAKE_FOURCC('r','m','a','t'), resource::Material)
    public:
        typedef hresmgr::THandle<TextureRes> TextureResHandle;
        typedef hresmgr::TWeakHandle<TextureRes> TextureResWeakHandle;

        Material() = default;
        ~Material();

        MaterialInputHandle getInputParameterHandle(const char* name) {
            MaterialInputHandle r;
            for (uint16_t i=0, n=(uint16_t)inputs.size(); i<n; ++i) {
                if (hcrt::strcmp(inputs[i].name, name) == 0) {
                    r.type = (uint16_t)inputs[i].dataType;
                    r.idx = i;
                    return r;
                }
            }
            return r;
        }
        char const* getInputParameterName(MaterialInputHandle h) {
            return inputs[h.idx].name;
        }
        uint32_t getTechnqiuePassCount(TechniqueType in_type) { 
            uint32_t idx = getTechnqiueIndex(in_type);
            return (idx == InvalidIndex) ? 0 : (uint32_t)techniques[idx].passes.size();
        }
        Program getTechnqiuePassProgram(TechniqueType in_type, uint32_t pass) { 
            uint32_t idx = getTechnqiueIndex(in_type);
            return (idx == InvalidIndex) ? getInvalidProgram() : techniques[idx].passes[pass].program;
        }
        State getTechnqiuePassState(TechniqueType in_type, uint32_t pass) { 
            uint32_t idx = getTechnqiueIndex(in_type);
            return (idx == InvalidIndex) ? State() : techniques[idx].passes[pass].state;
        }
        void setParameters(MaterialHandleData const* hrestrict in_data, uint32_t in_data_count, uint8_t const* hrestrict base_add);
    private:
        uint32_t getTechnqiueIndex(TechniqueType in_type) { 
            for (uint32_t i=0, n=(uint32_t)techniques.size(); i<n; ++i) {
                if (techniques[i].type == in_type)
                    return i;
            }
            return ~0ul;
        }

        typedef hresmgr::TWeakHandle<Shader> ShaderResHandle;

        struct Pass {
            State   state;
            ShaderResHandle vertex;
            ShaderResHandle pixel;
            Program program;
        };

        struct Technique {
            TechniqueType type;
            hstd::vector<Pass> passes;
        };

        typedef bgfx::UniformHandle UniformHandle;

        struct Input {
            static const uint16_t Invalid = uint16_t(~0ul);
            const char* name = nullptr;
            MaterialInputData dataType = resource::MaterialInputData_NONE;
            uint16_t dataIdx : 15;
            uint16_t set : 1;
            UniformHandle uniform;
        };


        hstd::vector<Technique> techniques;
        hstd::vector<TextureResHandle> boundTextures;
        hstd::vector<uint8_t> inputData;
        hstd::vector<Input> inputs;
};

class MaterialSetup {
    HART_OBJECT_TYPE(HART_MAKE_FOURCC('r','m','s','t'), resource::MaterialSetup)
    public:
        void flushParameters(bool force) {
            if (!dirty && !force) return;
            material->setParameters(inputs.data(), (uint32_t)inputs.size(), inputData.data());
            dirty = false;
        }
        MaterialInputHandle getInputParameterHandle(const char* name) {
            MaterialInputHandle h;
            for (uint32_t i=0, n=(uint32_t)inputs.size(); i<n; ++i) {
                if (hcrt::strcmp(material->getInputParameterName(inputs[i].handle), name) == 0) {
                    h.type = inputs[i].handle.type;
                    h.idx = i;
                    return h;
                }
            }
            return h;
        }
        void setParameter(MaterialInputHandle p, hMat44 const* d) {
            hdbassert(p.isValid() && p.type == resource::MaterialInputData_Mat44Input, "Invalid parameter handle!\n");
            hcrt::memcpy(&inputData[inputs[p.idx].dataOffset], &d, sizeof(hMat44));
            dirty = true;
        }
        void setParameter(MaterialInputHandle p, hVec4 const* d) {
            hdbassert(p.isValid() && p.type == resource::MaterialInputData_Vec4Input, "Invalid parameter handle!\n");
            hcrt::memcpy(&inputData[inputs[p.idx].dataOffset], &d, sizeof(hVec4));
            dirty = true;
        }
        void setParameter(MaterialInputHandle p, Texture const* d);
        Material* getMaterial() { return material.getData(); } 
    private:
        typedef MaterialHandleData Input;
        typedef hresmgr::TWeakHandle<Material> MaterialResHandle;

        hstd::vector<Input> inputs;
        hstd::vector<uint8_t> inputData;
        MaterialResHandle material;
        bool dirty = true;
};

}
}

namespace hrnd = hart::render;