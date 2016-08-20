/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
//NOTE: alot of this crap I don't need to worry about yet. First goal is to get a SDL2 window
// running with bgfx flipping buffers. SDL events should be handled as before (i.e. table to dispatch to listeners)
// then systems that care (e.g. input & controllers) can handle events as needed.
#pragma once

#include "hart/config.h"
#include "hart/core/engine.h"
#include "hart/core/resourcemanager.h"
#include "hart/base/filesystem.h"

// object factory classes
#include "hart/render/shader.h"

#if (HART_PLATFORM == HART_PLATFORM_WINDOWS)
#   define SDL_MAIN_HANDLED
#endif // BX_PLATFORM_WINDOWS

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <bgfx/bgfxplatform.h>
#if defined(None) // X11 defines this...
#   undef None
#endif // defined(None)

#include <stdio.h>
#include <vector>


namespace hart {
namespace engine {

    ///
    static void* sdlNativeWindowHandle(SDL_Window* _window)
    {
        SDL_SysWMinfo wmi;
        SDL_VERSION(&wmi.version);
        if (!SDL_GetWindowWMInfo(_window, &wmi) )
        {
            return NULL;
        }

#   if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
        return (void*)wmi.info.x11.window;
#   elif BX_PLATFORM_OSX
        return wmi.info.cocoa.window;
#   elif BX_PLATFORM_WINDOWS
        return wmi.info.win.window;
#   elif BX_PLATFORM_STEAMLINK
        return wmi.info.vivante.window;
#   endif // BX_PLATFORM_
    }

    struct Context
    {
        static Event SDL2SystemEvent[SDL_LASTEVENT];
        static uint32_t systemEvent2SDL[Event::Max];

        Context()
            : m_width(HART_DEFAULT_WND_WIDTH)
            , m_height(HART_DEFAULT_WND_HEIGHT)
            , m_aspectRatio(16.0f/9.0f)
            , m_mx(0)
            , m_my(0)
            , m_mz(0)
            , m_mouseLock(false)
            , m_fullscreen(false)
        {
            SDL2SystemEvent[SDL_QUIT] = Event::Quit;
            SDL2SystemEvent[SDL_APP_TERMINATING] = Event::AppTerminating;
            SDL2SystemEvent[SDL_APP_LOWMEMORY] = Event::AppLowmemory;
            SDL2SystemEvent[SDL_APP_WILLENTERBACKGROUND] = Event::AppWillenterbackground;
            SDL2SystemEvent[SDL_APP_DIDENTERBACKGROUND] = Event::AppDidenterbackground;
            SDL2SystemEvent[SDL_APP_WILLENTERFOREGROUND] = Event::AppWillenterforeground;
            SDL2SystemEvent[SDL_APP_DIDENTERFOREGROUND] = Event::AppDidenterforeground;
            SDL2SystemEvent[SDL_WINDOWEVENT] = Event::WindowEvent;
            SDL2SystemEvent[SDL_SYSWMEVENT] = Event::SyswmEvent;
            SDL2SystemEvent[SDL_KEYDOWN] = Event::KeyDown;
            SDL2SystemEvent[SDL_KEYUP] = Event::KeyUp;
            SDL2SystemEvent[SDL_TEXTEDITING] = Event::TextEditing;
            SDL2SystemEvent[SDL_TEXTINPUT] = Event::TextInput;
            SDL2SystemEvent[SDL_MOUSEMOTION] = Event::MouseMotion;
            SDL2SystemEvent[SDL_MOUSEBUTTONDOWN] = Event::MouseButtondown;
            SDL2SystemEvent[SDL_MOUSEBUTTONUP] = Event::MouseButtonup;
            SDL2SystemEvent[SDL_MOUSEWHEEL] = Event::MouseWheel;
            SDL2SystemEvent[SDL_JOYAXISMOTION] = Event::JoyAxisMotion;
            SDL2SystemEvent[SDL_JOYBALLMOTION] = Event::JoyBallMotion;
            SDL2SystemEvent[SDL_JOYHATMOTION] = Event::JoyHatMotion;
            SDL2SystemEvent[SDL_JOYBUTTONDOWN] = Event::JoyButtonDown;
            SDL2SystemEvent[SDL_JOYBUTTONUP] = Event::JoyButtonUp;
            SDL2SystemEvent[SDL_JOYDEVICEADDED] = Event::JoyDeviceAdded;
            SDL2SystemEvent[SDL_JOYDEVICEREMOVED] = Event::JoyDeviceRemoved;
            SDL2SystemEvent[SDL_CONTROLLERAXISMOTION] = Event::ControllerAxisMotion;
            SDL2SystemEvent[SDL_CONTROLLERBUTTONDOWN] = Event::ControllerButtonDown;
            SDL2SystemEvent[SDL_CONTROLLERBUTTONUP] = Event::ControllerButtonUp;
            SDL2SystemEvent[SDL_CONTROLLERDEVICEADDED] = Event::ControllerDeviceAdded;
            SDL2SystemEvent[SDL_CONTROLLERDEVICEREMOVED] = Event::ControllerDeviceRemoved;
            SDL2SystemEvent[SDL_CONTROLLERDEVICEREMAPPED] = Event::ControllerDeviceRemapped;
            SDL2SystemEvent[SDL_FINGERDOWN] = Event::FingerDown;
            SDL2SystemEvent[SDL_FINGERUP] = Event::FingerUp;
            SDL2SystemEvent[SDL_FINGERMOTION] = Event::FingerMotion;
            SDL2SystemEvent[SDL_DOLLARGESTURE] = Event::DollarGesture;
            SDL2SystemEvent[SDL_DOLLARRECORD] = Event::DollarRecord;
            SDL2SystemEvent[SDL_MULTIGESTURE] = Event::MultiGesture;
            SDL2SystemEvent[SDL_CLIPBOARDUPDATE] = Event::ClipboardUpdate;
            SDL2SystemEvent[SDL_DROPFILE] = Event::Dropfile;
            SDL2SystemEvent[SDL_RENDER_TARGETS_RESET] = Event::RenderTargetsReset;
            SDL2SystemEvent[SDL_USEREVENT] = Event::UserEvent;

            systemEvent2SDL[(uint32_t)Event::Quit] = SDL_QUIT;
            systemEvent2SDL[(uint32_t)Event::AppTerminating] = SDL_APP_TERMINATING;
            systemEvent2SDL[(uint32_t)Event::AppLowmemory] = SDL_APP_LOWMEMORY;
            systemEvent2SDL[(uint32_t)Event::AppWillenterbackground] = SDL_APP_WILLENTERBACKGROUND;
            systemEvent2SDL[(uint32_t)Event::AppDidenterbackground] = SDL_APP_DIDENTERBACKGROUND;
            systemEvent2SDL[(uint32_t)Event::AppWillenterforeground] = SDL_APP_WILLENTERFOREGROUND;
            systemEvent2SDL[(uint32_t)Event::AppDidenterforeground] = SDL_APP_DIDENTERFOREGROUND;
            systemEvent2SDL[(uint32_t)Event::WindowEvent] = SDL_WINDOWEVENT;
            systemEvent2SDL[(uint32_t)Event::SyswmEvent] = SDL_SYSWMEVENT;
            systemEvent2SDL[(uint32_t)Event::KeyDown] = SDL_KEYDOWN;
            systemEvent2SDL[(uint32_t)Event::KeyUp] = SDL_KEYUP;
            systemEvent2SDL[(uint32_t)Event::TextEditing] = SDL_TEXTEDITING;
            systemEvent2SDL[(uint32_t)Event::TextInput] = SDL_TEXTINPUT;
            systemEvent2SDL[(uint32_t)Event::MouseMotion] = SDL_MOUSEMOTION;
            systemEvent2SDL[(uint32_t)Event::MouseButtondown] = SDL_MOUSEBUTTONDOWN;
            systemEvent2SDL[(uint32_t)Event::MouseButtonup] = SDL_MOUSEBUTTONUP;
            systemEvent2SDL[(uint32_t)Event::MouseWheel] = SDL_MOUSEWHEEL;
            systemEvent2SDL[(uint32_t)Event::JoyAxisMotion] = SDL_JOYAXISMOTION;
            systemEvent2SDL[(uint32_t)Event::JoyBallMotion] = SDL_JOYBALLMOTION;
            systemEvent2SDL[(uint32_t)Event::JoyHatMotion] = SDL_JOYHATMOTION;
            systemEvent2SDL[(uint32_t)Event::JoyButtonDown] = SDL_JOYBUTTONDOWN;
            systemEvent2SDL[(uint32_t)Event::JoyButtonUp] = SDL_JOYBUTTONUP;
            systemEvent2SDL[(uint32_t)Event::JoyDeviceAdded] = SDL_JOYDEVICEADDED;
            systemEvent2SDL[(uint32_t)Event::JoyDeviceRemoved] = SDL_JOYDEVICEREMOVED;
            systemEvent2SDL[(uint32_t)Event::ControllerAxisMotion] = SDL_CONTROLLERAXISMOTION;
            systemEvent2SDL[(uint32_t)Event::ControllerButtonDown] = SDL_CONTROLLERBUTTONDOWN;
            systemEvent2SDL[(uint32_t)Event::ControllerButtonUp] = SDL_CONTROLLERBUTTONUP;
            systemEvent2SDL[(uint32_t)Event::ControllerDeviceAdded] = SDL_CONTROLLERDEVICEADDED;
            systemEvent2SDL[(uint32_t)Event::ControllerDeviceRemoved] = SDL_CONTROLLERDEVICEREMOVED;
            systemEvent2SDL[(uint32_t)Event::ControllerDeviceRemapped] = SDL_CONTROLLERDEVICEREMAPPED;
            systemEvent2SDL[(uint32_t)Event::FingerDown] = SDL_FINGERDOWN;
            systemEvent2SDL[(uint32_t)Event::FingerUp] = SDL_FINGERUP;
            systemEvent2SDL[(uint32_t)Event::FingerMotion] = SDL_FINGERMOTION;
            systemEvent2SDL[(uint32_t)Event::DollarGesture] = SDL_DOLLARGESTURE;
            systemEvent2SDL[(uint32_t)Event::DollarRecord] = SDL_DOLLARRECORD;
            systemEvent2SDL[(uint32_t)Event::MultiGesture] = SDL_MULTIGESTURE;
            systemEvent2SDL[(uint32_t)Event::ClipboardUpdate] = SDL_CLIPBOARDUPDATE;
            systemEvent2SDL[(uint32_t)Event::Dropfile] = SDL_DROPFILE;
            systemEvent2SDL[(uint32_t)Event::RenderTargetsReset] = SDL_RENDER_TARGETS_RESET;
            systemEvent2SDL[(uint32_t)Event::UserEvent] = SDL_USEREVENT;
        }

        EventHandle addEventHandler(Event sysEventID, EventHandler handler) {
            EventHandle hdl = {sysEventID, (uint32_t)eventHandlers[(uint32_t)sysEventID].size()};
            eventHandlers[(uint32_t)sysEventID].push_back(handler);
            return hdl;
        }

        void removeEventHandler(EventHandle handle) {
            eventHandlers[(uint32_t)handle.event].erase(eventHandlers[(uint32_t)handle.event].cbegin()+handle.loc);
        }

        int run(int _argc, char** _argv) {
            SDL_Init(0
                | SDL_INIT_GAMECONTROLLER
                );

            m_window = SDL_CreateWindow("hart"
                            , SDL_WINDOWPOS_UNDEFINED
                            , SDL_WINDOWPOS_UNDEFINED
                            , m_width
                            , m_height
                            , SDL_WINDOW_SHOWN
                            | SDL_WINDOW_RESIZABLE
                            );

            m_flags = ENTRY_WINDOW_FLAG_ASPECT_RATIO
                | ENTRY_WINDOW_FLAG_FRAME;

            hfs::initialise_filesystem();
            hresmgr::initialise();

            //s_userEventStart = SDL_RegisterEvents(7);

            bgfx::sdlSetWindow(m_window);
            bgfx::renderFrame();

            //m_thread.init(MainThreadEntry::threadFunc, &m_mte);

            // Force window resolution...
            //WindowHandle defaultWindow = { 0 };
            //setWindowSize(defaultWindow, m_width, m_height, true);
            SDL_SetWindowSize(m_window, m_width, m_height);

            /*
            bx::CrtFileReader reader;
            if (bx::open(&reader, "gamecontrollerdb.txt") )
            {
                bx::AllocatorI* allocator = getAllocator();
                uint32_t size = (uint32_t)bx::getSize(&reader);
                void* data = BX_ALLOC(allocator, size);
                bx::read(&reader, data, size);
                bx::close(&reader);

                SDL_GameControllerAddMapping( (char*)data);

                BX_FREE(allocator, data);
            }
            */
            // Init engine
            hobjfact::objectFactoryRegistar(hrnd::Shader::getObjectDefinition(), nullptr);

            bool exit = false;
            SDL_Event event;
            while (!exit)
            {
                bgfx::renderFrame();

                while (SDL_PollEvent(&event) )
                {
                    switch (event.type)
                    {
                    case SDL_QUIT:
                        //m_eventQueue.postExitEvent();
                        exit = true;
                        break;
                    default:
                        for (const auto& i : eventHandlers[(uint32_t)SDL2SystemEvent[event.type]]) {
                            i(SDL2SystemEvent[event.type], &event);
                        }
                        break;
/*
                    case SDL_MOUSEMOTION:
                        {
                            const SDL_MouseMotionEvent& mev = event.motion;
                            m_mx = mev.x;
                            m_my = mev.y;

                            WindowHandle handle = findHandle(mev.windowID);
                            if (isValid(handle) )
                            {
                                m_eventQueue.postMouseEvent(handle, m_mx, m_my, m_mz);
                            }
                        }
                        break;

                    case SDL_MOUSEBUTTONDOWN:
                    case SDL_MOUSEBUTTONUP:
                        {
                            const SDL_MouseButtonEvent& mev = event.button;
                            WindowHandle handle = findHandle(mev.windowID);
                            if (isValid(handle) )
                            {
                                MouseButton::Enum button;
                                switch (mev.button)
                                {
                                default:
                                case SDL_BUTTON_LEFT:   button = MouseButton::Left;   break;
                                case SDL_BUTTON_MIDDLE: button = MouseButton::Middle; break;
                                case SDL_BUTTON_RIGHT:  button = MouseButton::Right;  break;
                                }

                                m_eventQueue.postMouseEvent(handle
                                    , mev.x
                                    , mev.y
                                    , 0
                                    , button
                                    , mev.type == SDL_MOUSEBUTTONDOWN
                                    );
                            }
                        }
                        break;

                    case SDL_MOUSEWHEEL:
                        {
                            const SDL_MouseWheelEvent& mev = event.wheel;
                            m_mz += mev.y;

                            WindowHandle handle = findHandle(mev.windowID);
                            if (isValid(handle) )
                            {
                                m_eventQueue.postMouseEvent(handle, m_mx, m_my, m_mz);
                            }
                        }
                        break;

                    case SDL_TEXTINPUT:
                        {
                            const SDL_TextInputEvent& tev = event.text;
                            WindowHandle handle = findHandle(tev.windowID);
                            if (isValid(handle) )
                            {
                                m_eventQueue.postCharEvent(handle, 1, (const uint8_t*)tev.text);
                            }
                        }
                        break;

                    case SDL_KEYDOWN:
                        {
                            const SDL_KeyboardEvent& kev = event.key;
                            WindowHandle handle = findHandle(kev.windowID);
                            if (isValid(handle) )
                            {
                                uint8_t modifiers = translateKeyModifiers(kev.keysym.mod);
                                Key::Enum key = translateKey(kev.keysym.scancode);

                                // TODO: These keys are not captured by SDL_TEXTINPUT. Should be probably handled by SDL_TEXTEDITING. This is a workaround for now.
                                if (key == 1) // Escape
                                {
                                    uint8_t pressedChar[4];
                                    pressedChar[0] = 0x1b;
                                    m_eventQueue.postCharEvent(handle, 1, pressedChar);
                                }
                                else if (key == 2) // Enter
                                {
                                    uint8_t pressedChar[4];
                                    pressedChar[0] = 0x0d;
                                    m_eventQueue.postCharEvent(handle, 1, pressedChar);
                                }
                                else if (key == 5) // Backspace
                                {
                                    uint8_t pressedChar[4];
                                    pressedChar[0] = 0x08;
                                    m_eventQueue.postCharEvent(handle, 1, pressedChar);
                                }
                                else
                                {
                                    m_eventQueue.postKeyEvent(handle, key, modifiers, kev.state == SDL_PRESSED);
                                }
                            }
                        }
                        break;

                    case SDL_KEYUP:
                        {
                            const SDL_KeyboardEvent& kev = event.key;
                            WindowHandle handle = findHandle(kev.windowID);
                            if (isValid(handle) )
                            {
                                uint8_t modifiers = translateKeyModifiers(kev.keysym.mod);
                                Key::Enum key = translateKey(kev.keysym.scancode);
                                m_eventQueue.postKeyEvent(handle, key, modifiers, kev.state == SDL_PRESSED);
                            }
                        }
                        break;

                    case SDL_WINDOWEVENT:
                        {
                            const SDL_WindowEvent& wev = event.window;
                            switch (wev.event)
                            {
                            case SDL_WINDOWEVENT_RESIZED:
                            case SDL_WINDOWEVENT_SIZE_CHANGED:
                                {
                                    WindowHandle handle = findHandle(wev.windowID);
                                    setWindowSize(handle, wev.data1, wev.data2);
                                }
                                break;

                            case SDL_WINDOWEVENT_SHOWN:
                            case SDL_WINDOWEVENT_HIDDEN:
                            case SDL_WINDOWEVENT_EXPOSED:
                            case SDL_WINDOWEVENT_MOVED:
                            case SDL_WINDOWEVENT_MINIMIZED:
                            case SDL_WINDOWEVENT_MAXIMIZED:
                            case SDL_WINDOWEVENT_RESTORED:
                            case SDL_WINDOWEVENT_ENTER:
                            case SDL_WINDOWEVENT_LEAVE:
                            case SDL_WINDOWEVENT_FOCUS_GAINED:
                            case SDL_WINDOWEVENT_FOCUS_LOST:
                                break;

                            case SDL_WINDOWEVENT_CLOSE:
                                {
                                    WindowHandle handle = findHandle(wev.windowID);
                                    if (0 == handle.idx)
                                    {
                                        m_eventQueue.postExitEvent();
                                        exit = true;
                                    }
                                }
                                break;
                            }
                        }
                        break;

                    case SDL_JOYAXISMOTION:
                        {
                            const SDL_JoyAxisEvent& jev = event.jaxis;
                            GamepadHandle handle = findGamepad(jev.which);
                            if (isValid(handle) )
                            {
                                GamepadAxis::Enum axis = translateGamepadAxis(jev.axis);
                                m_gamepad[handle.idx].update(m_eventQueue, defaultWindow, handle, axis, jev.value);
                            }
                        }
                        break;

                    case SDL_CONTROLLERAXISMOTION:
                        {
                            const SDL_ControllerAxisEvent& aev = event.caxis;
                            GamepadHandle handle = findGamepad(aev.which);
                            if (isValid(handle) )
                            {
                                GamepadAxis::Enum axis = translateGamepadAxis(aev.axis);
                                m_gamepad[handle.idx].update(m_eventQueue, defaultWindow, handle, axis, aev.value);
                            }
                        }
                        break;

                    case SDL_JOYBUTTONDOWN:
                    case SDL_JOYBUTTONUP:
                        {
                            const SDL_JoyButtonEvent& bev = event.jbutton;
                            GamepadHandle handle = findGamepad(bev.which);

                            if (isValid(handle) )
                            {
                                Key::Enum key = translateGamepad(bev.button);
                                if (Key::Count != key)
                                {
                                    m_eventQueue.postKeyEvent(defaultWindow, key, 0, event.type == SDL_JOYBUTTONDOWN);
                                }
                            }
                        }
                        break;

                    case SDL_CONTROLLERBUTTONDOWN:
                    case SDL_CONTROLLERBUTTONUP:
                        {
                            const SDL_ControllerButtonEvent& bev = event.cbutton;
                            GamepadHandle handle = findGamepad(bev.which);
                            if (isValid(handle) )
                            {
                                Key::Enum key = translateGamepad(bev.button);
                                if (Key::Count != key)
                                {
                                    m_eventQueue.postKeyEvent(defaultWindow, key, 0, event.type == SDL_CONTROLLERBUTTONDOWN);
                                }
                            }
                        }
                        break;

                    case SDL_JOYDEVICEADDED:
                        {
                            GamepadHandle handle = { m_gamepadAlloc.alloc() };
                            if (isValid(handle) )
                            {
                                const SDL_JoyDeviceEvent& jev = event.jdevice;
                                m_gamepad[handle.idx].create(jev);
                                m_eventQueue.postGamepadEvent(defaultWindow, handle, true);
                            }
                        }
                        break;

                    case SDL_JOYDEVICEREMOVED:
                        {
                            const SDL_JoyDeviceEvent& jev = event.jdevice;
                            GamepadHandle handle = findGamepad(jev.which);
                            if (isValid(handle) )
                            {
                                m_gamepad[handle.idx].destroy();
                                m_gamepadAlloc.free(handle.idx);
                                m_eventQueue.postGamepadEvent(defaultWindow, handle, false);
                            }
                        }
                        break;

                    case SDL_CONTROLLERDEVICEADDED:
                        {
                            GamepadHandle handle = { m_gamepadAlloc.alloc() };
                            if (isValid(handle) )
                            {
                                const SDL_ControllerDeviceEvent& cev = event.cdevice;
                                m_gamepad[handle.idx].create(cev);
                                m_eventQueue.postGamepadEvent(defaultWindow, handle, true);
                            }
                        }
                        break;

                    case SDL_CONTROLLERDEVICEREMAPPED:
                        {

                        }
                        break;

                    case SDL_CONTROLLERDEVICEREMOVED:
                        {
                            const SDL_ControllerDeviceEvent& cev = event.cdevice;
                            GamepadHandle handle = findGamepad(cev.which);
                            if (isValid(handle) )
                            {
                                m_gamepad[handle.idx].destroy();
                                m_gamepadAlloc.free(handle.idx);
                                m_eventQueue.postGamepadEvent(defaultWindow, handle, false);
                            }
                        }
                        break;

                    default:
                        {
                            const SDL_UserEvent& uev = event.user;
                            switch (uev.type - s_userEventStart)
                            {
                            case SDL_USER_WINDOW_CREATE:
                                {
                                    WindowHandle handle = getWindowHandle(uev);
                                    Msg* msg = (Msg*)uev.data2;

                                    m_window[handle.idx] = SDL_CreateWindow(msg->m_title.c_str()
                                                                , msg->m_x
                                                                , msg->m_y
                                                                , msg->m_width
                                                                , msg->m_height
                                                                , SDL_WINDOW_SHOWN
                                                                | SDL_WINDOW_RESIZABLE
                                                                );

                                    m_flags[handle.idx] = msg->m_flags;

                                    void* nwh = sdlNativeWindowHandle(m_window[handle.idx]);
                                    if (NULL != nwh)
                                    {
                                        m_eventQueue.postWindowEvent(handle, nwh);
                                        m_eventQueue.postSizeEvent(handle, msg->m_width, msg->m_height);
                                    }

                                    delete msg;
                                }
                                break;

                            case SDL_USER_WINDOW_DESTROY:
                                {
                                    WindowHandle handle = getWindowHandle(uev);
                                    if (isValid(handle) )
                                    {
                                        m_eventQueue.postWindowEvent(handle);
                                        SDL_DestroyWindow(m_window[handle.idx]);
                                        m_window[handle.idx] = NULL;
                                    }
                                }
                                break;

                            case SDL_USER_WINDOW_SET_TITLE:
                                {
                                    WindowHandle handle = getWindowHandle(uev);
                                    Msg* msg = (Msg*)uev.data2;
                                    if (isValid(handle) )
                                    {
                                        SDL_SetWindowTitle(m_window[handle.idx], msg->m_title.c_str() );
                                    }
                                    delete msg;
                                }
                                break;

                            case SDL_USER_WINDOW_SET_POS:
                                {
                                    WindowHandle handle = getWindowHandle(uev);
                                    Msg* msg = (Msg*)uev.data2;
                                    SDL_SetWindowPosition(m_window[handle.idx], msg->m_x, msg->m_y);
                                    delete msg;
                                }
                                break;

                            case SDL_USER_WINDOW_SET_SIZE:
                                {
                                    WindowHandle handle = getWindowHandle(uev);
                                    Msg* msg = (Msg*)uev.data2;
                                    if (isValid(handle) )
                                    {
                                        setWindowSize(handle, msg->m_width, msg->m_height);
                                    }
                                    delete msg;
                                }
                                break;

                            case SDL_USER_WINDOW_TOGGLE_FRAME:
                                {
                                    WindowHandle handle = getWindowHandle(uev);
                                    if (isValid(handle) )
                                    {
                                        m_flags[handle.idx] ^= ENTRY_WINDOW_FLAG_FRAME;
                                        SDL_SetWindowBordered(m_window[handle.idx], (SDL_bool)!!(m_flags[handle.idx] & ENTRY_WINDOW_FLAG_FRAME) );
                                    }
                                }
                                break;

                            case SDL_USER_WINDOW_TOGGLE_FULL_SCREEN:
                                {
                                    WindowHandle handle = getWindowHandle(uev);
                                    m_fullscreen = !m_fullscreen;
                                    SDL_SetWindowFullscreen(m_window[handle.idx], m_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                                }
                                break;

                            case SDL_USER_WINDOW_MOUSE_LOCK:
                                {
                                    SDL_SetRelativeMouseMode(!!uev.code ? SDL_TRUE : SDL_FALSE);
                                }
                                break;

                            default:
                                break;
                            }
                        }
                        break;
*/
                    }
                }

                //DO Tick here?
            }

            while (bgfx::RenderFrame::NoContext != bgfx::renderFrame() ) {};

            // Engine shutdown

            SDL_DestroyWindow(m_window);
            SDL_Quit();

            return 0;
        }

        //Only support one window currently
        SDL_Window* m_window;
        uint32_t m_flags;

        uint32_t m_width;
        uint32_t m_height;
        float m_aspectRatio;

        int32_t m_mx;
        int32_t m_my;
        int32_t m_mz;
        bool m_mouseLock;
        bool m_fullscreen;

        std::vector<EventHandler> eventHandlers[Event::Max];
    };

    static Context s_ctx;

Event Context::SDL2SystemEvent[SDL_LASTEVENT];
uint32_t Context::systemEvent2SDL[Event::Max];

int32_t run(int argc, char* argv[]) {
    return s_ctx.run(argc, argv);
}

hart::engine::EventHandle addEventHandler(Event sysEventID,EventHandler handler) {
    return s_ctx.addEventHandler(sysEventID, handler);
}

void removeEventHandler(EventHandle handle) {
    s_ctx.removeEventHandler(handle);
}

}
} // namespace entry

