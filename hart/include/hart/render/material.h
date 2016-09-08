/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#pragma once

#include "hart/config.h"
#include "hart/base/util.h"
#include "hart/base/std.h"
#include "hart/core/objectfactory.h"
#include "hart/render/state.h"
#include "hart/fbs/material_generated.h"
#include "hart/fbs/materialsetup_generated.h"

namespace hart {
namespace render {

typedef resource::TechniqueType TechniqueType;
class Shader;
struct Program;

class Material {
    HART_OBJECT_TYPE(HART_MAKE_FOURCC('r','m','a','t'), resource::Material)
    public:
        Material() = default;
        ~Material();

        uint32_t getTechnqiuePassCount(TechniqueType in_type) { 
            uint32_t idx = getTechnqiueIndex(in_type);
            return (idx == ~0ul) ? 0 : (uint32_t)techniques[idx].passes.size();
        }
        Program* getTechnqiuePassProgram(TechniqueType in_type, uint32_t pass) { 
            uint32_t idx = getTechnqiueIndex(in_type);
            return (idx == ~0ul) ? nullptr : techniques[idx].passes[pass].program;
        }
        State getTechnqiuePassState(TechniqueType in_type, uint32_t pass) { 
            uint32_t idx = getTechnqiueIndex(in_type);
            return (idx == ~0ul) ? State() : techniques[idx].passes[pass].state;
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

        hstd::vector<Technique> techniques;
};

class MaterialSetup {
    HART_OBJECT_TYPE(HART_MAKE_FOURCC('r','m','s','t'), resource::MaterialSetup)
    public:
        void flushParameters() {/*TODO:*/}
        Material* getMaterial() { return material; } 
    private:
        Material* material = nullptr;
};

}
}
