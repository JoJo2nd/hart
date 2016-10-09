/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "level.h"

HART_OBJECT_TYPE_DECL(Tileset);
HART_OBJECT_TYPE_DECL(Level);

//HACK: fudge to test level animations. clean up
static hstd::vector<Level*> loadedLevels;

void registierLevelObject() {
    hobjfact::objectFactoryRegister(Tileset::getObjectDefinition(), nullptr);
    hobjfact::objectFactoryRegister(Level::getObjectDefinition(), nullptr);
}

Tileset::~Tileset() {
}

bool Tileset::deserialiseObject(MarshallType const* in_data, hobjfact::SerialiseParams const& params) {
    tileWidth = in_data->tilewidth();
    tileHeight = in_data->tileheight();
    tileCount = in_data->tilecount();
    tileColumns = in_data->tilecolumns();
    spacing = in_data->spacing();
    margin = in_data->margin();
    firstGID = in_data->firstgid();

    textureResource = hresmgr::tweakGetResource<hrnd::TextureRes>(huuid::fromData(*in_data->imageasset()));

    auto const* in_animations = in_data->animations();
    animations.resize(in_animations->size());
    for (uint32_t i=0, n=(uint32_t)animations.size(); i < n; ++i) {
        resource::TileAnimations const* in_anim = (*in_animations)[i];
        animations[i].resize(in_anim->frames()->size());
        for (uint32_t f=0, fn=(in_anim->frames()->size()); f < fn; ++f) {
            animations[i][f].id = (*in_anim->frames())[f]->id();
            animations[i][f].tickLen = (*in_anim->frames())[f]->len();
        }
    }

    tiles.resize(tileCount);
    uint32_t cur_col = 0;
    uint32_t cur_x = 0, tw = textureResource->getWidth(); //in pixels
    uint32_t cur_y = 0, th = textureResource->getHeight(); //in pixels
    auto const* in_tiles = in_data->tiles();
    // Initialise tiles that have any properties (only tile with props are stored)
    for (uint32_t i=0, n=in_tiles->size(); i < n; ++i) {
        resource::Tile const* in_t = (*in_tiles)[i];

        uint16_t t_id = in_t->id();
        Tile* t = &tiles[t_id];
        if (in_t->transparent()) t->flags |= Tile::Flag_Transparent;
        if (in_t->animated()) {
            t->flags |= Tile::Flag_Animated;
            for (uint32_t a=0, an = (uint32_t)animations.size(); a < an; ++a) {
                if (animations[a][0].id == t_id) {
                    t->animationIndex = a;
                    break;
                }
            }
        }
    }
    // setup all tile info
    for (uint32_t i=0, n=tileCount; i < n; ++i) {
        Tile* t = &tiles[i];
        t->uvs[0] = float(cur_x + margin)/float(tw);
        t->uvs[1] = float(cur_y + margin)/float(th);
        t->uvs[2] = float(cur_x + margin + tileWidth)/float(tw);
        t->uvs[3] = float(cur_y + margin + tileHeight)/float(th);

        if (cur_col >= (tileColumns-1)) {
            cur_x = 0;
            cur_col = 0;
            cur_y += tileHeight + spacing;
        } else {
            cur_x += tileWidth + spacing;
            ++cur_col;
        }
    }

    return true;
}

Level::~Level() {
    hdbfatal("STUB, this needs writing to release resources created in deserialiseObject\n");
}

bool Level::deserialiseObject(MarshallType const* in_data, hobjfact::SerialiseParams const& params) {

    auto const* in_tilesets = in_data->tilesets();
    tilesets.resize(in_tilesets->size());
    for (uint32_t i=0, n=in_tilesets->size(); i < n; ++i) {
        tilesets[i].deserialiseObject((*in_tilesets)[i], params);
    }

    auto const* in_entities = in_data->entities();
    entities.resize(in_entities->size());
    for (uint32_t i=0, n=in_entities->size(); i<n; ++i) {
        entities[i].id = (*in_entities)[i]->levelid();
        entities[i].entity = hresmgr::tweakGetResource<hety::Entity>(huuid::fromData(*(*in_entities)[i]->assetuuid()));
    }

    // Build our layer tiles. 32x32 each. Two for each layer; one static, one dynamic.
    AnimSpriteBlock tmp_anim_block;
    auto const* in_layers = in_data->layers();
    hrnd::VertexElement elements[] = {
        {hrnd::Semantic::Position,  hrnd::SemanticType::Float, 2, false},
        {hrnd::Semantic::TexCoord0, hrnd::SemanticType::Float, 2, false},
    };
    vDecl = hrnd::createVertexDecl(elements, (uint16_t)HART_ARRAYSIZE(elements));
    static const uint32_t quadCount = 32*32;
    static const uint32_t indexCount = 6;
    static const uint32_t vtxCount = 4;
    struct Vtx {
        float x,y;
        float u,v;
    };
    Vtx tmp_vb[quadCount*vtxCount], tmp_anim_vb[quadCount*vtxCount];
    uint16_t tmp_ib[quadCount*indexCount], tmp_anim_ib[quadCount*indexCount];
    for (uint32_t l=0, ln=in_layers->size(); l < ln; ++l) {
        uint32_t l_width = (*in_layers)[l]->width();
        uint32_t l_height = (*in_layers)[l]->height();
        LayerType l_layer = (*in_layers)[l]->name();
        uint16_t const* t_data = (*in_layers)[l]->tiles()->data();

        for (uint32_t y=0; y < l_height; y += 32) {
            for (uint32_t x=0; x < l_width; x += 32) {
                for (uint32_t tsi = 0, tsn = (uint32_t)tilesets.size(); tsi<tsn; ++tsi) {
                    Tileset* cur_ts = &tilesets[tsi];
                    Vtx* v_ptr = tmp_vb,* va_ptr = tmp_anim_vb;
                    uint16_t* i_ptr = tmp_ib,* ia_ptr = tmp_anim_ib;
                    uint16_t base_vtx = 0,base_avtx = 0,quads = 0, animquads = 0;
                    tmp_anim_block.sprites.clear();

                    for (uint32_t yy=0; yy < 32; ++yy) {
                        for (uint32_t xx=0; xx < 32; ++xx) {
                            if (yy+y >= l_height) continue;
                            if (xx+x >= l_width) continue;
                            uint32_t loc = ((yy+y)*l_width) + (xx+x);
                            uint32_t tw, th;
                            Tileset* ts;
                            if (Tileset::Tile const* t = getTile(t_data[loc], &tw, &th, &ts)) {
                                if (ts != cur_ts) continue; // not of this tileset, so not added to this sprite
                                uint16_t* li_ptr = nullptr;
                                Vtx* lv_ptr = nullptr;
                                uint16_t vtxstart;
                                if (t->flags & Tileset::Tile::Flag_Animated) {
                                    li_ptr = ia_ptr; lv_ptr = va_ptr; 
                                    ia_ptr += indexCount;
                                    va_ptr += vtxCount;
                                    vtxstart = base_avtx;
                                    base_avtx += vtxCount;
                                    AnimSprite as;
                                    as.seq = &ts->animations[t->animationIndex];
                                    as.offset = animquads++;
                                    as.playTicks = 0;
                                    tmp_anim_block.sprites.push_back(as);
                                } else {
                                    li_ptr = i_ptr; lv_ptr = v_ptr;
                                    i_ptr += indexCount;
                                    v_ptr += vtxCount;
                                    vtxstart = base_vtx;
                                    base_vtx += vtxCount;
                                    ++quads;
                                }
                                li_ptr[0] = vtxstart+0; li_ptr[3] = vtxstart+2;
                                li_ptr[1] = vtxstart+1; li_ptr[4] = vtxstart+1;
                                li_ptr[2] = vtxstart+2; li_ptr[5] = vtxstart+3;
                                
                                lv_ptr[0].x = (float)(x+xx)*tw;      lv_ptr[0].y = (float)(y+yy)*th;    lv_ptr[0].u = t->uvs[0];     lv_ptr[0].v = t->uvs[1];
                                lv_ptr[1].x = (float)(x+xx+1)*tw;    lv_ptr[1].y = (float)(y+yy)*th;    lv_ptr[1].u = t->uvs[2];     lv_ptr[1].v = t->uvs[1];
                                lv_ptr[2].x = (float)(x+xx)*tw;      lv_ptr[2].y = (float)(y+yy+1)*th;  lv_ptr[2].u = t->uvs[0];     lv_ptr[2].v = t->uvs[3];
                                lv_ptr[3].x = (float)(x+xx+1)*tw;    lv_ptr[3].y = (float)(y+yy+1)*th;  lv_ptr[3].u = t->uvs[2];     lv_ptr[3].v = t->uvs[3];   
                            }
                        }
                    }
                    //create a tile block sprite
                    if (cur_ts && quads > 0) {
                        hrnd::IndexBuffer qib = hrnd::createIndexBuffer(tmp_ib, quads*indexCount*sizeof(uint16_t), 0);
                        hrnd::VertexBuffer qvb = hrnd::createVertexBuffer(tmp_vb, quads*vtxCount*sizeof(Vtx), vDecl, 0);
                        SpriteHandle sh = createSprite(l_layer);
                        setSpriteRenderData(sh, qib, qvb, cur_ts->textureResource->getTexture());
                        sprites.push_back(sh);
                    }
                    //create animated tile block sprite
                    if (cur_ts && animquads > 0) {
                        hrnd::IndexBuffer qib = hrnd::createIndexBuffer(tmp_anim_ib, animquads*indexCount*sizeof(uint16_t), hrnd::Flag_IndexBuffer_Dynamic);
                        hrnd::VertexBuffer qvb = hrnd::createVertexBuffer(tmp_anim_vb, animquads*vtxCount*sizeof(Vtx), vDecl, hrnd::Flag_VertexBuffer_Dynamic);
                        uint8_t* anim_data = new uint8_t[animquads*vtxCount*sizeof(Vtx)];
                        hcrt::memcpy(anim_data, tmp_anim_vb, animquads*vtxCount*sizeof(Vtx));
                        SpriteHandle sh = createSprite(l_layer);
                        setSpriteRenderData(sh, qib, qvb, cur_ts->textureResource->getTexture());
                        AnimSpriteBlock asb;
                        asb.sprite = sh;
                        asb.tileset = cur_ts;
                        asb.indexBuffer = qib;
                        asb.vertexBuffer = qvb;
                        asb.sprites = tmp_anim_block.sprites;
                        asb.animData.reset(anim_data);
                        asb.animDataLen = animquads*vtxCount*sizeof(Vtx);
                        animSpriteBlocks.push_back(std::move(asb));
                    }
                }
            }
        }
    }

    loadedLevels.push_back(this);

    return true;
}

Tileset::Tile const* Level::getTile(uint32_t tile_id, uint32_t* tw, uint32_t* th, Tileset** ts) {
    hdbassert(tw && th, "Invalid args\n");
    if (tile_id == 0) return nullptr;

    for (uint32_t i=0, n=(uint32_t)tilesets.size(); i < n; ++i) {
        if (tile_id >= tilesets[i].firstGID && tile_id < tilesets[i].firstGID+tilesets[i].tileCount) {
            *tw = tilesets[i].tileWidth;
            *th = tilesets[i].tileHeight;
            *ts = &tilesets[i];
            return &tilesets[i].tiles[tile_id-tilesets[i].firstGID];
        }
    }

    return nullptr;
}

void Level::updateTileAnimations() {
    //Hack to cope with a delta so quick, converting to int results in zero.
    //TODO: only tick game at 60Hz?
    static float fdeltams = 0.f;
    fdeltams += htime::deltaMS();
    uint32_t deltams = (uint32_t)fdeltams;
    fdeltams -= (float)deltams;
    for (auto& i : animSpriteBlocks) {
        bool dirty = false;
        struct Vtx {
            float x,y;
            float u,v;
        } *vb_ptr = (Vtx*)i.animData.get();
        for (auto& anim : i.sprites) {
            anim.playTicks += deltams;
            if (anim.playTicks >= (*anim.seq)[anim.frame].tickLen) {
                anim.playTicks -= (*anim.seq)[anim.frame].tickLen;
                anim.frame = (anim.frame+1)%anim.seq->size();
                uint16_t nfid = (*anim.seq)[anim.frame].id;
                uint32_t basei = anim.offset*4; //4 verts in a quad
                if (Tileset::Tile const* t = &i.tileset->tiles[nfid]) {
                    vb_ptr[basei+0].u = t->uvs[0];     vb_ptr[basei+0].v = t->uvs[1];
                    vb_ptr[basei+1].u = t->uvs[2];     vb_ptr[basei+1].v = t->uvs[1];
                    vb_ptr[basei+2].u = t->uvs[0];     vb_ptr[basei+2].v = t->uvs[3];
                    vb_ptr[basei+3].u = t->uvs[2];     vb_ptr[basei+3].v = t->uvs[3]; 
                }
                dirty = true;
            }
        }
        if (dirty) {
            hrnd::updateVertexBuffer(i.vertexBuffer, vb_ptr, i.animDataLen);
        }
    }
}

void updateAllLevelAnimations() {
    for (auto& i : loadedLevels) {
        i->updateTileAnimations();
    }
}
