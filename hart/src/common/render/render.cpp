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
    uint16_t stride;
    bgfx::VertexDecl decl;
};

struct Program {
    bgfx::ProgramHandle prog;
};

static struct {
    static const uint32_t MaxPasses = 16;

    uint8_t currentView;
    uint8_t currentPassCount;
    TechniqueType activeTech;
    MaterialSetup* currentMatSetup;
    //TODO: IndexBuffer* idxBuffer;
    //TODO: VertexBuffer* vtxBuffer;
    bgfx::TransientVertexBuffer tvb;
    bgfx::TransientIndexBuffer tib;
    bgfx::ProgramHandle currentProgram;
    struct {
        uint16_t x, y, w, h;
    } scissor;
    bool doingInlineSubmit : 1;
    bool dirtyScissor : 1;
    struct PassInfo {
        State state;
        Program* prog;
    } currentPass[MaxPasses];
} drawCtx;
static bgfx::Caps gfxCaps;
static resource::Profile currentProfile = resource::Profile_Direct3D11;
static hstd::vector<uint8_t> viewIDRemap;
static hart::Freelist<VertexDecl, 256> vertexDeclFreelist;
static hart::Freelist<Program> programFreelist;
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
static uint16_t bgfxAttribTypeSizes[] = {
    1, // from SemanticType::Uint8
    2, // from SemanticType::Int16
    2,  // from SemanticType::Half
    4, // from SemanticType::Float
};
static bgfx::BackbufferRatio::Enum bgfxBBRationReamp[] = {
    bgfx::BackbufferRatio::Equal,     //  from Ratio::Same,     
    bgfx::BackbufferRatio::Half,      //  from Ratio::Half,     
    bgfx::BackbufferRatio::Quarter,   //  from Ratio::Quarter,  
    bgfx::BackbufferRatio::Eighth,    //  from Ratio::Eighth,   
    bgfx::BackbufferRatio::Sixteenth, //  from Ratio::Sixteenth,
    bgfx::BackbufferRatio::Double,    //  from Ratio::Double,   
};


VertexDecl* createVertexDecl(VertexElement const* elements, uint16_t element_count) {
    VertexDecl* vd = new (vertexDeclFreelist.allocate()) VertexDecl;
    uint16_t stride = 0;
    vd->decl.begin();
    for (uint16_t i=0; i<element_count; ++i) {
        vd->decl.add(
            bgfxAttribRemap[(uint32_t)elements[i].sem], elements[i].elementCount, 
            bgfxAttribTypeRemap[(uint32_t)elements[i].semType], elements[i].normalized);
        stride += bgfxAttribTypeSizes[(uint32_t)elements[i].semType]*elements[i].elementCount;
    }
    vd->decl.end();
    vd->stride = stride;
    return vd;
}

void destroyVertexDecl(VertexDecl* vd) {
    if (!vd) return;

    vd->~VertexDecl();
    vertexDeclFreelist.release(vd);
}

Program* createProgram(Shader* vertex, Shader* pixel) {
    Program* p = new (programFreelist.allocate()) Program();
    p->prog = bgfx::createProgram(
        vertex->getShaderProfileObject(currentProfile),
        pixel->getShaderProfileObject(currentProfile));
    return p;
}

void destroyProgram(Program* p) {
    if (!p) return;

    bgfx::destroyProgram(p->prog);
    p->~Program();
    programFreelist.release(p);
}

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
            viewIDRemap.resize(views[i].id+1, ~0);
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

void begin(uint16_t view_id, TechniqueType tech, hMat44 const* view, hMat44 const* proj) {
    drawCtx.currentView = viewIDRemap[view_id];
    drawCtx.activeTech = tech;
    drawCtx.currentMatSetup = nullptr;
    drawCtx.doingInlineSubmit = false;
    drawCtx.dirtyScissor = false;
    bgfx::setViewTransform((uint8_t)drawCtx.currentView, view, proj);
}

void setScissor(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    drawCtx.scissor.x = x;
    drawCtx.scissor.y = y;
    drawCtx.scissor.w = w;
    drawCtx.scissor.h = h;
    drawCtx.dirtyScissor = true;
}

void setMaterialSetup(MaterialSetup* mat) {
    hdbassert(mat && mat->getMaterial(), "material pass must valid");
    drawCtx.currentMatSetup = mat;
    drawCtx.currentPassCount = (uint8_t)mat->getMaterial()->getTechnqiuePassCount(drawCtx.activeTech);
    for (uint8_t i=0, n=drawCtx.currentPassCount; i<n; ++i) {
        drawCtx.currentPass[i].state = mat->getMaterial()->getTechnqiuePassState(drawCtx.activeTech, i); 
        drawCtx.currentPass[i].prog = mat->getMaterial()->getTechnqiuePassProgram(drawCtx.activeTech, i);
    }
}

void submitInline(VertexDecl* fmt, void* idx, void* vtx, uint16_t numVertices, uint16_t prims) {
    hdbassert(fmt && vtx, "Invalid fmt or vtx parameter(s)\n");
    uint32_t numIndices = prims*3;
    if (!bgfx::checkAvailTransientVertexBuffer(numVertices, fmt->decl)) return;
    if (idx && !bgfx::checkAvailTransientIndexBuffer(numIndices)) return;

    bgfx::allocTransientVertexBuffer(&drawCtx.tvb, numVertices, fmt->decl);
    memcpy(drawCtx.tvb.data, vtx, numVertices * fmt->stride);
    bgfx::setVertexBuffer(&drawCtx.tvb, 0, numVertices);

    if (idx) {
        bgfx::allocTransientIndexBuffer(&drawCtx.tib, numIndices);
        memcpy(drawCtx.tib.data, idx, numIndices * sizeof(uint16_t));
        bgfx::setIndexBuffer(&drawCtx.tib, 0, prims);
    }

    if (drawCtx.dirtyScissor) {
        bgfx::setScissor(drawCtx.scissor.x, drawCtx.scissor.y, drawCtx.scissor.w, drawCtx.scissor.h);
    }
    drawCtx.dirtyScissor = false;
    for (uint8_t p=0, n=drawCtx.currentPassCount; p < n; ++p) {
        bgfx::setState(drawCtx.currentPass[p].state.stateMask);
        bgfx::submit(drawCtx.currentView, drawCtx.currentPass[p].prog->prog);
    }
}

void beginInlineBatch(VertexDecl* fmt, void* idx, uint32_t numIndices, void* vtx, uint16_t numVertices) {
    hdbassert(!drawCtx.doingInlineSubmit, "Can't nest inline batches");
    hdbassert(fmt && vtx, "Invalid fmt or vtx parameter(s)\n");
    
    if (bgfx::checkAvailTransientVertexBuffer(numVertices, fmt->decl)) {
        bgfx::allocTransientVertexBuffer(&drawCtx.tvb, numVertices, fmt->decl);
        memcpy(drawCtx.tvb.data, vtx, numVertices * fmt->stride);
    }

    if (idx && bgfx::checkAvailTransientIndexBuffer(numIndices)) {
        bgfx::allocTransientIndexBuffer(&drawCtx.tib, numIndices);
        memcpy(drawCtx.tib.data, idx, numIndices * sizeof(uint16_t));
    }

    drawCtx.doingInlineSubmit = true;
}

void inlineBatchSubmit(uint16_t ib_offset, uint32_t ib_count, uint16_t vb_offset, uint16_t vb_count) {
    hdbassert(drawCtx.doingInlineSubmit, "inlineBatchSubmit must be called between beginInlineBatch & endInlineBatch");
    bgfx::setVertexBuffer(&drawCtx.tvb, vb_offset, vb_count);
    bgfx::setIndexBuffer(&drawCtx.tib, ib_offset, ib_count);
    if (drawCtx.dirtyScissor) {
        bgfx::setScissor(drawCtx.scissor.x, drawCtx.scissor.y, drawCtx.scissor.w, drawCtx.scissor.h);
    }
    for (uint8_t p=0, n=drawCtx.currentPassCount; p < n; ++p) {
        bgfx::setState(drawCtx.currentPass[p].state.stateMask);
        bgfx::submit(drawCtx.currentView, drawCtx.currentPass[p].prog->prog);
    }
    drawCtx.dirtyScissor = false;
}

void endInlineBatch() {
    drawCtx.doingInlineSubmit = false;
}

void end() {

}

}
}
