/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/hart.h"
#include "sprite_renderer.h"
#include "fbs/player_generated.h"
#include "fbs/test_generated.h"

class Player : public hety::Component {
    HART_COMPONENT_OBJECT_TYPE(HART_MAKE_FOURCC('p','l','y','r'), resource::Player)
public:
    uint32_t exp;
    uint8_t level;
    bool someFlag;
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
    HART_COMPONENT_OBJECT_TYPE(HART_MAKE_FOURCC('t','e','s','t'), resource::Test)
public:
    uint32_t exp;
    uint8_t level;
    uint32_t someTestValue;
};

HART_COMPONENT_OBJECT_TYPE_DECL(Test);

bool Test::deserialiseObject(MarshallType const*, hobjfact::SerialiseParams const&) {
    return true;
}
bool Test::deserialiseComponent(MarshallType const* overrides, MarshallType const* base) {
    exp = overrides->exp();
    level = overrides->level();
    if (overrides->someTestValue()) someTestValue = overrides->someTestValue()->val(); else if (base->someTestValue()) someTestValue = base->someTestValue()->val();
    return true;
}

// Prototype for functions registering classes. Saves having to include the header files here
void registierSpriteObject();
void registierLevelObject();

// save the includes. also only temp/testing.
void updateAllLevelAnimations();

class Game : public hart::engine::GameInterface {
    virtual void postObjectFactoryRegister() {
        //TODO: Register any game class objects here.
        hobjfact::objectFactoryRegister(Player::getObjectDefinition(), nullptr);
        hobjfact::objectFactoryRegister(Test::getObjectDefinition(), nullptr);
        registierSpriteObject();
        registierLevelObject();
    }
    virtual void postSystemAssetLoad() {
        //TODO: Grab any required assets here.

        // Initialise any global systems here.
        initialiseSpriteRenderer();
    }
    virtual void taskGraphSetup(htasks::Graph* frameGraph) {
        //TODO: Inject game tasks into the frameGraph. Allows use to arrange game logic around engine flow
        htasks::TaskHandle res_update_task = frameGraph->findTaskByName("hresmgr::update");
    }
    virtual void preTick(htasks::Graph* frameGraph) {
        // work should be injected into the task graph. This should do nothing (or almost nothing).
    }
    virtual void tick(float delta) {
        // TODO: do game tick
        updateAllLevelAnimations();
    }
    virtual void postTick() {

    }
    virtual void render() {
        // TODO: correct window size
        renderSprites(1280, 720);
    }
};

int main(int argc, char* argv[]) {
    Game m;
    hEngine::run(argc, argv, &m);

    return 0;
}