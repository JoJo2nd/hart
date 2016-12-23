/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#pragma once

#include "fbs/gamestate_generated.h"
#include "hart/hart.h"

class GameState {
  HART_OBJECT_TYPE(HART_MAKE_FOURCC('g', 'a', 'm', 'e'), GameStateData)
};