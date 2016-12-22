/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/config.h"

#pragma once

namespace hart {
namespace input {

struct ButtonState {
    union {
        uint8_t flags;
        struct {
            bool raisingEdge : 1;
            bool fallingEdge : 1;
            bool state       : 1;
        };
    };
};

struct AxisState {
    int16_t  axisValue;
};

static const uint32_t MaxInputBindingNameLen = 64;

struct InputBinding {
    uint16_t actionID;
    char     platformName[MaxInputBindingNameLen];
};

void initialise();
void postUpdate();
void setupInputBindings(uint8_t pad_id, InputBinding const* bindings, uint32_t binds_count);
ButtonState getButtonState(uint16_t action_id, uint8_t pad_id);
AxisState getAxisState(uint16_t action_id, uint8_t pad_id);

}
}

namespace hin = hart::input;
