/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/render/state.h"
#include "bgfx/bgfx.h"

HART_OBJECT_TYPE_DECL(hart::render::State);

namespace hart {
namespace render {

bool State::deserialiseObject(MarshallType const* in_data, hobjfact::SerialiseParams const&) {
  stateMask = 0;
  if (in_data->rgbWrite()) stateMask |= BGFX_STATE_RGB_WRITE;
  if (in_data->alphaWrite()) stateMask |= BGFX_STATE_ALPHA_WRITE;
  if (in_data->depthWrite()) stateMask |= BGFX_STATE_DEPTH_WRITE;
  switch (in_data->depthTest()) {
  case resource::DepthTest_Less: stateMask |= BGFX_STATE_DEPTH_TEST_LESS;
  case resource::DepthTest_LEqual: stateMask |= BGFX_STATE_DEPTH_TEST_LEQUAL;
  case resource::DepthTest_Equal: stateMask |= BGFX_STATE_DEPTH_TEST_EQUAL;
  case resource::DepthTest_GEqual: stateMask |= BGFX_STATE_DEPTH_TEST_GEQUAL;
  case resource::DepthTest_Greater: stateMask |= BGFX_STATE_DEPTH_TEST_GREATER;
  case resource::DepthTest_NEqual: stateMask |= BGFX_STATE_DEPTH_TEST_NOTEQUAL;
  case resource::DepthTest_Never: stateMask |= BGFX_STATE_DEPTH_TEST_NEVER;
  case resource::DepthTest_Always: stateMask |= BGFX_STATE_DEPTH_TEST_ALWAYS;
  }
  if (in_data->independentAlpha()) stateMask |= BGFX_STATE_BLEND_INDEPENDENT;
  uint64_t blend_src = 0;
  uint64_t blend_dst = 0;
  uint64_t blend_src_alpha = 0;
  uint64_t blend_dst_alpha = 0;
  uint64_t blend_fn = 0;
  uint64_t blend_fn_alpha = 0;
  switch (in_data->blendSrc()) {
  case resource::BlendOp_Zero: blend_src = BGFX_STATE_BLEND_ZERO; break;
  case resource::BlendOp_One: blend_src = BGFX_STATE_BLEND_ONE; break;
  case resource::BlendOp_SrcColor: blend_src = BGFX_STATE_BLEND_SRC_COLOR; break;
  case resource::BlendOp_InvSrcColor: blend_src = BGFX_STATE_BLEND_INV_SRC_COLOR; break;
  case resource::BlendOp_SrcAlpha: blend_src = BGFX_STATE_BLEND_SRC_ALPHA; break;
  case resource::BlendOp_InvSrcAlpha: blend_src = BGFX_STATE_BLEND_INV_SRC_ALPHA; break;
  case resource::BlendOp_DstAlpha: blend_src = BGFX_STATE_BLEND_DST_ALPHA; break;
  case resource::BlendOp_InvDstAlpha: blend_src = BGFX_STATE_BLEND_INV_DST_ALPHA; break;
  case resource::BlendOp_DstColor: blend_src = BGFX_STATE_BLEND_DST_COLOR; break;
  case resource::BlendOp_InvDstColor: blend_src = BGFX_STATE_BLEND_INV_DST_COLOR; break;
  case resource::BlendOp_SrcAlphaSat: blend_src = BGFX_STATE_BLEND_SRC_ALPHA_SAT; break;
  case resource::BlendOp_Factor: blend_src = BGFX_STATE_BLEND_FACTOR; break;
  case resource::BlendOp_InvFactor: blend_src = BGFX_STATE_BLEND_INV_FACTOR; break;
  };
  switch (in_data->blendDst()) {
  case resource::BlendOp_Zero: blend_dst = BGFX_STATE_BLEND_ZERO; break;
  case resource::BlendOp_One: blend_dst = BGFX_STATE_BLEND_ONE; break;
  case resource::BlendOp_SrcColor: blend_dst = BGFX_STATE_BLEND_SRC_COLOR; break;
  case resource::BlendOp_InvSrcColor: blend_dst = BGFX_STATE_BLEND_INV_SRC_COLOR; break;
  case resource::BlendOp_SrcAlpha: blend_dst = BGFX_STATE_BLEND_SRC_ALPHA; break;
  case resource::BlendOp_InvSrcAlpha: blend_dst = BGFX_STATE_BLEND_INV_SRC_ALPHA; break;
  case resource::BlendOp_DstAlpha: blend_dst = BGFX_STATE_BLEND_DST_ALPHA; break;
  case resource::BlendOp_InvDstAlpha: blend_dst = BGFX_STATE_BLEND_INV_DST_ALPHA; break;
  case resource::BlendOp_DstColor: blend_dst = BGFX_STATE_BLEND_DST_COLOR; break;
  case resource::BlendOp_InvDstColor: blend_dst = BGFX_STATE_BLEND_INV_DST_COLOR; break;
  case resource::BlendOp_SrcAlphaSat: blend_dst = BGFX_STATE_BLEND_SRC_ALPHA_SAT; break;
  case resource::BlendOp_Factor: blend_dst = BGFX_STATE_BLEND_FACTOR; break;
  case resource::BlendOp_InvFactor: blend_dst = BGFX_STATE_BLEND_INV_FACTOR; break;
  };
  switch (in_data->blendFn()) {
  case resource::BlendEq_Add: blend_fn = BGFX_STATE_BLEND_EQUATION_ADD; break;
  case resource::BlendEq_Sub: blend_fn = BGFX_STATE_BLEND_EQUATION_SUB; break;
  case resource::BlendEq_Revsub: blend_fn = BGFX_STATE_BLEND_EQUATION_REVSUB; break;
  case resource::BlendEq_Min: blend_fn = BGFX_STATE_BLEND_EQUATION_MIN; break;
  case resource::BlendEq_Max: blend_fn = BGFX_STATE_BLEND_EQUATION_MAX; break;
  }
  switch (in_data->alphaBlendSrc()) {
  case resource::BlendOp_Zero: blend_src_alpha = BGFX_STATE_BLEND_ZERO; break;
  case resource::BlendOp_One: blend_src_alpha = BGFX_STATE_BLEND_ONE; break;
  case resource::BlendOp_SrcColor: blend_src_alpha = BGFX_STATE_BLEND_SRC_COLOR; break;
  case resource::BlendOp_InvSrcColor: blend_src_alpha = BGFX_STATE_BLEND_INV_SRC_COLOR; break;
  case resource::BlendOp_SrcAlpha: blend_src_alpha = BGFX_STATE_BLEND_SRC_ALPHA; break;
  case resource::BlendOp_InvSrcAlpha: blend_src_alpha = BGFX_STATE_BLEND_INV_SRC_ALPHA; break;
  case resource::BlendOp_DstAlpha: blend_src_alpha = BGFX_STATE_BLEND_DST_ALPHA; break;
  case resource::BlendOp_InvDstAlpha: blend_src_alpha = BGFX_STATE_BLEND_INV_DST_ALPHA; break;
  case resource::BlendOp_DstColor: blend_src_alpha = BGFX_STATE_BLEND_DST_COLOR; break;
  case resource::BlendOp_InvDstColor: blend_src_alpha = BGFX_STATE_BLEND_INV_DST_COLOR; break;
  case resource::BlendOp_SrcAlphaSat: blend_src_alpha = BGFX_STATE_BLEND_SRC_ALPHA_SAT; break;
  case resource::BlendOp_Factor: blend_src_alpha = BGFX_STATE_BLEND_FACTOR; break;
  case resource::BlendOp_InvFactor: blend_src_alpha = BGFX_STATE_BLEND_INV_FACTOR; break;
  };
  switch (in_data->alphaBlendDst()) {
  case resource::BlendOp_Zero: blend_dst_alpha = BGFX_STATE_BLEND_ZERO; break;
  case resource::BlendOp_One: blend_dst_alpha = BGFX_STATE_BLEND_ONE; break;
  case resource::BlendOp_SrcColor: blend_dst_alpha = BGFX_STATE_BLEND_SRC_COLOR; break;
  case resource::BlendOp_InvSrcColor: blend_dst_alpha = BGFX_STATE_BLEND_INV_SRC_COLOR; break;
  case resource::BlendOp_SrcAlpha: blend_dst_alpha = BGFX_STATE_BLEND_SRC_ALPHA; break;
  case resource::BlendOp_InvSrcAlpha: blend_dst_alpha = BGFX_STATE_BLEND_INV_SRC_ALPHA; break;
  case resource::BlendOp_DstAlpha: blend_dst_alpha = BGFX_STATE_BLEND_DST_ALPHA; break;
  case resource::BlendOp_InvDstAlpha: blend_dst_alpha = BGFX_STATE_BLEND_INV_DST_ALPHA; break;
  case resource::BlendOp_DstColor: blend_dst_alpha = BGFX_STATE_BLEND_DST_COLOR; break;
  case resource::BlendOp_InvDstColor: blend_dst_alpha = BGFX_STATE_BLEND_INV_DST_COLOR; break;
  case resource::BlendOp_SrcAlphaSat: blend_dst_alpha = BGFX_STATE_BLEND_SRC_ALPHA_SAT; break;
  case resource::BlendOp_Factor: blend_dst_alpha = BGFX_STATE_BLEND_FACTOR; break;
  case resource::BlendOp_InvFactor: blend_dst_alpha = BGFX_STATE_BLEND_INV_FACTOR; break;
  };
  switch (in_data->alphaBlendFn()) {
  case resource::BlendEq_Add: blend_fn_alpha = BGFX_STATE_BLEND_EQUATION_ADD; break;
  case resource::BlendEq_Sub: blend_fn_alpha = BGFX_STATE_BLEND_EQUATION_SUB; break;
  case resource::BlendEq_Revsub: blend_fn_alpha = BGFX_STATE_BLEND_EQUATION_REVSUB; break;
  case resource::BlendEq_Min: blend_fn_alpha = BGFX_STATE_BLEND_EQUATION_MIN; break;
  case resource::BlendEq_Max: blend_fn_alpha = BGFX_STATE_BLEND_EQUATION_MAX; break;
  }
  stateMask |= BGFX_STATE_BLEND_FUNC_SEPARATE(blend_src, blend_dst, blend_src_alpha, blend_dst_alpha);
  stateMask |= BGFX_STATE_BLEND_EQUATION_SEPARATE(blend_fn, blend_fn_alpha);
  switch (in_data->cull()) {
  case resource::CullMode_None: break;
  case resource::CullMode_CW: stateMask |= BGFX_STATE_CULL_CW; break;
  case resource::CullMode_CCW: stateMask |= BGFX_STATE_CULL_CCW; break;
  }
  return true;
}
}
}
