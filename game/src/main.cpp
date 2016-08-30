/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/core/engine.h"

class Game : public hart::engine::GameInterface {
    virtual void postSystemAssetLoad() {
        //TODO: Grab any required assets here.
    }
    virtual void taskGraphSetup(htasks::Graph* frameGraph) {
        //TODO: Inject game tasks into the frameGraph. Allows use to arrange game logic around engine flow
        htasks::TaskHandle res_update_task = frameGraph->findTaskByName("hresmgr::update");
    }
    virtual void tick(float delta, htasks::Graph* frameGraph) {
        // work should be injected into the task graph. This should do nothing (or almost nothing).
    }
};

int main(int argc, char* argv[]) {
    Game m;
    hEngine::run(argc, argv, &m);

    return 0;
}