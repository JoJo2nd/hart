/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/render/render.h"
#include "hart/base/freelist.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include "bgfx/bgfx.h"
#include "bgfx/bgfxplatform.h"    

namespace hart {
namespace render {

struct VertexDecl {
    bgfx::VertexDecl decl;
};

static struct {
    uint16_t currentView;
    TechniqueType activeTech;
    MaterialSetup* currentMatSetup;
    //TODO: IndexBuffer* idxBuffer;
    //TODO: VertexBuffer* vtxBuffer;
    struct {
        uint16_t x, y, w, h;
    } scissor;
} drawCtx;
static bgfx::Caps gfxCaps;
static hstd::vector<uint32_t> viewIDRemap;
static hart::Freelist<VertexDecl, 256> vertexDeclFreelist;
static bgfx::Attrib::Enum bgfxAttribRemap[] = {
    bgfx::Attrib::Position,  // from Semantic::Position
    bgfx::Attrib::Normal,    // from Semantic::Normal
    bgfx::Attrib::Tangent,   // from Semantic::Tangent
    bgfx::Attrib::Bitangent, // from Semantic::Bitangent
    bgfx::Attrib::Color0,    // from Semantic::Color0
    bgfx::Attrib::Color1,    // from Semantic::Color1
    bgfx::Attrib::TexCoord0, // from Semantic::TexCoord0
    bgfx::Attrib::TexCoord1, // from Semantic::TexCoord1
    bgfx::Attrib::TexCoord2, // from Semantic::TexCoord2
    bgfx::Attrib::TexCoord3, // from Semantic::TexCoord3
    bgfx::Attrib::TexCoord4, // from Semantic::TexCoord4
    bgfx::Attrib::TexCoord5, // from Semantic::TexCoord5
    bgfx::Attrib::TexCoord6, // from Semantic::TexCoord6
    bgfx::Attrib::TexCoord7, // from Semantic::TexCoord7
}; //remap
static bgfx::AttribType::Enum bgfxAttribTypeRemap[] = {
    bgfx::AttribType::Uint8, // from SemanticType::Uint8
    bgfx::AttribType::Int16, // from SemanticType::Int16
    bgfx::AttribType::Half,  // from SemanticType::Half
    bgfx::AttribType::Float, // from SemanticType::Float
};
static bgfx::BackbufferRatio::Enum bgfxBBRationReamp[] = {
    bgfx::BackbufferRatio::Equal,     //  from Ratio::Same,     
    bgfx::BackbufferRatio::Half,      //  from Ratio::Half,     
    bgfx::BackbufferRatio::Quarter,   //  from Ratio::Quarter,  
    bgfx::BackbufferRatio::Eighth,    //  from Ratio::Eighth,   
    bgfx::BackbufferRatio::Sixteenth, //  from Ratio::Sixteenth,
    bgfx::BackbufferRatio::Double,    //  from Ratio::Double,   
};

void initialise(SDL_Window* window) {
    bgfx::sdlSetWindow(window);
    bgfx::renderFrame(); // calling this before bgfx::init prevents the render thread being created
    bgfx::init(bgfx::RendererType::Direct3D11);
    gfxCaps = *bgfx::getCaps();
}

void resetViews(ViewDef* views, size_t count) {
    viewIDRemap.clear();
    for (uint32_t i=0, n=gfxCaps.maxViews; i<n; ++i) {
        bgfx::resetView(i);
    }
    for (uint32_t i=0, n=uint32_t(count); i<n; ++i) {
        if (viewIDRemap.size() < views[i].id) {
            viewIDRemap.resize(views[i].id+1, ~0ul);
        }
        viewIDRemap[views[i].id] = i;
        uint32_t flags = 0;
        if (views[i].clearColour) flags |= BGFX_CLEAR_COLOR;
        if (views[i].clearDepth) flags |= BGFX_CLEAR_DEPTH;
        if (views[i].clearStencil) flags |= BGFX_CLEAR_STENCIL;
        bgfx::setViewClear(0, flags, views[i].colourValue, views[i].depthValue, views[i].stencilValue);
        if (!views[i].useRatio)
            bgfx::setViewRect(i, 0, 0, views[i].w, views[i].h);
        else
            bgfx::setViewRect(i, 0, 0, bgfxBBRationReamp[uint32_t(views[i].ratio)]);

    }
}

VertexDecl* createVertexDecl(VertexElement const* elements, uint16_t element_count) {
    VertexDecl* vd = new (vertexDeclFreelist.allocate()) VertexDecl;
    vd->decl.begin();
    for (uint16_t i=0; i<element_count; ++i) {
        vd->decl.add(
            bgfxAttribRemap[(uint32_t)elements[i].sem], elements[i].elementCount, 
            bgfxAttribTypeRemap[(uint32_t)elements[i].semType], elements[i].normalized);
    }
    vd->decl.end();
    return vd;
}

void destroyVertexDecl(VertexDecl* vd) {
    vd->~VertexDecl();
    vertexDeclFreelist.release(vd);
}

void begin(uint16_t view_id, TechniqueType tech) {
    drawCtx.currentView = viewIDRemap[view_id];
    drawCtx.activeTech = tech;
    drawCtx.currentMatSetup = nullptr;
}

void end() {

}

}
}