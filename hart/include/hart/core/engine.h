/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
//TODO: Define the interface to start and run the engine. Remove this pad/input crud
#pragma once

#include "hart/config.h"
#include <string.h> // memset
#include <bx/bx.h>

#define ENTRY_WINDOW_FLAG_NONE         UINT32_C(0x00000000)
#define ENTRY_WINDOW_FLAG_ASPECT_RATIO UINT32_C(0x00000001)
#define ENTRY_WINDOW_FLAG_FRAME        UINT32_C(0x00000002)

namespace hart {
namespace engine {

int32_t run(int argc,char* argv[]);

}
} 

namespace hEngine = hart::engine;