/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/hart.h"
#include "sprite_renderer.h"
#include "poly_utils.h"
#include "fbs/tileset_generated.h"
#include "fbs/level_generated.h"

class Tileset {
    HART_OBJECT_TYPE(HART_MAKE_FOURCC('t','s','e','t'), resource::TileSet)
public:

    struct Tile {
        static const uint16_t InvalidAnimIndex = (uint16_t)~0ul;
        enum {
            Flag_Transparent    = 0x0001,        
            Flag_Animated       = 0x0002,
        };

        uint16_t flags = 0;
        uint16_t animationIndex = InvalidAnimIndex;
        float    uvs[4];// packed uvs (u1, v1, u2, v2)
    };

    struct AnimFrame {
        uint16_t id;      // tileID
        uint16_t tickLen; // frame len in ticks (1 tick == 1 ms)
    };

    Tileset() = default;
    //TODO: look into move constructor
    //Tileset(Tileset const& rhs) = delete;
    //Tileset& operator = (Tileset const& rhs) = delete;
    ~Tileset();

    typedef std::vector<AnimFrame> AnimSequence;
    typedef hresmgr::TWeakHandle<hrnd::TextureRes> TextureResWeakHandle;

    uint32_t tileWidth; // in pixels 
    uint32_t tileHeight; // in pixels
    uint32_t tileCount;
    uint32_t tileColumns;
    uint32_t spacing; // in pixels
    uint32_t margin; // in pixels
    uint32_t firstGID; // when referencing a tile from a level, tiles[(tileLevelId - firstGID)]
    TextureResWeakHandle textureResource; 
    std::vector<Tile> tiles;
    std::vector<AnimSequence> animations;
};

class Level {
    HART_OBJECT_TYPE(HART_MAKE_FOURCC('l','v','l','_'), resource::Level)
public:
    typedef hresmgr::TWeakHandle<hety::Entity> EntityResWeakHandle;

    Level() = default;
    Level(Level const& rhs) = delete;
    Level& operator = (Level const& rhs) = delete;
    ~Level();

    Tileset::Tile const* getTile(uint32_t tile_id, uint32_t* tw, uint32_t* th, Tileset** ts);
    void updateTileAnimations();

    struct EntityData {
        EntityResWeakHandle entity;
        uint32_t      id;
    };

    struct AnimSprite {
        Tileset::AnimSequence* seq = nullptr;
        uint16_t offset = 0; // In quads, allows addressing index & vertex buffer
        uint16_t playTicks = 0;
        uint16_t frame = 0;
    };

    struct AnimSpriteBlock {
        AnimSpriteBlock() = default;
        AnimSpriteBlock(AnimSpriteBlock&&) = default;
        ~AnimSpriteBlock() = default;

        hstd::vector<AnimSprite> sprites;
        hstd::unique_ptr<uint8_t> animData;
        Tileset* tileset;
        uint32_t animDataLen;
        SpriteHandle sprite;
        hrnd::IndexBuffer indexBuffer;
        hrnd::VertexBuffer vertexBuffer;
    };

    hrnd::VertexDecl* vDecl;
    hstd::vector<Tileset> tilesets;
    hstd::vector<EntityData> entities;
    hstd::vector<SpriteHandle> sprites;
    hstd::vector<AnimSpriteBlock> animSpriteBlocks;

    //TEMP
    hstd::vector<AABB> collisionQuads;
};

void updateAllLevelAnimations();
