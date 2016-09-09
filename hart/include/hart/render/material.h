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
#include "hart/base/vec.h"
#include "hart/base/matrix.h"
#include "hart/fbs/material_generated.h"
#include "hart/fbs/materialsetup_generated.h"

namespace hart {
namespace render {

typedef resource::TechniqueType TechniqueType;
typedef resource::MaterialInputData MaterialInputData;
class Shader;
struct Program;

struct MaterialInputHandle {
	static const uint16_t Invalid = uint16_t(~0ul);
    bool isValid() { return type != Invalid && idx != Invalid; }
    uint16_t type = Invalid;
    uint16_t idx = Invalid;
};

class Material {
	static const uint32_t InvalidIndex = uint32_t(~0ul);
    HART_OBJECT_TYPE(HART_MAKE_FOURCC('r','m','a','t'), resource::Material)
    public:
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
        uint32_t getTechnqiuePassCount(TechniqueType in_type) { 
            uint32_t idx = getTechnqiueIndex(in_type);
            return (idx == InvalidIndex) ? 0 : (uint32_t)techniques[idx].passes.size();
        }
        Program* getTechnqiuePassProgram(TechniqueType in_type, uint32_t pass) { 
            uint32_t idx = getTechnqiueIndex(in_type);
            return (idx == InvalidIndex) ? nullptr : techniques[idx].passes[pass].program;
        }
        State getTechnqiuePassState(TechniqueType in_type, uint32_t pass) { 
            uint32_t idx = getTechnqiueIndex(in_type);
            return (idx == InvalidIndex) ? State() : techniques[idx].passes[pass].state;
        }
    private:
        uint32_t getTechnqiueIndex(TechniqueType in_type) { 
            for (uint32_t i=0, n=(uint32_t)techniques.size(); i<n; ++i) {
                if (techniques[i].type == in_type)
                    return i;
            }
            return ~0ul;
        }

        struct Pass {
            State   state;
            Shader* vertex;
            Shader* pixel;
            Program* program;
        };

        struct Technique {
            TechniqueType type;
            hstd::vector<Pass> passes;
        };

        struct Input {
            const char* name = nullptr;
            MaterialInputData dataType = resource::MaterialInputData_NONE;
            uint16_t dataIdx = ~0;
		};

        hstd::vector<Technique> techniques;
        struct {
            hstd::vector<hMat44> matrix;
            hstd::vector<hVec4> vec;
            hstd::vector<hresmgr::Handle> texture;
        } inputsData;
        hstd::vector<Input> inputs;
};

class MaterialSetup {
    HART_OBJECT_TYPE(HART_MAKE_FOURCC('r','m','s','t'), resource::MaterialSetup)
    public:
        void flushParameters() {/*TODO:*/}
        MaterialInputHandle getInputParameterHandle(const char* name);
        void setParameter(MaterialInputHandle p, hMat44 const& d);
        void setParameter(MaterialInputHandle p, hVec4 const& d);
        //TODO: void setParameter(MaterialInputHandle p, Texture d);
        Material* getMaterial() { return material; } 
    private:
        struct Input {
            MaterialInputHandle handle;
            uint16_t dataOffset;
            uint16_t dataLen;
		};
        Material* material = nullptr;
        hstd::vector<Input> inputs;
        hstd::vector<uint8_t> inputData;
};

}
}
