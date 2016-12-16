/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/render/render.h"
#include "hart/base/freelist.h"  
#include "hart/render/state.h"
#include "hart/render/shader.h"
#include "hart/render/material.h"
#include "hart/render/vertexdecl.h"
#include "hart/render/program.h"

namespace hart {
namespace render {

struct VertexDecl {
    uint16_t stride;
    bgfx::VertexDecl decl;
};

static struct {
    int windowWidth;
    int windowHeight;
} gfxInfo;
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
    bool forceFlushMat : 1;
    struct PassInfo {
        State state;
        Program prog;
    } currentPass[MaxPasses];
} drawCtx;
static bgfx::Caps gfxCaps;
static resource::Profile currentProfile = resource::Profile_Direct3D11;
static hstd::vector<uint8_t> viewIDRemap;
static hstd::vector<ViewDef> viewDefs;
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

namespace debug {
#if HART_DEBUG_INFO

enum DebugPrimType : uint8_t {
    Line, 
    Quad,
    TQuad,

    Max
};

struct DebugPrim {
    DebugPrimType type;
};

struct PrimLine : DebugPrim {
    hVec3 s; 
    hVec3 e;
    colour_t c;
};

struct PrimQuad : DebugPrim  {
    hVec3 tl; 
    hVec3 br; 
    colour_t c;
};

struct PrimTQuad : PrimQuad {
    
    Texture t;
};

struct DebugPrimDraw {
    uint32_t startVtx;
    uint32_t numVtx;
    debug::DebugPrimType t;
};

struct DebugPrims {
    VertexDecl* vtxDecl;
    VertexDecl* tvtxDecl;
    VertexBuffer vtxBuffer;
    VertexBuffer tvtxBuffer;
    hstd::unique_ptr<uint8_t> workingBuffer;
    hstd::vector<uint8_t> debugPrims;
    hstd::vector<DebugPrimDraw> drawCalls;
    uint32_t v_count;
    uint32_t tv_count;
};

static const uint32_t MaxDebugPrimsVtxs = 4096; //
static const VertexElement debugVtxDecl[] = {
    {Semantic::Position,  SemanticType::Float, 3, false},
    {Semantic::Color0,    SemanticType::Uint8, 4,  true},
};
struct DebugVtx {
    float p[3];
    uint32_t c;
};
static const uint32_t debugVtxDeclSize = sizeof(DebugVtx);
static VertexElement debugTexVtxDecl[] = {
    {Semantic::Position,  SemanticType::Float, 3, false},
    {Semantic::TexCoord0, SemanticType::Float, 2, false},
    {Semantic::Color0,    SemanticType::Uint8, 4,  true},
};
struct DebugTexVtx {
    float p[3];
    float t[2];
    uint32_t c;
};
static const uint32_t debugTexVtxDeclSize = sizeof(DebugTexVtx);

static DebugPrims currentDebugPrims;
#endif
}

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

Program createProgram(Shader* vertex, Shader* pixel) {
    Program p = bgfx::createProgram(
        vertex->getShaderProfileObject(currentProfile),
        pixel->getShaderProfileObject(currentProfile));
    return p;
}

void initialise(SDL_Window* window) {
    SDL_GetWindowSize(window, &gfxInfo.windowWidth, &gfxInfo.windowHeight);
    bgfx::sdlSetWindow(window);
    bgfx::renderFrame(); // calling this before bgfx::init prevents the render thread being created
    bgfx::init(bgfx::RendererType::Direct3D11);
    gfxCaps = *bgfx::getCaps();

#if HART_DEBUG_INFO
    debug::currentDebugPrims.vtxDecl=createVertexDecl(debug::debugVtxDecl, (uint16_t)HART_ARRAYSIZE(debug::debugVtxDecl));
    debug::currentDebugPrims.tvtxDecl=createVertexDecl(debug::debugTexVtxDecl, (uint16_t)HART_ARRAYSIZE(debug::debugTexVtxDecl));
    debug::currentDebugPrims.vtxBuffer=createVertexBuffer(nullptr, debug::MaxDebugPrimsVtxs*debug::debugVtxDeclSize, debug::currentDebugPrims.vtxDecl, Flag_VertexBuffer_Dynamic);
    debug::currentDebugPrims.tvtxBuffer=createVertexBuffer(nullptr, debug::MaxDebugPrimsVtxs*debug::debugTexVtxDeclSize, debug::currentDebugPrims.tvtxDecl, Flag_VertexBuffer_Dynamic);
    debug::currentDebugPrims.workingBuffer.reset(new uint8_t[debug::MaxDebugPrimsVtxs*debug::debugVtxDeclSize]);
#endif
}

void resetViews(ViewDef* views, size_t count) {
    viewIDRemap.clear();
    for (uint32_t i=0, n=gfxCaps.maxViews; i<n; ++i) {
        bgfx::resetView(i);
    }
    for (uint32_t i=0, n=uint32_t(count); i<n; ++i) {
        if (viewIDRemap.size() < views[i].id+1) {
            viewIDRemap.resize(views[i].id+1, ~0);
        }
        viewIDRemap[views[i].id] = i;
        uint32_t flags = 0;
        if (views[i].clearColour) flags |= BGFX_CLEAR_COLOR;
        if (views[i].clearDepth) flags |= BGFX_CLEAR_DEPTH;
        if (views[i].clearStencil) flags |= BGFX_CLEAR_STENCIL;
        bgfx::setViewClear(i, flags, views[i].colourValue, views[i].depthValue, views[i].stencilValue);
        if (!views[i].useRatio)
            bgfx::setViewRect(i, 0, 0, views[i].w, views[i].h);
        else
            bgfx::setViewRect(i, 0, 0, bgfxBBRationReamp[uint32_t(views[i].ratio)]);
    }

    viewDefs.resize(count);
    hcrt::memcpy(viewDefs.data(), views, count*sizeof(ViewDef));
}

void begin(uint16_t view_id, TechniqueType tech, hMat44 const* view, hMat44 const* proj) {
    drawCtx.currentView = viewIDRemap[view_id];
    drawCtx.activeTech = tech;
    drawCtx.currentMatSetup = nullptr;
    drawCtx.doingInlineSubmit = false;
    drawCtx.dirtyScissor = false;
    drawCtx.forceFlushMat = false;
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
    drawCtx.forceFlushMat = true;
    drawCtx.currentPassCount = (uint8_t)mat->getMaterial()->getTechnqiuePassCount(drawCtx.activeTech);
    for (uint8_t i=0, n=drawCtx.currentPassCount; i<n; ++i) {
        drawCtx.currentPass[i].state = mat->getMaterial()->getTechnqiuePassState(drawCtx.activeTech, i); 
        drawCtx.currentPass[i].prog = mat->getMaterial()->getTechnqiuePassProgram(drawCtx.activeTech, i);
    }
}

void submit(VertexBuffer in_vb) {
    if (in_vb.type == 1)
        bgfx::setVertexBuffer(in_vb.dyvb);
    else
        bgfx::setVertexBuffer(in_vb.vb);

    if (drawCtx.dirtyScissor) {
        bgfx::setScissor(drawCtx.scissor.x, drawCtx.scissor.y, drawCtx.scissor.w, drawCtx.scissor.h);
    }
    drawCtx.currentMatSetup->flushParameters(drawCtx.forceFlushMat);
    for (uint8_t p=0, n=drawCtx.currentPassCount; p < n; ++p) {
        bgfx::setState(drawCtx.currentPass[p].state.stateMask);
        bgfx::submit(drawCtx.currentView, drawCtx.currentPass[p].prog);
    }
    drawCtx.dirtyScissor = false;
    drawCtx.forceFlushMat = false;
}

void submit(IndexBuffer in_ib, VertexBuffer in_vb) {
    if (in_ib.type == 1)
        bgfx::setIndexBuffer(in_ib.dyib);
    else
        bgfx::setIndexBuffer(in_ib.ib);
    if (in_vb.type == 1)
        bgfx::setVertexBuffer(in_vb.dyvb);
    else
        bgfx::setVertexBuffer(in_vb.vb);

    if (drawCtx.dirtyScissor) {
        bgfx::setScissor(drawCtx.scissor.x, drawCtx.scissor.y, drawCtx.scissor.w, drawCtx.scissor.h);
    }
    drawCtx.currentMatSetup->flushParameters(drawCtx.forceFlushMat);
    for (uint8_t p=0, n=drawCtx.currentPassCount; p < n; ++p) {
        bgfx::setState(drawCtx.currentPass[p].state.stateMask);
        bgfx::submit(drawCtx.currentView, drawCtx.currentPass[p].prog);
    }
    drawCtx.dirtyScissor = false;
    drawCtx.forceFlushMat = false;
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
    drawCtx.currentMatSetup->flushParameters(drawCtx.forceFlushMat);
    for (uint8_t p=0, n=drawCtx.currentPassCount; p < n; ++p) {
        bgfx::setState(drawCtx.currentPass[p].state.stateMask);
        bgfx::submit(drawCtx.currentView, drawCtx.currentPass[p].prog);
    }
    drawCtx.dirtyScissor = false;
    drawCtx.forceFlushMat = false;
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
    drawCtx.currentMatSetup->flushParameters(drawCtx.forceFlushMat);
    for (uint8_t p=0, n=drawCtx.currentPassCount; p < n; ++p) {
        bgfx::setState(drawCtx.currentPass[p].state.stateMask);
        bgfx::submit(drawCtx.currentView, drawCtx.currentPass[p].prog);
    }
    drawCtx.forceFlushMat = false;
    drawCtx.dirtyScissor = false;
}

void endInlineBatch() {
    drawCtx.doingInlineSubmit = false;
}

void end() {

}

IndexBuffer createIndexBuffer(void* data, uint32_t datalen, uint32_t flags) {
    hdbassert(!(flags & ~Flag_IndexBuffer_FlagMask), "Unsupported flag requiested");
    IndexBuffer r;
    r.type = (flags & Flag_IndexBuffer_Dynamic) ? 1 : 0;
    if (r.type == 1)
        if (data == nullptr)
            r.dyib = bgfx::createDynamicIndexBuffer(datalen, BGFX_BUFFER_NONE);
        else {
            r.dyib = bgfx::createDynamicIndexBuffer(bgfx::copy(data, datalen), BGFX_BUFFER_NONE);
        }
    else
        r.ib = bgfx::createIndexBuffer(bgfx::copy(data, datalen), BGFX_BUFFER_NONE);

    return r;
}
void destroyIndexBuffer(IndexBuffer in_ib) {
    if (in_ib.type == 1) {
        bgfx::destroyDynamicIndexBuffer(in_ib.dyib);
    } else {
        bgfx::destroyIndexBuffer(in_ib.ib);
    }
}

VertexBuffer createVertexBuffer(void* data, uint32_t datalen, VertexDecl const* fmt, uint32_t flags) {
    hdbassert(!(flags & ~Flag_VertexBuffer_FlagMask), "Unsupported flag requiested");
    hdbassert(fmt, "vertex format is invalid.");
    VertexBuffer r;
    r.type = (flags & Flag_VertexBuffer_Dynamic) ? 1 : 0;
    if (r.type == 1)
        if (data == nullptr)
            r.dyvb = bgfx::createDynamicVertexBuffer(datalen, fmt->decl, BGFX_BUFFER_NONE);
        else {
            r.dyvb = bgfx::createDynamicVertexBuffer(bgfx::copy(data, datalen), fmt->decl, BGFX_BUFFER_NONE);
        }
    else
        r.vb = bgfx::createVertexBuffer(bgfx::copy(data, datalen), fmt->decl, BGFX_BUFFER_NONE);

    return r;
}

void updateVertexBuffer(VertexBuffer in_vb, void* data, uint32_t datalen) {
    bgfx::updateDynamicVertexBuffer(in_vb.dyvb, 0, bgfx::copy(data, datalen));
}

void destroyVertexBuffer(VertexBuffer in_vb) {
    if (in_vb.type == 1)
        bgfx::destroyDynamicVertexBuffer(in_vb.dyvb);
    else
        bgfx::destroyVertexBuffer(in_vb.vb);
}

void endFrame() {
    bgfx::frame();
    bgfx::setScissor(0, 0, gfxInfo.windowWidth, gfxInfo.windowHeight);
    uint32_t i = 0;
    for(auto const& view : viewDefs) {
        uint32_t flags = 0;
        if(view.clearColour) flags |= BGFX_CLEAR_COLOR;
        if(view.clearDepth) flags |= BGFX_CLEAR_DEPTH;
        if(view.clearStencil) flags |= BGFX_CLEAR_STENCIL;
        bgfx::setViewClear(i,flags,view.colourValue,view.depthValue,view.stencilValue);
        if(!view.useRatio)
            bgfx::setViewRect(i,0,0,view.w,view.h);
        else
            bgfx::setViewRect(i,0,0,bgfxBBRationReamp[uint32_t(view.ratio)]);
        // Touch the view to ensure a clear is made
        bgfx::touch(i++);
    }
}

namespace debug {
#if HART_DEBUG_INFO
void addLine(hVec3 s, hVec3 e, colour_t c) {
    uint32_t end = (uint32_t)currentDebugPrims.debugPrims.size();
    currentDebugPrims.debugPrims.resize(end + sizeof(PrimLine));
    PrimLine* p = (PrimLine*)&currentDebugPrims.debugPrims[end];
    p->type = DebugPrimType::Line;
    p->s = s;
    p->e = e;
    p->c = c;
    ++currentDebugPrims.v_count;
}

void addQuad(hVec3 tl, hVec3 br, colour_t c) {
    uint32_t end = (uint32_t)currentDebugPrims.debugPrims.size();
    currentDebugPrims.debugPrims.resize(end + sizeof(PrimQuad));
    PrimQuad* p = (PrimQuad*)&currentDebugPrims.debugPrims[end];
    p->type = DebugPrimType::Quad;
    p->tl = tl;
    p->br = br;
    p->c = c;
    ++currentDebugPrims.v_count;
}

void addTexQuad(hVec3 tl, hVec3 br, Texture t, colour_t c) {
    //TODO:
/*    
    uint32_t end = currentDebugPrims.debugPrims.size();
    currentDebugPrims.debugPrims.resize(end + sizeof(PrimTQuad));
    PrimTQuad* p = (PrimTQuad*)&currentDebugPrims.debugPrims[end];
    p->tl = tl;
    p->br = br;
    p->t = t;
    p->c = c;   
*/
}

void flushAndSumbitDebugPrims(uint16_t view, MaterialSetup* mat, hMat44 const* view_mtx, hMat44 const* proj_mtx) {
    // update dynamic vertex buffer and build draw calls
    uint8_t view_id = viewIDRemap[view];
    uint32_t byte_size = (uint32_t)currentDebugPrims.debugPrims.size();
    uint8_t* ptr = currentDebugPrims.debugPrims.data(), *end = ptr+byte_size;
    uint32_t vtx_count = 0;
    uint32_t tvtx_count = 0;
    DebugPrimDraw dc;
    hstd::vector<DebugPrimDraw> drawCalls; //TODO: make this global
    dc.t = debug::DebugPrimType::Max;
    dc.startVtx = 0;
    dc.numVtx = 0;

    DebugVtx* vb_start = (DebugVtx*)currentDebugPrims.workingBuffer.get();
    DebugVtx* vb = vb_start;
    DebugTexVtx* tvb = nullptr;
    while (ptr < end) {
        DebugPrim* dp = (DebugPrim*)ptr;
        // TODO: need to break on texture swap...
        if (dc.t != dp->type) {
            if (dc.numVtx) {
                drawCalls.push_back(dc);
            }
            dc.t = dp->type;
            dc.startVtx = dp->type == DebugPrimType::TQuad ? tvtx_count : vtx_count;
            dc.numVtx = 0;
        }
        switch (dp->type) {
        case DebugPrimType::Line: {
            PrimLine* l = (PrimLine*)ptr;
            vb->p[0] = l->s.getX();
            vb->p[1] = l->s.getY();
            vb->p[2] = l->s.getZ();
            vb->c = l->c;
            vb++;
            vb->p[0] = l->e.getX();
            vb->p[1] = l->e.getY();
            vb->p[2] = l->e.getZ();
            vb->c = l->c;
            vb++;
            dc.numVtx += 2;
            vtx_count += 2;
            ptr = (uint8_t*)(l+1);
        } break;
        case DebugPrimType::Quad: {
            PrimQuad* q = (PrimQuad*)ptr;
            // Tri 1
            vb->p[0] = q->tl.getX();
            vb->p[1] = q->tl.getY();
            vb->p[2] = q->tl.getZ();
            vb->c = q->c;
            vb++;
            vb->p[0] = q->br.getX();
            vb->p[1] = q->tl.getY();
            vb->p[2] = q->tl.getZ();
            vb->c = q->c;
            vb++;
            vb->p[0] = q->br.getX();
            vb->p[1] = q->br.getY();
            vb->p[2] = q->tl.getZ();
            vb->c = q->c;
            vb++;
            //Tri 2
            vb->p[0] = q->tl.getX();
            vb->p[1] = q->tl.getY();
            vb->p[2] = q->tl.getZ();
            vb->c = q->c;
            vb++;
            vb->p[0] = q->br.getX();
            vb->p[1] = q->br.getY();
            vb->p[2] = q->tl.getZ();
            vb->c = q->c;
            vb++;
            vb->p[0] = q->tl.getX();
            vb->p[1] = q->br.getY();
            vb->p[2] = q->tl.getZ();
            vb->c = q->c;
            vb++;
            dc.numVtx += 6;
            vtx_count += 6;
            ptr = (uint8_t*)(q+1);
        } break;
//         case DebugPrimType::TQuad: {
//             PrimTQuad* q = (PrimTQuad*)ptr;
//             // TODO:
//             ptr = (uint8_t*)(q+1);
//         } break;
        }
    }
    if (dc.numVtx) {
        drawCalls.push_back(dc);
    }
    if (!((uintptr_t)vb-(uintptr_t)vb_start)) 
        return;

    bgfx::updateDynamicVertexBuffer(currentDebugPrims.vtxBuffer.dyvb, 0, bgfx::copy(vb_start, (uint32_t)((uintptr_t)vb-(uintptr_t)vb_start)));

    // submit
    bgfx::setViewTransform(view_id, view_mtx, proj_mtx);
    bgfx::setScissor(0, 0, gfxInfo.windowWidth, gfxInfo.windowHeight);
    State state = mat->getMaterial()->getTechnqiuePassState(TechniqueType_Main, 0); 
    Program prog = mat->getMaterial()->getTechnqiuePassProgram(TechniqueType_Main, 0);
    mat->flushParameters(drawCtx.forceFlushMat);
    for (auto const& i : drawCalls) {
        if (i.t == DebugPrimType::Line)
            bgfx::setState(state.stateMask | BGFX_STATE_PT_LINES);
        else if (i.t == DebugPrimType::Quad)
            bgfx::setState(state.stateMask);
        bgfx::setVertexBuffer(currentDebugPrims.vtxBuffer.dyvb, i.startVtx, i.numVtx);
        bgfx::submit(view_id, prog);
    }

    // clear
    currentDebugPrims.debugPrims.clear();
    currentDebugPrims.v_count = 0;
}
#endif
}
}
}
