/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "bgfx/bgfx.h"
#include "hart/base/util.h"
#include "hart/config.h"
#include "hart/core/objectfactory.h"
#include "hart/fbs/texture_generated.h"

namespace hart {
namespace render {

typedef bgfx::TextureHandle Texture;

class TextureRes {
  HART_OBJECT_TYPE(HART_MAKE_FOURCC('t', 'e', 'x', 'r'), resource::Texture)
public:
  ~TextureRes();

  uint32_t      width, height, depth;
  uint32_t      mips;
  TextureType   type;
  TextureFormat format;
  Texture       texture;
};

Texture createTexture2D(uint16_t width, uint16_t height, uint8_t numMips, TextureFormat format, uint32_t flags,
                        void const* mem, uint32_t memlen);
Texture createTexture(void const* raw_data, uint32_t data_len);
inline void destroyTexture(Texture in_texture) {
  bgfx::destroyTexture(in_texture);
}
}
}

namespace hrnd = hart::render;
