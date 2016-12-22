/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

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

struct VertexDecl;

VertexDecl* createVertexDecl(VertexElement const* elements, uint16_t element_count);
void destroyVertexDecl();

}
}

namespace hrnd = hart::render;
