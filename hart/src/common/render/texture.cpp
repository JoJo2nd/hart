/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/render/texture.h"
#include "hart/base/crt.h"
#include "hart/base/debug.h"

HART_OBJECT_TYPE_DECL(hart::render::TextureRes);

namespace hart {
namespace render {

static bgfx::TextureFormat::Enum TextureFormatTobgfxTextureFormat[TextureFormat_MAX+1] = {
    bgfx::TextureFormat::BC1, // from -> TextureFormat_BC1
    bgfx::TextureFormat::BC2, // from -> TextureFormat_BC2
    bgfx::TextureFormat::BC3, // from -> TextureFormat_BC3
    bgfx::TextureFormat::BC4, // from -> TextureFormat_BC4
    bgfx::TextureFormat::BC5, // from -> TextureFormat_BC5
    bgfx::TextureFormat::BC6H, // from -> TextureFormat_BC6H
    bgfx::TextureFormat::BC7, // from -> TextureFormat_BC7
    bgfx::TextureFormat::ETC1, // from -> TextureFormat_ETC1
    bgfx::TextureFormat::ETC2, // from -> TextureFormat_ETC2
    bgfx::TextureFormat::ETC2A, // from -> TextureFormat_ETC2A
    bgfx::TextureFormat::ETC2A1, // from -> TextureFormat_ETC2A1
    bgfx::TextureFormat::PTC12, // from -> TextureFormat_PTC12
    bgfx::TextureFormat::PTC14, // from -> TextureFormat_PTC14
    bgfx::TextureFormat::PTC12A, // from -> TextureFormat_PTC12A
    bgfx::TextureFormat::PTC14A, // from -> TextureFormat_PTC14A
    bgfx::TextureFormat::PTC22, // from -> TextureFormat_PTC22
    bgfx::TextureFormat::PTC24, // from -> TextureFormat_PTC24
    bgfx::TextureFormat::R1, // from -> TextureFormat_R1
    bgfx::TextureFormat::A8, // from -> TextureFormat_A8
    bgfx::TextureFormat::R8, // from -> TextureFormat_R8
    bgfx::TextureFormat::R8I, // from -> TextureFormat_R8I
    bgfx::TextureFormat::R8U, // from -> TextureFormat_R8U
    bgfx::TextureFormat::R8S, // from -> TextureFormat_R8S
    bgfx::TextureFormat::R16, // from -> TextureFormat_R16
    bgfx::TextureFormat::R16I, // from -> TextureFormat_R16I
    bgfx::TextureFormat::R16U, // from -> TextureFormat_R16U
    bgfx::TextureFormat::R16F, // from -> TextureFormat_R16F
    bgfx::TextureFormat::R16S, // from -> TextureFormat_R16S
    bgfx::TextureFormat::R32I, // from -> TextureFormat_R32I
    bgfx::TextureFormat::R32U, // from -> TextureFormat_R32U
    bgfx::TextureFormat::R32F, // from -> TextureFormat_R32F
    bgfx::TextureFormat::RG8, // from -> TextureFormat_RG8
    bgfx::TextureFormat::RG8I, // from -> TextureFormat_RG8I
    bgfx::TextureFormat::RG8U, // from -> TextureFormat_RG8U
    bgfx::TextureFormat::RG8S, // from -> TextureFormat_RG8S
    bgfx::TextureFormat::RG16, // from -> TextureFormat_RG16
    bgfx::TextureFormat::RG16I, // from -> TextureFormat_RG16I
    bgfx::TextureFormat::RG16U, // from -> TextureFormat_RG16U
    bgfx::TextureFormat::RG16F, // from -> TextureFormat_RG16F
    bgfx::TextureFormat::RG16S, // from -> TextureFormat_RG16S
    bgfx::TextureFormat::RG32I, // from -> TextureFormat_RG32I
    bgfx::TextureFormat::RG32U, // from -> TextureFormat_RG32U
    bgfx::TextureFormat::RG32F, // from -> TextureFormat_RG32F
    bgfx::TextureFormat::RGB8, // from -> TextureFormat_RGB8
    bgfx::TextureFormat::RGB8I, // from -> TextureFormat_RGB8I
    bgfx::TextureFormat::RGB8U, // from -> TextureFormat_RGB8U
    bgfx::TextureFormat::RGB8S, // from -> TextureFormat_RGB8S
    bgfx::TextureFormat::RGB9E5F, // from -> TextureFormat_RGB9E5F
    bgfx::TextureFormat::BGRA8, // from -> TextureFormat_BGRA8
    bgfx::TextureFormat::RGBA8, // from -> TextureFormat_RGBA8
    bgfx::TextureFormat::RGBA8I, // from -> TextureFormat_RGBA8I
    bgfx::TextureFormat::RGBA8U, // from -> TextureFormat_RGBA8U
    bgfx::TextureFormat::RGBA8S, // from -> TextureFormat_RGBA8S
    bgfx::TextureFormat::RGBA16, // from -> TextureFormat_RGBA16
    bgfx::TextureFormat::RGBA16I, // from -> TextureFormat_RGBA16I
    bgfx::TextureFormat::RGBA16U, // from -> TextureFormat_RGBA16U
    bgfx::TextureFormat::RGBA16F, // from -> TextureFormat_RGBA16F
    bgfx::TextureFormat::RGBA16S, // from -> TextureFormat_RGBA16S
    bgfx::TextureFormat::RGBA32I, // from -> TextureFormat_RGBA32I
    bgfx::TextureFormat::RGBA32U, // from -> TextureFormat_RGBA32U
    bgfx::TextureFormat::RGBA32F, // from -> TextureFormat_RGBA32F
    bgfx::TextureFormat::R5G6B5, // from -> TextureFormat_R5G6B5
    bgfx::TextureFormat::RGBA4, // from -> TextureFormat_RGBA4
    bgfx::TextureFormat::RGB5A1, // from -> TextureFormat_RGB5A1
    bgfx::TextureFormat::RGB10A2, // from -> TextureFormat_RGB10A2
    bgfx::TextureFormat::R11G11B10F, // from -> TextureFormat_R11G11B10F
    bgfx::TextureFormat::D16, // from -> TextureFormat_D16
    bgfx::TextureFormat::D24, // from -> TextureFormat_D24
    bgfx::TextureFormat::D24S8, // from -> TextureFormat_D24S8
    bgfx::TextureFormat::D32, // from -> TextureFormat_D32
    bgfx::TextureFormat::D16F, // from -> TextureFormat_D16F
    bgfx::TextureFormat::D24F, // from -> TextureFormat_D24F
    bgfx::TextureFormat::D32F, // from -> TextureFormat_D32F
    bgfx::TextureFormat::D0S8, // from -> TextureFormat_D0S8
};

bool TextureRes::deserialiseObject(MarshallType const* in_data, hobjfact::SerialiseParams const&) {
    type = in_data->type();
    format = in_data->format();
    width = in_data->width();
    height = in_data->height();
    depth = in_data->depth();
    mips = in_data->mips();

    switch (type) {
    case TextureType_Tex2D: {
        texture = createTexture2D(width, height, mips, format, 0, in_data->data()->data(), in_data->data()->size());
    } break;
    default: hdbfatal("Invalid texture type");
    } 

    return true;
}

TextureRes::~TextureRes() {
    render::destroyTexture(texture);
}

Texture createTexture2D(uint16_t width, uint16_t height, uint8_t numMips, TextureFormat format, uint32_t flags, void const* mem, uint32_t memlen) {
    bgfx::Memory const* m = bgfx::copy(mem, memlen);
    return bgfx::createTexture2D(width, height, numMips, TextureFormatTobgfxTextureFormat[format], flags, m);
}

}
}