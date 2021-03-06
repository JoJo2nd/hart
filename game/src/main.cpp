/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "collision_world.h"
#include "fbs/inputactions_generated.h"
#include "fbs/player_generated.h"
#include "fbs/test_generated.h"
#include "game_state.h"
#include "hart/hart.h"
#include "sprite_renderer.h"

static bool debugRenderTileGrid = false;

class Player : public hety::Component {
  HART_COMPONENT_OBJECT_TYPE(HART_MAKE_FOURCC('p', 'l', 'y', 'r'), resource::Player)
public:
  uint32_t exp;
  uint8_t  level;
  bool     someFlag;
};

HART_COMPONENT_OBJECT_TYPE_DECL(Player);

bool Player::deserialiseObject(MarshallType const*, hobjfact::SerialiseParams const&) {
  return true;
}
bool Player::deserialiseComponent(MarshallType const* overrides, MarshallType const* base) {
  exp = overrides->exp();
  level = overrides->level();
  someFlag = overrides->someFlag() ? overrides->someFlag()->val() : base->someFlag() ? base->someFlag()->val() : false;
  return true;
}

class Test : public hety::Component {
  HART_COMPONENT_OBJECT_TYPE(HART_MAKE_FOURCC('t', 'e', 's', 't'), resource::Test)
public:
  uint32_t exp;
  uint8_t  level;
  uint32_t someTestValue;
};

HART_COMPONENT_OBJECT_TYPE_DECL(Test);

bool Test::deserialiseObject(MarshallType const*, hobjfact::SerialiseParams const&) {
  return true;
}
bool Test::deserialiseComponent(MarshallType const* overrides, MarshallType const* base) {
  exp = overrides->exp();
  level = overrides->level();
  if (overrides->someTestValue())
    someTestValue = overrides->someTestValue()->val();
  else if (base->someTestValue())
    someTestValue = base->someTestValue()->val();
  return true;
}

// Prototype for functions registering classes. Saves having to include the header files here
void registierSpriteObject();
void registierLevelObject();

// save the includes. also only temp/testing.
void updateAllLevelAnimations();

class Game : public hart::engine::GameInterface {
  // 16:9 view for 8x8 tiles (45x30 tiles)
  // 16:10 is 384x240 (48x30 tiles) <- even number so better divider?
  static const uint32_t viewWidth = 360;
  static const uint32_t viewHeight = 240;

  virtual void postObjectFactoryRegister() {
    // TODO: Register any game class objects here.
    hobjfact::objectFactoryRegister(Player::getObjectDefinition(), nullptr);
    hobjfact::objectFactoryRegister(Test::getObjectDefinition(), nullptr);
    registierSpriteObject();
    registierLevelObject();

    // Grab default key bindings from ini
    hstd::vector<hin::InputBinding> i_binds;
    const char**                    k_names = EnumNamesInputAction();

    for (uint32_t i = 0; k_names[i]; ++i) {
      char const*       k_bind = hconfigopt::getStr("binding", k_names[i], "");
      char const*       k_ptr = k_bind;
      char const*       k_start = k_ptr;
      hin::InputBinding b;
      uint32_t          k_size = 0;
      while (*(k_ptr) != '\0' && (uint16_t)(k_ptr - k_start) < hin::MaxInputBindingNameLen - 1) {
        if (*(k_ptr) == ':') {
          b.actionID = i;
          hcrt::strncpy(b.platformName, (uint16_t)(k_ptr - k_start), k_start);
          b.platformName[(uint16_t)(k_ptr - k_start)] = '\0';
          i_binds.push_back(b);
          k_start = k_ptr + 1;
          k_size = 0;
        }
        ++k_ptr;
      }

      if ((uint16_t)(k_ptr - k_start)) {
        b.actionID = i;
        hcrt::strncpy(b.platformName, (uint16_t)(k_ptr - k_start), k_start);
        b.platformName[(uint16_t)(k_ptr - k_start)] = '\0';
        i_binds.push_back(b);
      }
    }

    hin::setupInputBindings(0, i_binds.data(), (uint32_t)i_binds.size());
  }
  virtual void postSystemAssetLoad() {
    // TODO: Grab any required assets here.

    // Initialise any global systems here.
    initialiseSpriteRenderer();
    collisionworld::initialiseWorld();

//
#if HART_DEBUG_INFO
    hEngine::addDebugMenu("Debug Options", []() {
      if (ImGui::Begin("Game Debug options", nullptr, 0)) {
        ImGui::Checkbox("Render level grid", &debugRenderTileGrid);
        // Add more as needed.
      }
      ImGui::End();
    });
#endif
  }
  virtual void taskGraphSetup(htasks::Graph* frameGraph) {
    // TODO: Inject game tasks into the frameGraph. Allows use to arrange game logic around engine flow
    htasks::TaskHandle res_update_task = frameGraph->findTaskByName("hresmgr::update");
  }
  virtual void preTick(htasks::Graph* frameGraph) {
    // work should be injected into the task graph. This should do nothing (or almost nothing).
  }
  virtual void tick(float delta) {
    // Do game tick
    ///* Input test
    hin::ButtonState btn;
    hin::AxisState   axis;
    btn = hin::getButtonState(InputAction_Up, 0);
    if (btn.flags) hdbprintf("Up (%x)\n", btn.flags);
    btn = hin::getButtonState(InputAction_Down, 0);
    if (btn.flags) hdbprintf("Down (%x)\n", btn.flags);
    btn = hin::getButtonState(InputAction_Left, 0);
    if (btn.flags) hdbprintf("Left (%x)\n", btn.flags);
    btn = hin::getButtonState(InputAction_Right, 0);
    if (btn.flags) hdbprintf("Right (%x)\n", btn.flags);
    btn = hin::getButtonState(InputAction_Select, 0);
    if (btn.flags) hdbprintf("Select (%x)\n", btn.flags);
    btn = hin::getButtonState(InputAction_Cancel, 0);
    if (btn.flags) hdbprintf("Cancel (%x)\n", btn.flags);
    axis = hin::getAxisState(InputAction_UpDownAxis, 0);
    if (axis.axisValue > 1500 || axis.axisValue < -1500) hdbprintf("Up<->Down (%d)\n", axis.axisValue);
    axis = hin::getAxisState(InputAction_LeftRightAxis, 0);
    if (axis.axisValue > 1500 || axis.axisValue < -1500) hdbprintf("Left<->Right (%d)\n", axis.axisValue);
    //*/
    updateAllLevelAnimations();
  }
  virtual void postTick() {}
  virtual void render() {
    // TODO: correct window size
    hMat44 proj, view;
    proj = hMat44::orthographic(0.0f, (float)viewWidth, (float)viewHeight, 0.0f, -1.0f, 1.0f);
    view = hMat44::identity();
#if HART_DEBUG_INFO
    hEngine::setDebugMatrices(view, proj);
#endif
    renderSprites((uint32_t)viewWidth, (uint32_t)viewHeight);

#if HART_DEBUG_INFO
    if (debugRenderTileGrid) {
      // draw an 8x8 grid (Flashbacks tile size)
      for (uint32_t x = 0; x < viewWidth; x += 8) {
        hrnd::debug::addLine(hVec3((float)x, 0.f, 0.f), hVec3((float)x, (float)viewHeight, 0.f), 0xFFFF0000);
      }
      for (uint32_t y = 0; y < viewHeight; y += 8) {
        hrnd::debug::addLine(hVec3(0.f, (float)y, 0.f), hVec3((float)viewWidth, (float)y, 0.f), 0xFFFF0000);
      }
      hrnd::debug::addQuad(hVec3(8 * 100, (float)viewHeight - (8 * 5), 0.f), hVec3(8 * 102, (float)viewHeight, 0.f),
                           0xFFFF00FF);
    }
#endif
  }

  GameState mainState;
};

int main(int argc, char* argv[]) {
  Game m;
  hEngine::run(argc, argv, &m);

  return 0;
}
