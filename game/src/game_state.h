/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#pragma once

#include "hart/hart.h"
#include "fbs/gamestate_generated.h"

class GameState {
    HART_OBJECT_TYPE(HART_MAKE_FOURCC('g','a','m','e'), GameStateData)
};