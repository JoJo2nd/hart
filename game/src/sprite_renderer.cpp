/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "sprite_renderer.h"

struct RndSprite {
    hrnd::IndexBuffer ib;
    hrnd::VertexBuffer vb;
    hrnd::Texture t;
};

static const uint16_t FreeBit = 0x8000; // limits us to 32687 handles. Probably not an issue.
static const uint16_t InvalidHandle = 0xFFFF; // limits us to 32686 handles. Probably not an issue.
static const uint32_t LayerCount = LayerType_MAX+1;

static struct SpriteRenderer {
    static uint32_t MaxSprites;
    hstd::vector<uint16_t> handles;
    hstd::vector<uint16_t> slots;
    hstd::vector<RndSprite> rndSprites;
    hstd::vector<hVec4> rndSpriteBounds;
    uint32_t handleFLHead = 0;
    uint32_t spriteCount = 0;
} sr[LayerCount];
static hrnd::MaterialSetup* material;
static hrnd::MaterialInputHandle textureInputHandle;
static hresmgr::THandle<hrnd::MaterialSetup, hresmgr::HandleNonCopyable> materialHdl;
uint32_t SpriteRenderer::MaxSprites = 4096;

void initialiseSpriteRenderer() {
    SpriteRenderer::MaxSprites = 4096; // Load from config vars?       
    static huuid::uuid_t material_id = huuid::fromDwords(0x682625d0ef63442e,0xa4a121a0b24a2d17);

    // get the loaded pointers
    hresmgr::loadResource(material_id, &materialHdl);
    materialHdl.loaded();
    material = materialHdl.getData();
    hdbassert(material, "Sprite material isn't loaded. It should be loaded during startup.\n");
    textureInputHandle = material->getInputParameterHandle("s_tex");
    hdbassert(textureInputHandle.isValid(), "Sprite material is missing s_tex texture input.\n");

    for (uint32_t sl=0; sl < LayerCount; ++sl) {
        sr[sl].handles.resize(SpriteRenderer::MaxSprites, FreeBit);
        sr[sl].slots.resize(SpriteRenderer::MaxSprites, InvalidHandle);
        sr[sl].rndSprites.resize(SpriteRenderer::MaxSprites);
        sr[sl].rndSpriteBounds.resize(SpriteRenderer::MaxSprites);

        //init the free lists
        for (uint16_t i=0, n=(uint16_t)sr[sl].handles.size(); i < n; ++i) {
           sr[sl].handles[i] = i+1 < n ? (uint16_t)(i+1 | FreeBit) : InvalidHandle;
        }
    }
}

SpriteHandle createSprite(LayerType layer_index) {
    hdbassert(sr[layer_index].spriteCount < SpriteRenderer::MaxSprites, "Ran out of sprites to allocate from");
    SpriteHandle hdl = InvalidHandle;
    hdl = sr[layer_index].handleFLHead & ~FreeBit;
    sr[layer_index].handleFLHead = sr[layer_index].handles[hdl] & ~FreeBit;

    sr[layer_index].handles[hdl] = sr[layer_index].spriteCount;
    sr[layer_index].slots[sr[layer_index].spriteCount] = hdl;
    ++sr[layer_index].spriteCount;
    return (layer_index << 16) | hdl;
}

void destroySprite(SpriteHandle hdl) {
    uint16_t li = (uint16_t)((hdl & 0xFFFF0000) >> 16);
    hdbassert(sr[li].spriteCount, "calling destroy Sprite when no sprites exist in this layer (%d)", li);
    uint16_t h = (hdl & 0xFFFF);
    uint16_t s = sr[li].handles[h];    
    //mark the handle free and add to the freelist
    sr[li].handles[h] = sr[li].handleFLHead;
    sr[li].handleFLHead = h | FreeBit;
    //compact down the slots
    --sr[li].spriteCount;
    if (sr[li].spriteCount) {
        sr[li].rndSprites[s] = sr[li].rndSprites[sr[li].spriteCount];
        sr[li].rndSpriteBounds[s] = sr[li].rndSpriteBounds[sr[li].spriteCount];
        sr[li].slots[s] = sr[li].slots[sr[li].spriteCount];
        sr[li].handles[sr[li].slots[s]] = s;
    }
}

void setSpriteRenderData(SpriteHandle hdl, hrnd::IndexBuffer ib, hrnd::VertexBuffer vb, hrnd::Texture tex) {
    uint16_t li = (uint16_t)((hdl & 0xFFFF0000) >> 16);
    uint16_t h = (hdl & 0xFFFF);
    uint16_t i = sr[li].handles[h];
    sr[li].rndSprites[i].ib = ib;
    sr[li].rndSprites[i].vb = vb;
    sr[li].rndSprites[i].t = tex;
}

void setSpriteTexture(SpriteHandle hdl, hrnd::Texture tex) {
    uint16_t li = (uint16_t)((hdl & 0xFFFF0000) >> 16);
    uint16_t h = (hdl & 0xFFFF);
    uint16_t i = sr[li].handles[h];
    sr[li].rndSprites[i].t = tex;
}

void setSpriteBounds(SpriteHandle hdl, hVec4 sprite_rect) {
    uint16_t li = (uint16_t)((hdl & 0xFFFF0000) >> 16);
    uint16_t h = (hdl & 0xFFFF);
    uint16_t i = sr[li].handles[h];
    sr[li].rndSpriteBounds[i] = sprite_rect;
}

void renderSprites(uint32_t width, uint32_t height) {
    hMat44 ortho;
    ortho = hMat44::orthographic(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
    hrnd::begin(hrnd::View_Main, hrnd::TechniqueType::TechniqueType_Main, nullptr, &ortho);
    hrnd::setMaterialSetup(material);
    for (uint32_t sl=0; sl < LayerCount; ++sl) {
        for (uint32_t s=0, sn=sr[sl].spriteCount; s < sn; ++s) {
            //update texture
            material->setParameter(textureInputHandle, &sr[sl].rndSprites[s].t);
            hrnd::submit(sr[sl].rndSprites[s].ib, sr[sl].rndSprites[s].vb);
        }
    }
    hrnd::end();
}