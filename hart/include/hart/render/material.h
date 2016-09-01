/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#pragma once

#include "hart/config.h"
#include "hart/base/util.h"
#include "hart/base/std.h"
#include "hart/core/objectfactory.h"
#include "hart/fbs/material_generated.h"
#include "hart/fbs/materialsetup_generated.h"

namespace hart {
namespace render {

class Shader;

class Material {
    HART_OBJECT_TYPE(HART_MAKE_FOURCC('r','m','a','t'), resource::Material)
    public:
        
    private:
        struct Pass {
            Shader* vertex;
            Shader* pixel;
        };

        struct Technique {
            hstd::string name;
            hstd::vector<Pass> passes;
        };

        hstd::vector<Technique> techniques;
};

class MaterialSetup {
    HART_OBJECT_TYPE(HART_MAKE_FOURCC('r','m','s','t'), resource::MaterialSetup)
    public:
        
    private:
        
};

}
}
