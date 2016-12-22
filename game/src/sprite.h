/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "fbs/sprite_generated.h"
#include "fbs/spriteanimtype_generated.h"
#include "hart/hart.h"

class Sprite {
  HART_OBJECT_TYPE(HART_MAKE_FOURCC('s', 'p', 'r', 't'), resource::Sprite)
public:
  Sprite() = default;
  Sprite(Sprite const& rhs) = delete;
  Sprite& operator=(Sprite const& rhs) = delete;
  ~Sprite();

  static const uint32_t MaxAnimTypes = SpriteAnimType_MAX + 1;

  struct Frame {
    hVec4    uvs;
    uint32_t page;
  };

  struct AnimFrame {
    uint32_t frame;
    float    lenght;
  };

  hstd::vector<hrnd::Texture> texturePages;
  hstd::vector<Frame>         frames;
  hstd::vector<AnimFrame>     anims[MaxAnimTypes];
#if HART_DEBUG_INFO
  char const* friendlyName;
#endif
};
