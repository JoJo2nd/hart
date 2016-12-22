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
static const uint32_t MaxControllerButtons = 32;
// SDL_GameControllerGetAxisFromString()
static const uint32_t MaxControllerAxes = 16;
static const uint32_t MaxControllerHats = 16;
static const uint32_t MaxControllerHatMappings = MaxControllerHats * 4; // one for each hat direction

static const uint32_t MaxMappinds = MaxScancodes + MaxControllerButtons + MaxControllerAxes + MaxControllerHatMappings;
static const uint32_t CtrButtonFirstIndex = MaxScancodes;
static const uint32_t CtrAxisFirstIndex = MaxScancodes + MaxControllerButtons;
static const uint32_t CtrHatFirstIndex = MaxScancodes + MaxControllerButtons + MaxControllerAxes;
enum class CtrHatAxis {
  Up = 0,
  Down = 1,
  Left = 2,
  Right = 3,

  Max
};

static const uint16_t InvalidMapping = (uint16_t)~0;
static const uint8_t  SysKeyboardIndex = 0;

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
      if (hcrt::strlen(SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)i))))
  {
          hdbprintf("Keycode: %s\n",
  SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)i)));
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
  // reg events
  events.push_back(
    addEventHandler(engine::Event::JoyDeviceAdded, [&](engine::Event evt_id, engine::EventData const* evt) {
      if (evt->jdevice.which < MaxPads) {
        sysPads[evt->jdevice.which].joy = SDL_JoystickOpen(evt->jdevice.which);
      }
    }));
  events.push_back(
    addEventHandler(engine::Event::JoyDeviceRemoved, [&](engine::Event evt_id, engine::EventData const* evt) {
      if (evt->jdevice.which < MaxPads) {
        SDL_JoystickClose(sysPads[evt->jdevice.which].joy);
        sysPads[evt->jdevice.which].joy = nullptr;
      }
    }));
  events.push_back(
    addEventHandler(engine::Event::JoyButtonDown, [&](engine::Event evt_id, engine::EventData const* evt) {
      if (evt->jbutton.which < MaxPads && evt->jbutton.button < MaxControllerButtons) {
        uint16_t aid = sysPads[evt->jbutton.which].actionMappings[evt->jbutton.button + CtrButtonFirstIndex];
        if (aid != InvalidMapping) {
          sysPads[evt->jbutton.which].buttons[aid].raisingEdge = true;
          sysPads[evt->jbutton.which].buttons[aid].fallingEdge = false;
          sysPads[evt->jbutton.which].buttons[aid].state = true;
        }
      }
    }));
  events.push_back(addEventHandler(engine::Event::JoyButtonUp, [&](engine::Event evt_id, engine::EventData const* evt) {
    if (evt->jbutton.which < MaxPads && evt->jbutton.button < MaxControllerButtons) {
      uint16_t aid = sysPads[evt->jbutton.which].actionMappings[evt->jbutton.button + CtrButtonFirstIndex];
      if (aid != InvalidMapping) {
        sysPads[evt->jbutton.which].buttons[aid].raisingEdge = false;
        sysPads[evt->jbutton.which].buttons[aid].fallingEdge = true;
        sysPads[evt->jbutton.which].buttons[aid].state = false;
      }
    }
  }));
  events.push_back(
    addEventHandler(engine::Event::JoyHatMotion, [&](engine::Event evt_id, engine::EventData const* evt) {
      if (evt->jhat.which < MaxPads && evt->jhat.hat < MaxControllerHats) {
        uint16_t idx[2] = {0, 0};
        uint16_t unset[(uint16_t)CtrHatAxis::Max] = {0, 1, 2, 3};
        switch (evt->jhat.value) {
        case SDL_HAT_LEFTUP:
          idx[0] = evt->jhat.hat + CtrHatFirstIndex + (int16_t)CtrHatAxis::Up;
          idx[1] = evt->jhat.hat + CtrHatFirstIndex + (int16_t)CtrHatAxis::Left;
          break;
        case SDL_HAT_UP: idx[0] = evt->jhat.hat + CtrHatFirstIndex + (int16_t)CtrHatAxis::Up; break;
        case SDL_HAT_RIGHTUP:
          idx[0] = evt->jhat.hat + CtrHatFirstIndex + (int16_t)CtrHatAxis::Up;
          idx[1] = evt->jhat.hat + CtrHatFirstIndex + (int16_t)CtrHatAxis::Right;
          break;
        case SDL_HAT_RIGHT: idx[0] = evt->jhat.hat + CtrHatFirstIndex + (int16_t)CtrHatAxis::Right; break;
        case SDL_HAT_RIGHTDOWN:
          idx[0] = evt->jhat.hat + CtrHatFirstIndex + (int16_t)CtrHatAxis::Right;
          idx[1] = evt->jhat.hat + CtrHatFirstIndex + (int16_t)CtrHatAxis::Down;
          break;
        case SDL_HAT_DOWN: idx[0] = evt->jhat.hat + CtrHatFirstIndex + (int16_t)CtrHatAxis::Down; break;
        case SDL_HAT_LEFTDOWN:
          idx[0] = evt->jhat.hat + CtrHatFirstIndex + (int16_t)CtrHatAxis::Down;
          idx[1] = evt->jhat.hat + CtrHatFirstIndex + (int16_t)CtrHatAxis::Left;
          break;
        case SDL_HAT_LEFT: idx[0] = evt->jhat.hat + CtrHatFirstIndex + (int16_t)CtrHatAxis::Left; break;
        default: // Centred. which requires no action and clears all other
                 // buttons.
          break;
        }
        for (uint32_t i = 0; i < HART_ARRAYSIZE(idx); ++i) {
          if (idx[i] < (evt->jhat.hat + CtrHatFirstIndex)) continue;
          uint16_t aid = sysPads[evt->jhat.which].actionMappings[idx[i]];
          if (aid != InvalidMapping) {
            unset[idx[i] - (evt->jhat.hat + CtrHatFirstIndex)] = InvalidMapping;
            if (!sysPads[evt->jhat.which].buttons[aid].state) {
              sysPads[evt->jhat.which].buttons[aid].raisingEdge = true;
              sysPads[evt->jhat.which].buttons[aid].fallingEdge = false;
              sysPads[evt->jhat.which].buttons[aid].state = true;
            }
          }
        }
        for (uint32_t i = 0; i < HART_ARRAYSIZE(unset); ++i) {
          if (unset[i] != InvalidMapping) {
            uint16_t aid = sysPads[evt->jhat.which].actionMappings[evt->jhat.hat + CtrHatFirstIndex + i];
            if (aid != InvalidMapping && sysPads[evt->jhat.which].buttons[aid].state) {
              sysPads[evt->jhat.which].buttons[aid].raisingEdge = false;
              sysPads[evt->jhat.which].buttons[aid].fallingEdge = true;
              sysPads[evt->jhat.which].buttons[aid].state = false;
            }
          }
        }
      }
    }));
  events.push_back(
    addEventHandler(engine::Event::JoyAxisMotion, [&](engine::Event evt_id, engine::EventData const* evt) {
      if (evt->jaxis.which < MaxPads && evt->jaxis.axis < MaxControllerAxes) {
        uint16_t aid = sysPads[evt->jaxis.which].actionMappings[evt->jaxis.axis + CtrAxisFirstIndex];
        if (aid != InvalidMapping) {
          sysPads[evt->jaxis.which].axes[aid].axisValue = evt->jaxis.value;
        }
      }
    }));
  events.push_back(addEventHandler(engine::Event::KeyDown, [&](engine::Event evt_id, engine::EventData const* evt) {
    uint16_t aid = sysPads[SysKeyboardIndex].actionMappings[evt->key.keysym.scancode];
    if (aid != InvalidMapping) {
      sysPads[SysKeyboardIndex].buttons[aid].raisingEdge = true;
      sysPads[SysKeyboardIndex].buttons[aid].fallingEdge = false;
      sysPads[SysKeyboardIndex].buttons[aid].state = true;
    }
  }));
  events.push_back(addEventHandler(engine::Event::KeyUp, [&](engine::Event evt_id, engine::EventData const* evt) {
    uint16_t aid = sysPads[SysKeyboardIndex].actionMappings[evt->key.keysym.scancode];
    if (aid != InvalidMapping) {
      sysPads[SysKeyboardIndex].buttons[aid].raisingEdge = false;
      sysPads[SysKeyboardIndex].buttons[aid].fallingEdge = true;
      sysPads[SysKeyboardIndex].buttons[aid].state = false;
    }
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
      uint32_t     idx = (uint16_t)sc;
      sysPads[pad_id].actionMappings[idx] = bindings[i].actionID;
      if (sysPads[pad_id].buttons.size() <= bindings[i].actionID) {
        sysPads[pad_id].buttons.resize(bindings[i].actionID + 1);
      }
    } else if (hcrt::strncmp(bindings[i].platformName, "JoyBtn", 6) == 0) {
      int      bt = hcrt::atoi(bindings[i].platformName + 6);
      uint32_t idx = CtrButtonFirstIndex + (uint16_t)bt;
      sysPads[pad_id].actionMappings[idx] = bindings[i].actionID;
      if (sysPads[pad_id].buttons.size() <= bindings[i].actionID) {
        sysPads[pad_id].buttons.resize(bindings[i].actionID + 1);
      }
    } else if (hcrt::strncmp(bindings[i].platformName, "JoyAxis", 7) == 0) {
      int      ax = hcrt::atoi(bindings[i].platformName + 7);
      uint32_t idx = CtrAxisFirstIndex + (uint16_t)ax;
      sysPads[pad_id].actionMappings[idx] = bindings[i].actionID;
      if (sysPads[pad_id].axes.size() <= bindings[i].actionID) {
        sysPads[pad_id].axes.resize(bindings[i].actionID + 1);
      }
    } else if (hcrt::strncmp(bindings[i].platformName, "JoyHat", 6) == 0) {
      // this is in the format JoyHat[index][Up|Down|Left|Right]
      char*    dir = nullptr;
      int32_t  hat = strtol(bindings[i].platformName + 6, &dir, 10);
      uint32_t idx = 0;
      if (!dir) continue; // Skip binding, failed to parse
      if (hcrt::strcmp(dir, "Up") == 0) {
        idx = CtrHatFirstIndex + (uint16_t)hat + (uint16_t)CtrHatAxis::Up;
      } else if (hcrt::strcmp(dir, "Down") == 0) {
        idx = CtrHatFirstIndex + (uint16_t)hat + (uint16_t)CtrHatAxis::Down;
      } else if (hcrt::strcmp(dir, "Left") == 0) {
        idx = CtrHatFirstIndex + (uint16_t)hat + (uint16_t)CtrHatAxis::Left;
      } else if (hcrt::strcmp(dir, "Right") == 0) {
        idx = CtrHatFirstIndex + (uint16_t)hat + (uint16_t)CtrHatAxis::Right;
      } else
        continue; // Skip binding, failed to parse
      sysPads[pad_id].actionMappings[idx] = bindings[i].actionID;
      if (sysPads[pad_id].buttons.size() <= bindings[i].actionID) {
        sysPads[pad_id].buttons.resize(bindings[i].actionID + 1);
      }
    }
  }
}

ButtonState getButtonState(uint16_t action_id, uint8_t pad_id) {
  if (pad_id < MaxPads && action_id < sysPads[pad_id].buttons.size()) return sysPads[pad_id].buttons[action_id];
  return ButtonState();
}

AxisState getAxisState(uint16_t action_id, uint8_t pad_id) {
  if (pad_id < MaxPads && action_id < sysPads[pad_id].axes.size()) return sysPads[pad_id].axes[action_id];
  return AxisState();
}
}
}