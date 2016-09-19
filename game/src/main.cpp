/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/core/engine.h"

class Game : public hart::engine::GameInterface {
    virtual void postObjectFactoryRegister() {
        //TODO: Register any game class objects here.
    }
    virtual void postSystemAssetLoad() {
        //TODO: Grab any required assets here.
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
    }
    virtual void postTick() {

    }
    virtual void render() {

    }
};

int main(int argc, char* argv[]) {
    Game m;
    hEngine::run(argc, argv, &m);

    return 0;
}