/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#pragma once

#include "hart/config.h"
#include "hart/render/state.h"
#include "hart/render/shader.h"
#include "hart/render/material.h"

namespace hart {
namespace render {

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

};

struct VertexDecl;

void initialiseViews(ViewDef* views, size_t count);

VertexDecl* createVertexDecl(VertexElement const* elements, uint16_t element_count);
void destroyVertexDecl();

void begin(uint16_t view_idx, TechniqueType tech);
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
