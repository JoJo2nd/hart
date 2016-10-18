/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/base/time.h"
#include <windows.h>

namespace hart {
namespace time {

static int64_t time;
static int64_t lastTime;
static float tickMS;
static float tickS;
static int64_t startTime;
static int64_t freq;
static int64_t freqMicro;

GameTick tickInfo;

static uint64_t getTicks() {
    uint64_t v;
    QueryPerformanceCounter( (LARGE_INTEGER*)(&v) );
    return v;
}

float elapsedSec() {
    return float((time - startTime)/freq) / 1000.0f;
}

uint64_t elapsedMS() {
    return (time - startTime)/freq;
}

float deltaMS() {
    return tickMS;
}

float deltaSec() {
    return tickS;
}

uint32_t hours() {
    return uint32_t(( elapsedSec() / 60.f ) / 60.f );
}

uint32_t mins() {
    return uint32_t((elapsedSec() / 60.f) - (hours() * 60.f));
}

uint32_t secs() {
    return uint32_t(elapsedSec() - (mins() * 60.f));
}

void initialise() {
    time = 0;
    lastTime = 0;
    tickS = 0.0f;
    tickMS = 0;

    QueryPerformanceFrequency( (LARGE_INTEGER*)( &freq ) );
    freq       /= 1000; // to millisecond converter

    QueryPerformanceCounter( (LARGE_INTEGER*)( &startTime ) );
    time = startTime;
    lastTime = time;
}

void update() {
    if ( freq != 0 ) {
        lastTime = time;
        time = getTicks();
        tickMS = (float)(time - lastTime)/float(freq);
        tickS = tickMS / 1000.0f;
    }
}

Timer::Timer() {
    reset();
}

void Timer::reset() {
    begin = getTicks();
    pauseStack = 0;
    lastPause = begin;
    pauseTotal = 0;
}

void Timer::setPause(bool val) {
    if (val) {
        ++pauseStack;
        if (pauseStack == 1) {
            lastPause = getTicks();
        }
    } else if (!val && pauseStack > 0) {
        pauseTotal += getTicks() - lastPause;
    }
}

uint64_t Timer::elaspedPause() const {
    uint64_t current = getTicks() - lastPause;
    return getPaused() ? pauseTotal + current : pauseTotal;
}

float Timer::elapsedSec() const {
    return elapsedMS() / 1000.f;
}

float Timer::elapsedMS() const {
    return float((getTicks() - begin) - elaspedPause()) / float(freq);
}

}
}
