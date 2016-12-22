/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "mat_aos.h"
#include "vectormath_aos.h"

namespace hart {
namespace math {
typedef Vectormath::Aos::Matrix4 Mat44;
typedef Vectormath::Aos::Matrix3 Mat33;
}
}

typedef hart::math::Mat44 hMat44;
typedef hart::math::Mat33 hMat33;
