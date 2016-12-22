/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#pragma once

#include "hart/hart.h"
#include "poly_utils.h"

namespace collisionworld {

typedef uint16_t ColHandle;

/*
 * Set up an empty collision world. Call only once.
 */
void initialiseWorld();
/*
 * Add an non-moving solid axis aligned bounding box to the world.
 */
ColHandle addStaticPrimitive(AABB const& aabb);
/*
 * Remove an non-moving solid axis aligned bounding box from the world that was
 * previously added.
 */
void removeStaticPrimitive(ColHandle hdl);

/*
 * Ray test ray (p) to (p+d) against the world. Returns true if a collision
 * happened and
 * if so, the distance along the ray and the intersection point are return.
 */
bool raytestWorld(hVec3 p, hVec3 d, float* min_t, hVec3* out_p);
/*
 * Sweep test an axis aligned bounding box through the world. Returns true if
 * something was
 * collided with. When true, the FIRST entry point and the LAST exit point are
 * returned. These are
 * as distances along va.
 */
bool intersectMovingAABBWorld(AABB a, hVec3 va, float* first_t, float* last_t);
}
