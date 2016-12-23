/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "vectormath_aos.h"
#include "vec_aos.h"

namespace hart {
namespace math {
typedef Vectormath::Aos::Vector3 Vec3;
typedef Vectormath::Aos::Vector4 Vec4;
}
}

typedef hart::math::Vec3 hVec3;
typedef hart::math::Vec4 hVec4;
