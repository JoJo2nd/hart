/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "game_state.h"

HART_OBJECT_TYPE_DECL(GameState);

bool GameState::deserialiseObject(MarshallType const* in_data, hobjfact::SerialiseParams const&) {
    return true;
}