/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include "bgfx/bgfx.h"
#include "bgfx/bgfxplatform.h" 

namespace hart {
namespace render {

class Shader;
// bgfx Typedefs. I may in future not use bgfx but it's a loong way off
typedef bgfx::ProgramHandle Program;

Program createProgram(Shader* vertex, Shader* pixel);
inline Program getInvalidProgram() {
    Program p = BGFX_INVALID_HANDLE;
    return p;
}
inline void destroyProgram(Program p) {
    if (!isValid(p)) return;
    bgfx::destroyProgram(p);
}

}
}
