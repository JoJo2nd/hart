/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#pragma once

#include "hart/config.h"
#include "hart/base/matrix.h"
#include "hart/render/technique.h"
#include "bgfx/bgfx.h"

struct SDL_Window;

namespace hart {
namespace render {

class Material;
class MaterialSetup;
class Shader;

enum View {
    View_Main = 1,
    View_Debug = 32,
};

enum class Ratio {
    Same,      // 
    Half,      // 
    Quarter,   // 
    Eighth,    // 
    Sixteenth, // 
    Double,    // 
};

enum FlagsIndexBuffer {
    Flag_IndexBuffer_Dynamic    = 0x80000000,
    Flag_IndexBuffer_32bit      = 0x40000000,
};

static const uint32_t Flag_IndexBuffer_FlagMask = 
    Flag_IndexBuffer_Dynamic | 
    Flag_IndexBuffer_32bit;

enum FlagsVertexBuffer {
    Flag_VertexBuffer_Dynamic       = 0x80000000,
};
static const uint32_t Flag_VertexBuffer_FlagMask = 
    Flag_VertexBuffer_Dynamic;

struct ViewDef {
    uint32_t id = ~0ul;
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t w = ~0ul;
    uint32_t h = ~0ul;
    Ratio    ratio = Ratio::Same;
    float    depthValue = 1.f;
    uint32_t colourValue = 0x000000ff;
    uint32_t stencilValue = 0;
    // TODO: frame buffers
    bool     useRatio = true;
    bool     clearColour = false;
    bool     clearDepth = false;
    bool     clearStencil = false;
};

struct IndexBuffer {
    union {
        bgfx::IndexBufferHandle ib;
        bgfx::DynamicIndexBufferHandle dyib;
    };
    uint16_t type; // 0 for ib , 1 for dynamic
};
struct VertexBuffer {
    union {
        bgfx::VertexBufferHandle vb;
        bgfx::DynamicVertexBufferHandle dyvb;
    };
    uint16_t type;
};
struct VertexDecl;

void initialise(SDL_Window* window);

void resetViews(ViewDef* views, size_t count);

void begin(uint16_t view_id, TechniqueType tech, hMat44 const* view, hMat44 const* proj);
void setMaterialSetup(MaterialSetup* mat);
void setScissor(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void submit(VertexBuffer in_vb);
void submit(IndexBuffer in_ib, VertexBuffer in_vb);
void submitInline(VertexDecl* fmt, void* idx, void* vtx, uint16_t prims);
void beginInlineBatch(VertexDecl* fmt, void* idx, uint32_t numIndices, void* vtx, uint16_t numVertices);
void inlineBatchSubmit(uint16_t ib_offset, uint32_t ib_count, uint16_t vb_offset, uint16_t vb_count);
void endInlineBatch();
void end();

void endFrame();

IndexBuffer createIndexBuffer(void* data, uint32_t datalen, uint32_t flags);
void destroyIndexBuffer(IndexBuffer in_ib);

VertexBuffer createVertexBuffer(void* data, uint32_t datalen, VertexDecl const* fmt, uint32_t flags);
void updateVertexBuffer(VertexBuffer in_vb, void* data, uint32_t datalen);
void destroyVertexBuffer(VertexBuffer in_vb);

}
}

namespace hrnd = hart::render;
