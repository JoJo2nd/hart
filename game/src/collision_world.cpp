/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "collision_world.h"

namespace collisionworld {

struct PrimitiveNode {
  static const uint16_t NullNode = (uint16_t)~0u;
  AABB                  aabb; // primitive if not in freelist
  uint16_t              next; // freelist next
  bool                  free : 1;
};

static struct World {
  hstd::vector<PrimitiveNode> primitives;
  uint16_t                    freelistHead;
  uint16_t                    primCount;
} world;


#if HART_DEBUG_INFO
static bool  debugRenderPrims = false;
static bool  debugRayTest = false;
static bool  debugAABBTest = false;
static float debugStart[2] = {10.f * 8.f, 6.f * 8.f};
static float debugEnd[2] = {16.f * 8.f, 28.f * 8.f};
static float debugBox[2] = {16.f, 35.f};
#endif

void initialiseWorld() {
  world.primCount = hconfigopt::getUint("collisionworld", "maxprimitives", 1024);
  world.primitives.resize(world.primCount);
  for (int32_t i = 0; i < (world.primCount - 1); ++i) {
    world.primitives[i].next = i + 1;
    world.primitives[i].free = true;
  }
  world.primitives[world.primCount - 1].next = PrimitiveNode::NullNode;
  world.freelistHead = 0;
  world.primCount = 0;
#if HART_DEBUG_INFO
  hart::engine::addDebugMenu("Collision World", []() {
    if (ImGui::Begin("Collision World Debug", nullptr, 0)) {
      ImGui::Checkbox("Render collision primitives", &debugRenderPrims);
      ImGui::Checkbox("Enable debug ray test", &debugRayTest);
      if (debugRayTest) {
        ImGui::InputFloat2("Ray start", debugStart);
        ImGui::InputFloat2("Ray end", debugEnd);
      }
      ImGui::Checkbox("Enable debug moving box test", &debugAABBTest);
      if (debugAABBTest) {
        ImGui::InputFloat2("Box start", debugStart);
        ImGui::InputFloat2("Box end", debugEnd);
        ImGui::InputFloat2("Box Width, Height", debugBox);
      }
    }
    ImGui::End();
    if (debugRenderPrims) {
      for (uint16_t i = 0, in = (uint16_t)world.primitives.size(), p = 0, pn = world.primCount; i < in && p < pn; ++i) {
        if (world.primitives[i].free) continue;
        AABB const& aabb = world.primitives[i].aabb;
        hVec3       tl(aabb.getX(), aabb.getY(), 0.f);
        hVec3       tr(aabb.getZ(), aabb.getY(), 0.f);
        hVec3       bl(aabb.getX(), aabb.getW(), 0.f);
        hVec3       br(aabb.getZ(), aabb.getW(), 0.f);
        hrnd::debug::addLine(tl, tr, 0xFF0000FF);
        hrnd::debug::addLine(tr, br, 0xFF0000FF);
        hrnd::debug::addLine(br, bl, 0xFF0000FF);
        hrnd::debug::addLine(bl, tl, 0xFF0000FF);
        hrnd::debug::addQuad(tl, br, 0x400000FF);
        ++p;
      }
    }
    if (debugRayTest) {
      hVec3 intersect;
      float t;
      hVec3 s(debugStart[0], debugStart[1], 0.f);
      hVec3 e(debugEnd[0] - debugStart[0], debugEnd[1] - debugStart[1], 0.f);

      if (raytestWorld(s, e, &t, &intersect)) {
        hrnd::debug::addLine(s, intersect, 0xFF00FF00);
        hrnd::debug::addLine(intersect, s + e, 0xFF0000FF);
      } else {
        hrnd::debug::addLine(s, s + e, 0xFF00FF00);
      }
    }
    if (debugAABBTest) {
      AABB aabb = aabbFromPoints(hVec3(debugStart[0] - (debugBox[0] / 2), debugStart[1] - (debugBox[1] / 2), 0.f),
                                 hVec3(debugStart[0] + (debugBox[0] / 2), debugStart[1] + (debugBox[1] / 2), 0.f));
      hVec3 centre(debugStart[0], debugStart[1], 0.f);
      hVec3 dir(debugEnd[0] - debugStart[0], debugEnd[1] - debugStart[1], 0.f);
      float first, last;
      hVec3 tl(debugStart[0] - (debugBox[0] / 2), debugStart[1] + (debugBox[1] / 2), 0.f);
      hVec3 tr(debugStart[0] + (debugBox[0] / 2), debugStart[1] + (debugBox[1] / 2), 0.f);
      hVec3 bl(debugStart[0] - (debugBox[0] / 2), debugStart[1] - (debugBox[1] / 2), 0.f);
      hVec3 br(debugStart[0] + (debugBox[0] / 2), debugStart[1] - (debugBox[1] / 2), 0.f);
      hrnd::debug::addQuad(tl, br, 0x40FFFFFF);
      if (intersectMovingAABBWorld(aabb, dir, &first, &last)) {
		  hVec3 ctl = tl + dir * first;
		  hVec3 cbr = br + dir * first;
		  hrnd::debug::addQuad(ctl, cbr, 0x800000FF);
      }
	  first = last = 1.f;
      hVec3 etl = tl + dir * first;
      hVec3 ebr = br + dir * first;
      hrnd::debug::addQuad(etl, ebr, 0x80FF0000);
      hrnd::debug::addLine(centre, centre + dir * first, 0x80FF0000);
    }
  });
#endif
}

ColHandle addStaticPrimitive(AABB const& aabb) {
  ColHandle r = world.freelistHead;
  if (r != PrimitiveNode::NullNode) {
    world.freelistHead = world.primitives[r].next;
    world.primitives[r].aabb = aabb;
    world.primitives[r].free = false;
    ++world.primCount;
  }
  return r;
}

void removeStaticPrimitive(ColHandle hdl) {
  if (hdl == PrimitiveNode::NullNode) return;
  world.primitives[hdl].next = world.freelistHead;
  world.primitives[hdl].free = true;
  world.freelistHead = hdl;
  --world.primCount;
}

bool raytestWorld(hVec3 pt, hVec3 d, float* min_t, hVec3* out_p) {
  /*
   * Note that this method currently brute forces the collision checks. Will likely need improvement but it'll work for
   * now. It at least uses an array for cache friendlyness
   */
  hVec3 pend = pt + d;
  // TODO: need a aabb construct call & struct to get this right
  AABB ray_bounds = aabbFromPoints(pt, pend);
  *min_t = FLT_MAX;

  for (uint16_t i = 0, in = (uint16_t)world.primitives.size(), p = 0, pn = world.primCount; i < in && p < pn; ++i) {
    if (world.primitives[i].free)
      continue; // TODO: sort the list to not contain free elements in the middle of non-free ones.
    if (intersectAABB(world.primitives[i].aabb, ray_bounds)) {
      float mint;
      hVec3 outp;
      if (intersectRayAABB(pt, d, world.primitives[i].aabb, &mint, &outp) && *min_t > mint) {
        *min_t = mint;
        *out_p = outp;
      }
    }
    ++p;
  }
  return *min_t < FLT_MAX;
}

bool intersectMovingAABBWorld(AABB a, hVec3 va, float* first_t, float* last_t) {
  /*
   * Note that this method currently brute forces the collision checks. Will likely need improvement but it'll work for
   * now. It at least uses an array for cache friendlyness
   */
  hVec3 zeroV(0.f, 0.f, 0.f);
  *first_t = FLT_MAX;
  *last_t = -FLT_MAX;
  for (uint16_t i = 0, in = (uint16_t)world.primitives.size(), p = 0, pn = world.primCount; i < in && p < pn; ++i) {
    if (world.primitives[i].free)
      continue; // TODO: sort the list to not contain free elements in the middle of non-free ones.
    float ft, lt;
    if (intersectMovingAABB(a, world.primitives[i].aabb, va, zeroV, &ft, &lt)) {
      *first_t = hutil::tmin(*first_t, ft);
      *last_t = hutil::tmax(*last_t, lt);
    }
  }

  return *first_t < *last_t;
}
}
