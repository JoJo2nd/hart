/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"

namespace hart {
namespace time {

struct GameTick {
  float   deltaMS;
  uint8_t interval30Hz;
  uint8_t interval15Hz;
  uint8_t interval10Hz;
  uint8_t interval5Hz;
};

extern GameTick tickInfo;

void initialise();
void update();

float    elapsedSec();
uint64_t elapsedMS();

float deltaMS();
float deltaSec();

uint32_t hours();
uint32_t mins();
uint32_t secs();


class Timer {
public:
  Timer();
  void reset();
  void setPause(bool val);
  bool  getPaused() const { return pauseStack != 0; }
  float elapsedSec() const;
  float elapsedMS() const;

private:
  uint64_t elaspedPause() const;

  uint64_t begin = 0;
  uint64_t lastPause = 0;
  uint64_t pauseTotal = 0;
  uint32_t pauseStack = 0;
};
}
}

namespace htime = hart::time;
