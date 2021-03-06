/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
// TODO: Define the interface to start and run the engine. Remove this pad/input crud
#pragma once

#include "hart/config.h"
#include "hart/base/std.h"
#include "hart/core/taskgraph.h"
#include "hart/base/matrix.h"
#include "imgui/imgui.h"
#include <SDL2/SDL_events.h>

#define ENTRY_WINDOW_FLAG_NONE UINT32_C(0x00000000)
#define ENTRY_WINDOW_FLAG_ASPECT_RATIO UINT32_C(0x00000001)
#define ENTRY_WINDOW_FLAG_FRAME UINT32_C(0x00000002)

namespace hart {
namespace engine {

enum class
  Event : uint32_t {
    Quit,           /**< User-requested quit */
    AppTerminating, /**< The application is being terminated by the OS. Called on iOS in applicationWillTerminate().
                       Called on Android in onDestroy()*/
    AppLowmemory,   /**< The application is low on memory, free memory if possible. Called on iOS in
                       applicationDidReceiveMemoryWarning(). Called on Android in onLowMemory()*/
    AppWillenterbackground,   /**< The application is about to enter the background. Called on iOS in
                                 applicationWillResignActive(). Called on Android in onPause() */
    AppDidenterbackground,    /**< The application did enter the background and may not get CPU for some time. Called on
                                 iOS in applicationDidEnterBackground(). Called on Android in onPause() */
    AppWillenterforeground,   /**< The application is about to enter the foreground. Called on iOS in
                                 applicationWillEnterForeground(). Called on Android in onResume() */
    AppDidenterforeground,    /**< The application is now interactive. Called on iOS in applicationDidBecomeActive().
                                 Called on Android in onResume() */
    WindowEvent,              /**< Window state change */
    SyswmEvent,               /**< System specific event */
    KeyDown,                  /**< Key pressed */
    KeyUp,                    /**< Key released */
    TextEditing,              /**< Keyboard text editing (composition) */
    TextInput,                /**< Keyboard text input */
    MouseMotion,              /**< Mouse moved */
    MouseButtondown,          /**< Mouse button pressed */
    MouseButtonup,            /**< Mouse button released */
    MouseWheel,               /**< Mouse wheel motion */
    JoyAxisMotion,            /**< Joystick axis motion */
    JoyBallMotion,            /**< Joystick trackball motion */
    JoyHatMotion,             /**< Joystick hat position change */
    JoyButtonDown,            /**< Joystick button pressed */
    JoyButtonUp,              /**< Joystick button released */
    JoyDeviceAdded,           /**< A new joystick has been inserted into the system */
    JoyDeviceRemoved,         /**< An opened joystick has been removed */
    ControllerAxisMotion,     /**< Game controller axis motion */
    ControllerButtonDown,     /**< Game controller button pressed */
    ControllerButtonUp,       /**< Game controller button released */
    ControllerDeviceAdded,    /**< A new Game controller has been inserted into the system */
    ControllerDeviceRemoved,  /**< An opened Game controller has been removed */
    ControllerDeviceRemapped, /**< The controller mapping was updated */
    FingerDown,
    FingerUp,
    FingerMotion,
    DollarGesture,
    DollarRecord,
    MultiGesture,
    ClipboardUpdate,    /**< The clipboard changed */
    Dropfile,           /**< The system requests a file open */
    RenderTargetsReset, /**< The render targets have been reset */
    UserEvent,

    Max
  };

// SDL_Events covers us for the time being. TODO: abstract it
typedef SDL_Event EventData;
typedef hstd::function<void(Event, EventData const*)> EventHandler;
struct EventHandle {
  Event    event;
  uint32_t loc;
};
typedef hstd::function<void()> DebugMenuCallback;
typedef uint32_t               DebugMenuHandle;

class GameInterface {
public:
  // Called after engine object classes have be registered
  virtual void postObjectFactoryRegister() = 0;
  // Called after main system assets are loaded.
  virtual void postSystemAssetLoad() = 0;
  // Called to initialise the frame task graph with any game jobs.
  virtual void taskGraphSetup(htasks::Graph* frameGraph) = 0;
  // Called to allow work loads in the task graph to be setup up before the game logic update
  virtual void preTick(htasks::Graph* frameGraph) = 0;
  // Called every game logic update. Runs in parallel to frameGraph
  virtual void tick(float delta) = 0;
  // Called after game logic update. Runs in parallel to frameGraph
  virtual void postTick() = 0;
  // Called every frame to submit draw calls.
  virtual void render() = 0;
};

int32_t run(int argc, char* argv[], GameInterface* game);

EventHandle addEventHandler(Event sysEventID, EventHandler handler);
void removeEventHandler(EventHandle handle);

#if HART_DEBUG_INFO
DebugMenuHandle addDebugMenu(char const* name, DebugMenuCallback debug_menu);
void removeDebugMenu(DebugMenuHandle handle);
void setDebugMatrices(hMat44 const& view, hMat44 const& proj);
#endif
}
}
namespace hEngine = hart::engine;