/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#pragma once

#include "hart/config.h"
#include "hart/base/util.h"
#include "hart/core/objectfactory.h"
#include "hart/fbs/shader_generated.h"

namespace hart {
namespace render {

class Shader {
    HART_OBJECT_TYPE(HART_MAKE_FOURCC('s','d','r','c'), resource::ShaderCollection)
    public:
        //stuff

};

}
}

namespace hrnd = hart::render;
