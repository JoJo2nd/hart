/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#pragma once

#include "hart/config.h"
#include "hart/render/state.h"
#include "hart/render/shader.h"
#include "hart/render/material.h"

struct SDL_Window;

namespace hart {
namespace render {

enum class View {
    Debug = 32,
};

enum class Ratio {
    Same,      // 
    Half,      // 
    Quarter,   // 
    Eighth,    // 
    Sixteenth, // 
    Double,    // 
};

enum class Semantic {
    Position,  //
    Normal,    //
    Tangent,   //
    Bitangent, //
    Color0,    //
    Color1,    //
    TexCoord0, //
    TexCoord1, //
    TexCoord2, //
    TexCoord3, //
    TexCoord4, //
    TexCoord5, //
    TexCoord6, //
    TexCoord7, //
};

enum class SemanticType {
    Uint8,  //
    Int16,  //
    Half,   // May not be supported everywhere
    Float,  //
};

struct VertexElement {
    Semantic sem;
    SemanticType semType;
    uint8_t elementCount;
    bool normalized;
};

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

struct VertexDecl;

void initialise(SDL_Window* window);

void resetViews(ViewDef* views, size_t count);

VertexDecl* createVertexDecl(VertexElement const* elements, uint16_t element_count);
void destroyVertexDecl();

void begin(uint16_t view_id, TechniqueType tech);
void setMaterialSetup(MaterialSetup* mat);
void setScissor();
void setIndexBuffer();
void setVertexBuffer();
void submit();
void submitInline(VertexDecl* fmt, void* idx, void* vtx, uint16_t prims);
void end();

void endFrame();

}
}

namespace hrnd = hart::render;
