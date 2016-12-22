/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/hart.h"
#include "fbs/layertype_generated.h"

typedef uint32_t SpriteHandle;

void initialiseSpriteRenderer();

SpriteHandle createSprite(LayerType layer_index);
void destroySprite(SpriteHandle hdl);

void setSpriteRenderData(SpriteHandle hdl, hrnd::IndexBuffer ib, hrnd::VertexBuffer vb, hrnd::Texture tex);
void setSpriteTexture(SpriteHandle hdl, hrnd::Texture tex);
void setSpriteBounds(SpriteHandle hdl, hVec4 sprite_rect);

void renderSprites(uint32_t width, uint32_t height);
