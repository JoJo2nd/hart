/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/hart.h"

// 2D AABB with min stored in x,y/0,1 max stored in z,w/2,3
typedef hVec4 AABB;

inline AABB aabbFromPoints(hVec3 p1, hVec3 p2) {
  return AABB(hutil::tmin(p1[0], p2[0]), hutil::tmin(p1[1], p2[1]), hutil::tmax(p1[0], p2[0]),
              hutil::tmax(p1[1], p2[1]));
}

/*
 *
 * Functions defined here are reference. You may need to replace with hand
 * written SIMD one in cases where you wish to
 * batch together many tests (e.g. collision detection).
 *
 */

// TODO:
// void buildConcavePolySoup(hVec4 const* points, uint32_t point_count);

// TODO:
// bool satIntersectMovingPolys(ColPoly const& a, ColPoly const& b, hVec3 va,
// hVec3 vb, float* first_t, float* last_t);

inline void closestPointOnAABB(hVec3 p, AABB a, hVec3* out_p) {
  for (uint32_t i = 0; i < 2; ++i) {
    float v = p[i];
    if (v < a[i]) v = a[i];
    if (v > a[i + 2]) v = a[i + 2];
    (*out_p)[i] = v;
  }
}

inline bool intersectRayAABB(hVec3 p, hVec3 d, AABB a, float* min_t, hVec3* out_p) {
  float tmin = 0.f;
  float tmax = FLT_MAX;
  for (uint32_t i = 0; i < 2; ++i) {
    if (hcrt::fabs(d[i]) < FLT_EPSILON) {
      // Ray is parallel to axis
      if (p[i] < a[i] || p[i] > a[i + 2]) return false;
    } else {
      float ood = 1.0f / d[i];
      float t1 = (a[i] - p[i]) * ood;
      float t2 = (a[i + 2] - p[i]) * ood;
      if (t1 > t2) hutil::tswap(t1, t2);
      if (t1 > tmin) tmin = t1;
      if (t2 < tmax) tmax = t2;
      if (tmin > tmax) return false;
    }
  }

  (*min_t) = tmin;
  (*out_p) = p + d * tmin;
  return true;
}

inline bool intersectAABB(AABB a, AABB b) {
  if (a[2] < b[0] || a[0] > b[2]) return false;
  if (a[3] < b[1] || a[1] > b[3]) return false;
  return true;
}

inline bool intersectMovingAABB(AABB a, AABB b, hVec3 va, hVec3 vb, float* first_t, float* last_t) {
  // See Realtime collision dectection section 5.5.8
  if (intersectAABB(a, b)) {
    *first_t = *last_t = 0.0f;
    return true;
  }
  hVec3 v = vb - va;
  float tfirst = 0.f;
  float tlast = 1.f;

  for (uint32_t i = 0; i < 2; ++i) {
    if (v[i] < 0.f) {
      if (b[i + 2] < a[i]) return false;
      if (a[i + 2] < b[i]) tfirst = hutil::tmax((a[i + 2] - b[i]) / v[i], tfirst);
      if (b[i + 2] > a[i]) tlast = hutil::tmin((a[i] - b[i + 2]) / v[i], tlast);
    } else if (v[i] > 0.f) {
      if (b[i] > a[i + 2]) return false;
      if (b[i + 2] < a[i]) tfirst = hutil::tmax((a[i] - b[i + 2]) / v[i], tfirst);
      if (a[i + 2] > b[i]) tlast = hutil::tmin((a[i + 2] - b[i]) / v[i], tlast);
    } else {
      if (a[i + 2] < b[i] || a[i] > b[i + 2]) return false;
    }

    // no overlap if time of first contact is after last contact
    if (tfirst > tlast) return false;
  }

  *first_t = tfirst;
  *last_t = tlast;
  return true;
}