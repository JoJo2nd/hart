/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"
#include "hart/base/util.h"
#include "hart/core/objectfactory.h"
#include "hart/fbs/texture_generated.h"
#include "bgfx/bgfx.h"

namespace hart {
namespace render {

typedef bgfx::TextureHandle Texture;

class TextureRes {
    HART_OBJECT_TYPE(HART_MAKE_FOURCC('t','e','x','r'), resource::Texture)
    public:
        ~TextureRes();
        
        Texture getTexture() const { return texture; }

    private:
        uint32_t width, height, depth;
        uint32_t mips;
        TextureType type;
        TextureFormat format;
        Texture texture;
};

Texture createTexture2D(uint16_t width, uint16_t height, uint8_t numMips, TextureFormat format, uint32_t flags, void const* mem, uint32_t memlen);
inline void destroyTexture(Texture in_texture) {
    bgfx::destroyTexture(in_texture);
}

}
}

namespace hrnd = hart::render;
