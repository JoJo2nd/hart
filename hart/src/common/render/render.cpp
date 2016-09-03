/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/render/render.h"
#include "hart/base/freelist.h"

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

void begin(uint16_t view_idx, TechniqueType tech) {
    drawCtx.currentView = view_idx;
    drawCtx.activeTech = tech;
    drawCtx.currentMatSetup = nullptr;
}

void end() {

}

}
}