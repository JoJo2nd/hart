/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/core/input.h"
#include "hart/base/std.h"
#include "hart/core/engine.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

namespace hart {
namespace input {

static const uint32_t MaxPads = 4;
// char const* name = SDL_GetKeyName(SDL_GetKeyFromScancode(scancode))
// SDL_Scancode = SDL_GetScancodeFromKey(SDL_GetKeyFromName(keyname))
static const uint32_t MaxScancodes = SDL_NUM_SCANCODES;
// SDL_GameControllerGetButtonFromString()
static const uint32_t MaxControllerButtons = SDL_CONTROLLER_BUTTON_MAX;
// SDL_GameControllerGetAxisFromString()
static const uint32_t MaxControllerAxes = SDL_CONTROLLER_AXIS_MAX;

static const uint32_t MaxMappinds = MaxScancodes + MaxControllerButtons + MaxControllerAxes;

static const uint16_t InvalidMapping = (uint16_t)~0;
static const uint8_t SysKeyboardIndex = 0;

static struct Pad {
    hstd::vector<ButtonState> buttons;
    hstd::vector<AxisState>   axes;
    uint16_t                  actionMappings[MaxMappinds];
    SDL_Joystick*             joy = nullptr;
} sysPads[MaxPads];
static hstd::vector<engine::EventHandle> events;

void initialise() {
    /* Print key names for reference
    for (uint32_t i=0; i < SDL_NUM_SCANCODES; ++i) {
        if (hcrt::strlen(SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)i)))) {
            hdbprintf("Keycode: %s\n", SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)i)));
        }
    }
    for (uint32_t i=0; i < MaxControllerButtons; ++i) {
        hdbprintf("JoyButton: JoyBtn%d\n",i);
    }
    for (uint32_t i=0; i < MaxControllerAxes; ++i) {
        hdbprintf("JoyAxis: JoyAxis%d\n",i);
    }
    //*/

    for (uint32_t p = 0; p < MaxPads; ++p) {
        sysPads[p].joy = SDL_JoystickOpen(p);
        if (!sysPads[p].joy && p != SysKeyboardIndex) continue;
        for (uint32_t i = 0, n = MaxMappinds; i < n; ++i) {
            sysPads[p].actionMappings[i] = InvalidMapping;
        }
    }
    //reg events
    //TODO: handle multiple events in single frame? or only process events at 60Hz?
    events.push_back(addEventHandler(engine::Event::JoyDeviceAdded, [&](engine::Event evt_id, engine::EventData const* evt) {
        if (evt->jdevice.which < MaxPads) {
            sysPads[evt->jdevice.which].joy = SDL_JoystickOpen(evt->jdevice.which);
        }
    }));
    events.push_back(addEventHandler(engine::Event::JoyDeviceRemoved, [&](engine::Event evt_id, engine::EventData const* evt) {
        if (evt->jdevice.which < MaxPads) {
            SDL_JoystickClose(sysPads[evt->jdevice.which].joy);
            sysPads[evt->jdevice.which].joy = nullptr;
        }
    }));
    events.push_back(addEventHandler(engine::Event::JoyButtonDown, [&](engine::Event evt_id, engine::EventData const* evt) {
        if (evt->jbutton.which < MaxPads && evt->jbutton.button < MaxControllerButtons) {
            uint16_t aid = sysPads[evt->jbutton.which].actionMappings[evt->jbutton.button+MaxScancodes];
            sysPads[evt->jbutton.which].buttons[aid].raisingEdge = true;
            sysPads[evt->jbutton.which].buttons[aid].fallingEdge = false;
            sysPads[evt->jbutton.which].buttons[aid].state = true;
        }
    }));
    events.push_back(addEventHandler(engine::Event::JoyButtonUp, [&](engine::Event evt_id, engine::EventData const* evt) {
        if (evt->jbutton.which < MaxPads && evt->jbutton.button < MaxControllerButtons) {
            uint16_t aid = sysPads[evt->jbutton.which].actionMappings[evt->jbutton.button+MaxScancodes];
            sysPads[evt->jbutton.which].buttons[aid].raisingEdge = false;
            sysPads[evt->jbutton.which].buttons[aid].fallingEdge = true;
            sysPads[evt->jbutton.which].buttons[aid].state = false;
        }
    }));
    events.push_back(addEventHandler(engine::Event::JoyAxisMotion, [&](engine::Event evt_id, engine::EventData const* evt) {
        if (evt->jaxis.which < MaxPads && evt->jaxis.axis < MaxControllerAxes) {
            uint16_t aid = sysPads[evt->jaxis.which].actionMappings[evt->jaxis.axis+MaxScancodes+MaxControllerButtons];
            sysPads[evt->jaxis.which].axes[aid].axisValue = evt->jaxis.value;
        }
    }));
    events.push_back(addEventHandler(engine::Event::KeyDown, [&](engine::Event evt_id, engine::EventData const* evt) {
        uint16_t aid = sysPads[SysKeyboardIndex].actionMappings[evt->key.keysym.scancode];
        sysPads[SysKeyboardIndex].buttons[aid].raisingEdge = true;
        sysPads[SysKeyboardIndex].buttons[aid].fallingEdge = false;
        sysPads[SysKeyboardIndex].buttons[aid].state = true;
    }));
    events.push_back(addEventHandler(engine::Event::KeyUp, [&](engine::Event evt_id, engine::EventData const* evt) {
        uint16_t aid = sysPads[SysKeyboardIndex].actionMappings[evt->key.keysym.scancode];
        sysPads[SysKeyboardIndex].buttons[aid].raisingEdge = false;
        sysPads[SysKeyboardIndex].buttons[aid].fallingEdge = true;
        sysPads[SysKeyboardIndex].buttons[aid].state = false;
    }));
}

void postUpdate() {
    for (uint32_t p = 0; p < MaxPads; ++p) {
        if (!sysPads[p].joy && p != SysKeyboardIndex) continue;
        for (auto& b : sysPads[p].buttons) {
            b.raisingEdge = false;
            b.fallingEdge = false;
        }
    }
}

void setupInputBindings(uint8_t pad_id, InputBinding const* bindings, uint32_t binds_count) {
    for (uint32_t i = 0, n = MaxMappinds; i < n; ++i) {
        sysPads[pad_id].actionMappings[i] = InvalidMapping;
    }
    for (uint32_t i = 0; i < binds_count; ++i) {
        if (SDL_GetKeyFromName(bindings[i].platformName) != SDLK_UNKNOWN) {
            SDL_Scancode sc = SDL_GetScancodeFromKey(SDL_GetKeyFromName(bindings[i].platformName));
            uint32_t idx = (uint16_t)sc;
            sysPads[pad_id].actionMappings[idx] = bindings[i].actionID;
            if (sysPads[pad_id].buttons.size() <= bindings[i].actionID) {
                sysPads[pad_id].buttons.resize(bindings[i].actionID+1);
            }
        } else if (SDL_GameControllerGetButtonFromString(bindings[i].platformName) != SDL_CONTROLLER_BUTTON_INVALID) {
            SDL_GameControllerButton bt = SDL_GameControllerGetButtonFromString(bindings[i].platformName);
            uint32_t idx = MaxScancodes + (uint16_t)bt;
            sysPads[pad_id].actionMappings[idx] = bindings[i].actionID;
            if (sysPads[pad_id].buttons.size() <= bindings[i].actionID) {
                sysPads[pad_id].buttons.resize(bindings[i].actionID+1);
            }
        } else if (SDL_GameControllerGetAxisFromString(bindings[i].platformName) != SDL_CONTROLLER_AXIS_INVALID) {
            SDL_GameControllerAxis ax = SDL_GameControllerGetAxisFromString(bindings[i].platformName);
            uint32_t idx = MaxScancodes + MaxControllerButtons + (uint16_t)ax;
            if (sysPads[pad_id].axes.size() <= bindings[i].actionID) {
                sysPads[pad_id].axes.resize(bindings[i].actionID+1);
            }
        }
    }
}

ButtonState getButtonState(uint16_t action_id, uint8_t pad_id) {
    if (pad_id < MaxPads && action_id < sysPads[pad_id].buttons.size())
        return sysPads[pad_id].buttons[action_id];
    return ButtonState();
}

AxisState getAxisState(uint16_t action_id, uint8_t pad_id) {
    if (pad_id < MaxPads && action_id < sysPads[pad_id].axes.size())
        return sysPads[pad_id].axes[action_id];
    return AxisState();
}

}
}