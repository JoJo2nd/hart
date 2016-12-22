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
namespace ktx {
/*
    The normal bgfx texture tools output data in KTX format.
    This isn't an awful format so we parse it directly for info then pass it on
   to bgfx.
    See https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/
*/
#ifndef KTX_DEFINES
#define KTX_DEFINES

#define KTX_ETC1_RGB8_OES 0x8D64
#define KTX_COMPRESSED_R11_EAC 0x9270
#define KTX_COMPRESSED_SIGNED_R11_EAC 0x9271
#define KTX_COMPRESSED_RG11_EAC 0x9272
#define KTX_COMPRESSED_SIGNED_RG11_EAC 0x9273
#define KTX_COMPRESSED_RGB8_ETC2 0x9274
#define KTX_COMPRESSED_SRGB8_ETC2 0x9275
#define KTX_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9276
#define KTX_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9277
#define KTX_COMPRESSED_RGBA8_ETC2_EAC 0x9278
#define KTX_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC 0x9279
#define KTX_COMPRESSED_RGB_PVRTC_4BPPV1_IMG 0x8C00
#define KTX_COMPRESSED_RGB_PVRTC_2BPPV1_IMG 0x8C01
#define KTX_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#define KTX_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG 0x8C03
#define KTX_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG 0x9137
#define KTX_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG 0x9138
#define KTX_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define KTX_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define KTX_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#define KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
#define KTX_COMPRESSED_LUMINANCE_LATC1_EXT 0x8C70
#define KTX_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT 0x8C72
#define KTX_COMPRESSED_RGBA_BPTC_UNORM_ARB 0x8E8C
#define KTX_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB 0x8E8D
#define KTX_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB 0x8E8E
#define KTX_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB 0x8E8F
#define KTX_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT 0x8A54
#define KTX_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT 0x8A55
#define KTX_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT 0x8A56
#define KTX_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT 0x8A57

#define KTX_R8 0x8229
#define KTX_R16 0x822A
#define KTX_RG8 0x822B
#define KTX_RG16 0x822C
#define KTX_R16F 0x822D
#define KTX_R32F 0x822E
#define KTX_RG16F 0x822F
#define KTX_RG32F 0x8230
#define KTX_RGBA8 0x8058
#define KTX_RGBA16 0x805B
#define KTX_RGBA16F 0x881A
#define KTX_R32UI 0x8236
#define KTX_RG32UI 0x823C
#define KTX_RGBA32UI 0x8D70
#define KTX_RGBA32F 0x8814
#define KTX_RGB565 0x8D62
#define KTX_RGBA4 0x8056
#define KTX_RGB5_A1 0x8057
#define KTX_RGB10_A2 0x8059
#define KTX_R8I 0x8231
#define KTX_R8UI 0x8232
#define KTX_R16I 0x8233
#define KTX_R16UI 0x8234
#define KTX_R32I 0x8235
#define KTX_R32UI 0x8236
#define KTX_RG8I 0x8237
#define KTX_RG8UI 0x8238
#define KTX_RG16I 0x8239
#define KTX_RG16UI 0x823A
#define KTX_RG32I 0x823B
#define KTX_RG32UI 0x823C
#define KTX_R8_SNORM 0x8F94
#define KTX_RG8_SNORM 0x8F95
#define KTX_RGB8_SNORM 0x8F96
#define KTX_RGBA8_SNORM 0x8F97
#define KTX_R16_SNORM 0x8F98
#define KTX_RG16_SNORM 0x8F99
#define KTX_RGB16_SNORM 0x8F9A
#define KTX_RGBA16_SNORM 0x8F9B
#define KTX_SRGB8 0x8C41
#define KTX_SRGB8_ALPHA8 0x8C43
#define KTX_RGBA32UI 0x8D70
#define KTX_RGB32UI 0x8D71
#define KTX_RGBA16UI 0x8D76
#define KTX_RGB16UI 0x8D77
#define KTX_RGBA8UI 0x8D7C
#define KTX_RGB8UI 0x8D7D
#define KTX_RGBA32I 0x8D82
#define KTX_RGB32I 0x8D83
#define KTX_RGBA16I 0x8D88
#define KTX_RGB16I 0x8D89
#define KTX_RGBA8I 0x8D8E
#define KTX_RGB8 0x8051
#define KTX_RGB8I 0x8D8F
#define KTX_RGB9_E5 0x8C3D
#define KTX_R11F_G11F_B10F 0x8C3A

#define KTX_ZERO 0
#define KTX_RED 0x1903
#define KTX_ALPHA 0x1906
#define KTX_RGB 0x1907
#define KTX_RGBA 0x1908
#define KTX_BGRA 0x80E1
#define KTX_RG 0x8227

#define KTX_BYTE 0x1400
#define KTX_UNSIGNED_BYTE 0x1401
#define KTX_SHORT 0x1402
#define KTX_UNSIGNED_SHORT 0x1403
#define KTX_INT 0x1404
#define KTX_UNSIGNED_INT 0x1405
#define KTX_FLOAT 0x1406
#define KTX_HALF_FLOAT 0x140B
#define KTX_UNSIGNED_INT_5_9_9_9_REV 0x8C3E
#define KTX_UNSIGNED_SHORT_5_6_5 0x8363
#define KTX_UNSIGNED_SHORT_4_4_4_4 0x8033
#define KTX_UNSIGNED_SHORT_5_5_5_1 0x8034
#define KTX_UNSIGNED_INT_2_10_10_10_REV 0x8368
#define KTX_UNSIGNED_INT_10F_11F_11F_REV 0x8C3B

#endif // KTX_DEFINES

static const uint32_t KtxFormatRemap[] = {
  KTX_COMPRESSED_RGBA_S3TC_DXT1_EXT,            // BC1
  KTX_COMPRESSED_RGBA_S3TC_DXT3_EXT,            // BC2
  KTX_COMPRESSED_RGBA_S3TC_DXT5_EXT,            // BC3
  KTX_COMPRESSED_LUMINANCE_LATC1_EXT,           // BC4
  KTX_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT,     // BC5
  KTX_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB,     // BC6H
  KTX_COMPRESSED_RGBA_BPTC_UNORM_ARB,           // BC7
  KTX_ETC1_RGB8_OES,                            // ETC1
  KTX_COMPRESSED_RGB8_ETC2,                     // ETC2
  KTX_COMPRESSED_RGBA8_ETC2_EAC,                // ETC2A
  KTX_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, // ETC2A1
  KTX_COMPRESSED_RGB_PVRTC_2BPPV1_IMG,          // PTC12
  KTX_COMPRESSED_RGB_PVRTC_4BPPV1_IMG,          // PTC14
  KTX_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG,         // PTC12A
  KTX_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG,         // PTC14A
  KTX_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG,         // PTC22
  KTX_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG,         // PTC24
  KTX_ZERO,                                     // R1
  KTX_ALPHA,                                    // A8
  KTX_R8,                                       // R8
  KTX_R8I,                                      // R8S
  KTX_R8UI,                                     // R8S
  KTX_R8_SNORM,                                 // R8S
  KTX_R16,                                      // R16
  KTX_R16I,                                     // R16I
  KTX_R16UI,                                    // R16U
  KTX_R16F,                                     // R16F
  KTX_R16_SNORM,                                // R16S
  KTX_R32I,                                     // R32I
  KTX_R32UI,                                    // R32U
  KTX_R32F,                                     // R32F
  KTX_RG8,                                      // RG8
  KTX_RG8I,                                     // RG8I
  KTX_RG8UI,                                    // RG8U
  KTX_RG8_SNORM,                                // RG8S
  KTX_RG16,                                     // RG16
  KTX_RG16I,                                    // RG16
  KTX_RG16UI,                                   // RG16
  KTX_RG16F,                                    // RG16F
  KTX_RG16_SNORM,                               // RG16S
  KTX_RG32I,                                    // RG32I
  KTX_RG32UI,                                   // RG32U
  KTX_RG32F,                                    // RG32F
  KTX_RGB8,                                     // RGB8
  KTX_RGB8I,                                    // RGB8I
  KTX_RGB8UI,                                   // RGB8U
  KTX_RGB8_SNORM,                               // RGB8S
  KTX_RGB9_E5,                                  // RGB9E5F
  KTX_BGRA,                                     // BGRA8
  KTX_RGBA8,                                    // RGBA8
  KTX_RGBA8I,                                   // RGBA8I
  KTX_RGBA8UI,                                  // RGBA8U
  KTX_RGBA8_SNORM,                              // RGBA8S
  KTX_RGBA16,                                   // RGBA16
  KTX_RGBA16I,                                  // RGBA16I
  KTX_RGBA16UI,                                 // RGBA16U
  KTX_RGBA16F,                                  // RGBA16F
  KTX_RGBA16_SNORM,                             // RGBA16S
  KTX_RGBA32I,                                  // RGBA32I
  KTX_RGBA32UI,                                 // RGBA32U
  KTX_RGBA32F,                                  // RGBA32F
  KTX_RGB565,                                   // R5G6B5
  KTX_RGBA4,                                    // RGBA4
  KTX_RGB5_A1,                                  // RGB5A1
  KTX_RGB10_A2,                                 // RGB10A2
  KTX_R11F_G11F_B10F,                           // R11G11B10F
};

struct Header {
  char     identifier[12];
  uint32_t endianness;
  uint32_t glType;
  uint32_t glTypeSize;
  uint32_t glFormat;
  uint32_t glInternalFormat;
  uint32_t glBaseInternalFormat;
  uint32_t pixelWidth;
  uint32_t pixelHeight;
  uint32_t pixelDepth;
  uint32_t numberOfArrayElements;
  uint32_t numberOfFaces;
  uint32_t numberOfMipmapLevels;
  uint32_t bytesOfKeyValueData;
  // array of char[bytesOfKeyValueData] follows.
};

struct KTXInfo {
  Header*       hdr;
  TextureFormat convFormat;
  void*         textureBaseMem;
};

static char FileIdentifier[16] = {(char)'«', (char)'K', (char)'T',  (char)'X',  (char)' ',    (char)'1',
                                  (char)'1', (char)'»', (char)'\r', (char)'\n', (char)'\x1A', (char)'\n'};
static uint32_t EndiannessCheck = 0x04030201;

bool read(void const* mem, size_t len, KTXInfo* o_info) {
  uint8_t* ptr = (uint8_t*)mem;
  if (len < sizeof(Header)) return false;
  Header* hdr = (Header*)ptr;
  ptr += sizeof(Header);
  ptr += hdr->bytesOfKeyValueData;
  if (hcrt::memcmp(FileIdentifier, hdr->identifier, HART_ARRAYSIZE(hdr->identifier)) != 0) return false;
  if (hdr->endianness != EndiannessCheck) return false;

  if (hdr->glInternalFormat != KTX_ZERO) {
    for (uint32_t i = 0, n = (uint32_t)HART_ARRAYSIZE(KtxFormatRemap); i < n; ++i) {
      if (hdr->glInternalFormat == KtxFormatRemap[i]) {
        o_info->convFormat = TextureFormat(i);
      }
    }
  } else if (hdr->glBaseInternalFormat == KTX_RED) {
    o_info->convFormat = TextureFormat_R8;
  } else if (hdr->glBaseInternalFormat == KTX_RGB) {
    o_info->convFormat = TextureFormat_RGB8;
  }

  o_info->hdr = hdr;
  o_info->textureBaseMem = ptr;
  return true;
}
}

static bgfx::TextureFormat::Enum TextureFormatTobgfxTextureFormat[TextureFormat_MAX + 1] = {
  bgfx::TextureFormat::BC1,        // from -> TextureFormat_BC1
  bgfx::TextureFormat::BC2,        // from -> TextureFormat_BC2
  bgfx::TextureFormat::BC3,        // from -> TextureFormat_BC3
  bgfx::TextureFormat::BC4,        // from -> TextureFormat_BC4
  bgfx::TextureFormat::BC5,        // from -> TextureFormat_BC5
  bgfx::TextureFormat::BC6H,       // from -> TextureFormat_BC6H
  bgfx::TextureFormat::BC7,        // from -> TextureFormat_BC7
  bgfx::TextureFormat::ETC1,       // from -> TextureFormat_ETC1
  bgfx::TextureFormat::ETC2,       // from -> TextureFormat_ETC2
  bgfx::TextureFormat::ETC2A,      // from -> TextureFormat_ETC2A
  bgfx::TextureFormat::ETC2A1,     // from -> TextureFormat_ETC2A1
  bgfx::TextureFormat::PTC12,      // from -> TextureFormat_PTC12
  bgfx::TextureFormat::PTC14,      // from -> TextureFormat_PTC14
  bgfx::TextureFormat::PTC12A,     // from -> TextureFormat_PTC12A
  bgfx::TextureFormat::PTC14A,     // from -> TextureFormat_PTC14A
  bgfx::TextureFormat::PTC22,      // from -> TextureFormat_PTC22
  bgfx::TextureFormat::PTC24,      // from -> TextureFormat_PTC24
  bgfx::TextureFormat::R1,         // from -> TextureFormat_R1
  bgfx::TextureFormat::A8,         // from -> TextureFormat_A8
  bgfx::TextureFormat::R8,         // from -> TextureFormat_R8
  bgfx::TextureFormat::R8I,        // from -> TextureFormat_R8I
  bgfx::TextureFormat::R8U,        // from -> TextureFormat_R8U
  bgfx::TextureFormat::R8S,        // from -> TextureFormat_R8S
  bgfx::TextureFormat::R16,        // from -> TextureFormat_R16
  bgfx::TextureFormat::R16I,       // from -> TextureFormat_R16I
  bgfx::TextureFormat::R16U,       // from -> TextureFormat_R16U
  bgfx::TextureFormat::R16F,       // from -> TextureFormat_R16F
  bgfx::TextureFormat::R16S,       // from -> TextureFormat_R16S
  bgfx::TextureFormat::R32I,       // from -> TextureFormat_R32I
  bgfx::TextureFormat::R32U,       // from -> TextureFormat_R32U
  bgfx::TextureFormat::R32F,       // from -> TextureFormat_R32F
  bgfx::TextureFormat::RG8,        // from -> TextureFormat_RG8
  bgfx::TextureFormat::RG8I,       // from -> TextureFormat_RG8I
  bgfx::TextureFormat::RG8U,       // from -> TextureFormat_RG8U
  bgfx::TextureFormat::RG8S,       // from -> TextureFormat_RG8S
  bgfx::TextureFormat::RG16,       // from -> TextureFormat_RG16
  bgfx::TextureFormat::RG16I,      // from -> TextureFormat_RG16I
  bgfx::TextureFormat::RG16U,      // from -> TextureFormat_RG16U
  bgfx::TextureFormat::RG16F,      // from -> TextureFormat_RG16F
  bgfx::TextureFormat::RG16S,      // from -> TextureFormat_RG16S
  bgfx::TextureFormat::RG32I,      // from -> TextureFormat_RG32I
  bgfx::TextureFormat::RG32U,      // from -> TextureFormat_RG32U
  bgfx::TextureFormat::RG32F,      // from -> TextureFormat_RG32F
  bgfx::TextureFormat::RGB8,       // from -> TextureFormat_RGB8
  bgfx::TextureFormat::RGB8I,      // from -> TextureFormat_RGB8I
  bgfx::TextureFormat::RGB8U,      // from -> TextureFormat_RGB8U
  bgfx::TextureFormat::RGB8S,      // from -> TextureFormat_RGB8S
  bgfx::TextureFormat::RGB9E5F,    // from -> TextureFormat_RGB9E5F
  bgfx::TextureFormat::BGRA8,      // from -> TextureFormat_BGRA8
  bgfx::TextureFormat::RGBA8,      // from -> TextureFormat_RGBA8
  bgfx::TextureFormat::RGBA8I,     // from -> TextureFormat_RGBA8I
  bgfx::TextureFormat::RGBA8U,     // from -> TextureFormat_RGBA8U
  bgfx::TextureFormat::RGBA8S,     // from -> TextureFormat_RGBA8S
  bgfx::TextureFormat::RGBA16,     // from -> TextureFormat_RGBA16
  bgfx::TextureFormat::RGBA16I,    // from -> TextureFormat_RGBA16I
  bgfx::TextureFormat::RGBA16U,    // from -> TextureFormat_RGBA16U
  bgfx::TextureFormat::RGBA16F,    // from -> TextureFormat_RGBA16F
  bgfx::TextureFormat::RGBA16S,    // from -> TextureFormat_RGBA16S
  bgfx::TextureFormat::RGBA32I,    // from -> TextureFormat_RGBA32I
  bgfx::TextureFormat::RGBA32U,    // from -> TextureFormat_RGBA32U
  bgfx::TextureFormat::RGBA32F,    // from -> TextureFormat_RGBA32F
  bgfx::TextureFormat::R5G6B5,     // from -> TextureFormat_R5G6B5
  bgfx::TextureFormat::RGBA4,      // from -> TextureFormat_RGBA4
  bgfx::TextureFormat::RGB5A1,     // from -> TextureFormat_RGB5A1
  bgfx::TextureFormat::RGB10A2,    // from -> TextureFormat_RGB10A2
  bgfx::TextureFormat::R11G11B10F, // from -> TextureFormat_R11G11B10F
  bgfx::TextureFormat::D16,        // from -> TextureFormat_D16
  bgfx::TextureFormat::D24,        // from -> TextureFormat_D24
  bgfx::TextureFormat::D24S8,      // from -> TextureFormat_D24S8
  bgfx::TextureFormat::D32,        // from -> TextureFormat_D32
  bgfx::TextureFormat::D16F,       // from -> TextureFormat_D16F
  bgfx::TextureFormat::D24F,       // from -> TextureFormat_D24F
  bgfx::TextureFormat::D32F,       // from -> TextureFormat_D32F
  bgfx::TextureFormat::D0S8,       // from -> TextureFormat_D0S8
};

bool TextureRes::deserialiseObject(MarshallType const* in_data, hobjfact::SerialiseParams const&) {
  ktx::KTXInfo t_info;
  if (!ktx::read(in_data->data()->data(), in_data->data()->size(), &t_info)) {
    hdbfatal("Invalid texture header");
    return false;
  }
  if (t_info.hdr->numberOfFaces == 6)
    type = TextureType_TexCube;
  else if (t_info.hdr->pixelDepth > 1)
    type = TextureType_Tex3D;
  else if (t_info.hdr->pixelHeight > 1)
    type = TextureType_Tex2D;
  else
    type = TextureType_Tex1D;
  format = t_info.convFormat;
  width = t_info.hdr->pixelWidth;
  height = t_info.hdr->pixelHeight;
  depth = t_info.hdr->pixelDepth;
  mips = t_info.hdr->numberOfMipmapLevels;

  bgfx::Memory const* m = bgfx::copy(in_data->data()->data(), in_data->data()->size());
  texture = createTexture(m);
  return true;
}

TextureRes::~TextureRes() {
  render::destroyTexture(texture);
}

Texture createTexture(void const* raw_data, uint32_t data_len) {
  bgfx::Memory const* m = bgfx::copy(raw_data, data_len);
  return createTexture(m);
}

Texture createTexture2D(uint16_t width, uint16_t height, uint8_t numMips, TextureFormat format, uint32_t flags,
                        void const* mem, uint32_t memlen) {
  bgfx::Memory const* m = bgfx::copy(mem, memlen);
  return bgfx::createTexture2D(width, height, numMips, TextureFormatTobgfxTextureFormat[format], flags, m);
}
}
}