/********************************************************************
    Written by James Moran and Luke Trow
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
//NOTE: alot of this crap I don't need to worry about yet. First goal is to get a SDL2 window
// running with bgfx flipping buffers. SDL events should be handled as before (i.e. table to dispatch to listeners)
// then systems that care (e.g. input & controllers) can handle events as needed.
#pragma once

#include "hart/config.h"
#include "hart/core/engine.h"
#include "hart/core/resourcemanager.h"
#include "hart/core/taskgraph.h"    
#include "hart/core/configoptions.h"
#include "hart/base/uuid.h"
#include "hart/base/filesystem.h"
#include "hart/base/util.h"
#include "hart/base/time.h"
#include "imgui.h"
#include "vectormath_aos.h"
#include "mat_aos.h"
#include "vec_aos.h"

// object factory classes
#include "hart/render/render.h"

#if (HART_PLATFORM == HART_PLATFORM_WINDOWS)
#   define SDL_MAIN_HANDLED
#endif // BX_PLATFORM_WINDOWS

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <bgfx/bgfx.h>
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

        Context() {
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

        static void imguiRenderStatic(ImDrawData* draw_data) {
            ImGuiIO& io = ImGui::GetIO();
            ((Context*)io.UserData)->imguiRender(draw_data);
        }

        void imguiRender(ImDrawData* draw_data) {
            const ImGuiIO& io = ImGui::GetIO();
            const float width  = io.DisplaySize.x;
            const float height = io.DisplaySize.y;

            {
                Vectormath::Aos::Matrix4 ortho;
                ortho = Vectormath::Aos::Matrix4::orthographic(0.0f, width, height, 0.0f, -1.0f, 1.0f);
                bgfx::setViewTransform(0, NULL, (float*)&ortho);
            }

    #if USE_ENTRY
            for (uint32_t ii = 1; ii < BX_COUNTOF(m_window); ++ii)
            {
                Window& window = m_window[ii];
                if (bgfx::isValid(window.m_fbh) )
                {
                    const uint8_t viewId = 0;
                    bgfx::setViewClear(viewId
                        , BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
                        , 0x303030ff
                        , 1.0f
                        , 0
                        );
                    bgfx::setViewFrameBuffer(viewId, window.m_fbh);
                    bgfx::setViewRect(viewId
                        , 0
                        , 0
                        , window.m_state.m_width
                        , window.m_state.m_height
                        );
                    float ortho[16];
                    bx::mtxOrtho(ortho
                        , 0.0f
                        , float(window.m_state.m_width)
                        , float(window.m_state.m_height)
                        , 0.0f
                        , -1.0f
                        , 1.0f
                        );
                    bgfx::setViewTransform(viewId
                        , NULL
                        , ortho
                        );
                }
            }
    #endif // USE_ENTRY

            // Render command lists
            for (int32_t ii = 0, num = draw_data->CmdListsCount; ii < num; ++ii)
            {
                bgfx::TransientVertexBuffer tvb;
                bgfx::TransientIndexBuffer tib;

                const ImDrawList* drawList = draw_data->CmdLists[ii];
                uint32_t numVertices = (uint32_t)drawList->VtxBuffer.size();
                uint32_t numIndices  = (uint32_t)drawList->IdxBuffer.size();

                if (!bgfx::checkAvailTransientVertexBuffer(numVertices, imgui.decl)
                ||  !bgfx::checkAvailTransientIndexBuffer(numIndices) )
                {
                    // not enough space in transient buffer just quit drawing the rest...
                    break;
                }

                bgfx::allocTransientVertexBuffer(&tvb, numVertices, imgui.decl);
                bgfx::allocTransientIndexBuffer(&tib, numIndices);

                ImDrawVert* verts = (ImDrawVert*)tvb.data;
                memcpy(verts, drawList->VtxBuffer.begin(), numVertices * sizeof(ImDrawVert) );

                ImDrawIdx* indices = (ImDrawIdx*)tib.data;
                memcpy(indices, drawList->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx) );

                uint32_t offset = 0;
                for (const ImDrawCmd* cmd = drawList->CmdBuffer.begin(), *cmdEnd = drawList->CmdBuffer.end(); cmd != cmdEnd; ++cmd)
                {
                    if (cmd->UserCallback)
                    {
                        cmd->UserCallback(drawList, cmd);
                    }
                    else if (0 != cmd->ElemCount)
                    {
                        uint64_t state = 0
                            | BGFX_STATE_RGB_WRITE
                            | BGFX_STATE_ALPHA_WRITE
                            | BGFX_STATE_MSAA
                            ;

                        bgfx::TextureHandle th = imgui.texture;
                        bgfx::ProgramHandle program = imgui.program;

                        if (NULL != cmd->TextureId)
                        {
                            hdbfatal("STUB: ImGui render texture callback");
                            /*
                            union { ImTextureID ptr; struct { bgfx::TextureHandle handle; uint8_t flags; uint8_t mip; } s; } texture = { cmd->TextureId };
                            state |= 0 != (IMGUI_FLAGS_ALPHA_BLEND & texture.s.flags)
                                ? BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
                                : BGFX_STATE_NONE
                                ;
                            th = texture.s.handle;
                            if (0 != texture.s.mip)
                            {
                                extern bgfx::ProgramHandle imguiGetImageProgram(uint8_t _mip);
                                program = imguiGetImageProgram(texture.s.mip);
                            }
                            */
                        }
                        else
                        {
                            state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
                        }

                        const uint16_t xx = uint16_t(hutil::tmax(cmd->ClipRect.x, 0.0f) );
                        const uint16_t yy = uint16_t(hutil::tmax(cmd->ClipRect.y, 0.0f) );
                        bgfx::setScissor(xx, yy
                                , uint16_t(hutil::tmin(cmd->ClipRect.z, 65535.0f)-xx)
                                , uint16_t(hutil::tmin(cmd->ClipRect.w, 65535.0f)-yy)
                                );

                        bgfx::setState(state);
                        bgfx::setTexture(0, imgui.textureUniform, th);
                        bgfx::setVertexBuffer(&tvb, 0, numVertices);
                        bgfx::setIndexBuffer(&tib, offset, cmd->ElemCount);
                        bgfx::submit(0, program);
                    }

                    offset += cmd->ElemCount;
                }
            }
        }

        int run(int _argc, char** _argv, GameInterface* game) {
            hprofile_startup();
            hprofile_namethread("Main Thread");

            hfs::initialise_filesystem();
            hfs::FileHandle fileHdl;
            hfs::FileOpHandle fop = hfs::openFile("/system.ini", hfs::Mode::Read, &fileHdl);
            if (hfs::fileOpWait(fop) != hfs::Error::Ok) {
                return -1;
            }
            hfs::FileStat stats;
            fop = hfs::fstatAsync(fileHdl, &stats);
            if (hfs::fileOpWait(fop) != hfs::Error::Ok) {
                return -1;
            }
            char* data = new char[stats.filesize];
            fop = hfs::freadAsync(fileHdl, data, stats.filesize, 0);
            if (hfs::fileOpWait(fop) != hfs::Error::Ok) {
                return -1;
            }
            hfs::closeFile(fileHdl);
            if (!hconfigopt::loadConfigOptions(data, stats.filesize)) {
                return -2;
            }
            delete data;
            data = nullptr;

            m_width = hconfigopt::getUint("renderer", "width", 854);
            m_height = hconfigopt::getUint("renderer", "height", 480);

            SDL_Init(0
                | SDL_INIT_GAMECONTROLLER
                );

            m_window = SDL_CreateWindow(hconfigopt::getStr("window", "title", "---")
                            , SDL_WINDOWPOS_UNDEFINED
                            , SDL_WINDOWPOS_UNDEFINED
                            , m_width
                            , m_height
                            , SDL_WINDOW_SHOWN
                            | SDL_WINDOW_RESIZABLE
                            );

            m_flags = ENTRY_WINDOW_FLAG_ASPECT_RATIO
                | ENTRY_WINDOW_FLAG_FRAME;

            htime::initialise();
            hresmgr::initialise();
            hart::tasks::scheduler::initialise(
                hconfigopt::getInt("taskgraph", "workercount", 4), 
                hconfigopt::getUint("taskgraph", "jobqueuesize", 256));

            bgfx::sdlSetWindow(m_window);
            bgfx::renderFrame(); // calling this before bgfx::init prevents the render thread being created
            bgfx::init(bgfx::RendererType::Direct3D11);

            //m_thread.init(MainThreadEntry::threadFunc, &m_mte);

            // Force window resolution...
            //WindowHandle defaultWindow = { 0 };
            //setWindowSize(defaultWindow, m_width, m_height, true);
            SDL_SetWindowSize(m_window, m_width, m_height);

            // Application init
            static uint32_t buttom_remap[] = {
                1, 0, 2, 3, 4
            };
            engineEvents[(uint32_t)EgEvent::MouseMove] = addEventHandler(Event::MouseMotion, [&](Event evt_id, EventData const* evt) {
                mouseX = evt->motion.x;
                mouseY = evt->motion.y;
            });
            engineEvents[(uint32_t)EgEvent::MouseBtnDown] = addEventHandler(Event::MouseButtondown, [&](Event evt_id, EventData const* evt) {
                mouseButtons[buttom_remap[evt->button.button]] = true;
            });
            engineEvents[(uint32_t)EgEvent::MouseBtnUp] = addEventHandler(Event::MouseButtonup, [&](Event evt_id, EventData const* evt) {
                mouseButtons[buttom_remap[evt->button.button]] = false;
            });
            engineEvents[(uint32_t)EgEvent::MouseWheel] = addEventHandler(Event::MouseWheel, [&](Event evt_id, EventData const* evt) {
                wheelDelta = evt->wheel.y;
            });
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
            // Init objects
            hobjfact::objectFactoryRegistar(hrnd::Shader::getObjectDefinition(), nullptr);
            hobjfact::objectFactoryRegistar(hrnd::Material::getObjectDefinition(), nullptr);
            hobjfact::objectFactoryRegistar(hrnd::MaterialSetup::getObjectDefinition(), nullptr);
            hobjfact::objectFactoryRegistar(hresmgr::Collection::getObjectDefinition(), nullptr);

            // init engine
            huuid::uuid_t sys_collection_resid = huuid::fromDwords(0x06360489280d4059,0x8faabfb0ed97e6fa);
            hresmgr::Handle sys_collection_hdl = hresmgr::loadResource(sys_collection_resid);

            while (!sys_collection_hdl.loaded()) hresmgr::update();

            {
            // ImGui init. Done after system collection load as this loads the imgui shaders
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize.x = float(m_width);
            io.DisplaySize.y = float(m_height);
            io.IniFilename = "imgui.ini";
            io.RenderDrawListsFn = imguiRenderStatic;  // Setup a render function, or set to NULL and call GetDrawData() after Render() to access the render data.
            io.UserData = this;

            imgui.decl
                .begin()
                .add(bgfx::Attrib::Position,  2, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
                .end();

            imgui.textureUniform = bgfx::createUniform("s_tex", bgfx::UniformType::Int1);

            uint8_t* data;
            int32_t width;
            int32_t height;
            io.Fonts->GetTexDataAsRGBA32(&data, &width, &height);
            imgui.texture = bgfx::createTexture2D( (uint16_t)width
                , (uint16_t)height
                , 1
                , bgfx::TextureFormat::BGRA8
                , 0
                , bgfx::copy(data, width*height*4)
                );

            static huuid::uuid_t vs_imgui_resid = huuid::fromDwords(0x25f9b47acb354dfd,0x83cf6931c1e339b4);
            static huuid::uuid_t fs_imgui_resid = huuid::fromDwords(0x58be50a6196b43c6,0xa7868c6b7a9ef9f4);                

            imgui.vsResHdl = hresmgr::loadResource(vs_imgui_resid);
            imgui.fsResHdl = hresmgr::loadResource(fs_imgui_resid);
            // get the loaded pointers
            imgui.vsResHdl.loaded();
            imgui.fsResHdl.loaded();
            //
            render::Shader* vs = (render::Shader*)imgui.vsResHdl.getData(render::Shader::getTypeCC());
            render::Shader* fs = (render::Shader*)imgui.fsResHdl.getData(render::Shader::getTypeCC());
            hdbassert(vs && fs, "ImGui shader aren't loaded. They should be loaded during startup");

            imgui.program = bgfx::createProgram(
                vs->getShaderProfileObject(render::resource::Profile_Direct3D11),
                fs->getShaderProfileObject(render::resource::Profile_Direct3D11)
            );

            bgfx::setViewClear(0
                ,BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
                ,0x303030ff
                ,1.0f
                ,0
            );
            bgfx::setViewRect(0
                ,0
                ,0
                ,m_width
                ,m_height
            );

            }

            // Info the game that main engine assets are loaded.
            game->postSystemAssetLoad();

            htime::update();

            taskGraph.addTask("hresmgr::update", [&](htasks::Info*) {
                hresmgr::update();
            });

            game->taskGraphSetup(&taskGraph);

            bool test_wnd_open = false;
            bool exit = false;
            SDL_Event event;
            while (!exit)
            {
                hprofile_scope(Frame);
                htime::update();

                while (SDL_PollEvent(&event) )
                {
                    switch (event.type)
                    {
                    case SDL_QUIT:
                        exit = true;
                        break;
                    default:
                        for (const auto& i : eventHandlers[(uint32_t)SDL2SystemEvent[event.type]]) {
                            i(SDL2SystemEvent[event.type], &event);
                        }
                        break;
                    }
                }
                ImGuiIO& io = ImGui::GetIO();
                io.DeltaTime = htime::deltaSec();
                io.MousePos = ImVec2(float(mouseX), float(mouseY));
                for (size_t i = 0, n = HART_ARRAYSIZE(mouseButtons); i < n; ++i) {
                    io.MouseDown[i] = mouseButtons[i];
                }
                io.MouseWheel = float(wheelDelta);

                ImGui::NewFrame();
                
                hprofile_start(game_tick);
                game->tick(htime::deltaSec(), &taskGraph);
                //TODO: renderDebugMenus call & game->renderDebugMenus
                ImGui::ShowTestWindow(&test_wnd_open);
                hprofile_end();

                taskGraph.kick();

                hprofile_start(RenderFrame);
                ImGui::Render();
                bgfx::frame();
                hprofile_end();

                taskGraph.wait();

                wheelDelta = 0;
            }

            // Engine shutdown

            SDL_DestroyWindow(m_window);
            SDL_Quit();
            hprofile_shutdown();
            return 0;
        }

        struct {
            bgfx::VertexDecl    decl;
            bgfx::ProgramHandle program;
            bgfx::TextureHandle texture;
            bgfx::UniformHandle textureUniform;
            hresmgr::Handle     vsResHdl;
            hresmgr::Handle     fsResHdl;
        } imgui;

        //Only support one window currently
        SDL_Window* m_window = nullptr;
        uint32_t m_flags = 0;

        uint32_t m_width = HART_DEFAULT_WND_WIDTH;
        uint32_t m_height = HART_DEFAULT_WND_HEIGHT;
        float m_aspectRatio = float(HART_DEFAULT_WND_WIDTH)/float(HART_DEFAULT_WND_HEIGHT);

        htasks::Graph taskGraph;

        int32_t mouseX = 0;
        int32_t mouseY = 0;
        int32_t wheelDelta = 0;
        bool    mouseButtons[5];
        bool m_mouseLock = false;
        bool m_fullscreen = false;

        enum class EgEvent {
            MouseMove,
            MouseWheel,
            MouseBtnUp,
            MouseBtnDown,
            Max,
        };
        EventHandle engineEvents[EgEvent::Max];

        std::vector<EventHandler> eventHandlers[Event::Max];
    };

    static Context s_ctx;

Event Context::SDL2SystemEvent[SDL_LASTEVENT];
uint32_t Context::systemEvent2SDL[Event::Max];

int32_t run(int argc, char* argv[], GameInterface* game) {
    return s_ctx.run(argc, argv, game);
}

hart::engine::EventHandle addEventHandler(Event sysEventID,EventHandler handler) {
    return s_ctx.addEventHandler(sysEventID, handler);
}

void removeEventHandler(EventHandle handle) {
    s_ctx.removeEventHandler(handle);
}

}
} // namespace entry

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

