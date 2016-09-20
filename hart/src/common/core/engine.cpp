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
#include "hart/base/matrix.h"
#include "hart/render/vertexdecl.h"
#include "hart/render/render.h"

// object factory classes
#include "hart/render/shader.h"
#include "hart/render/material.h"
#include "hart/render/texture.h"
#include "hart/core/entity.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <stdio.h>
#include <vector>


namespace hart {
namespace engine {

#if HART_DEBUG_INFO
struct DebugMenu {
    hstd::string name;
    DebugMenuCallback renderFn;
    bool enabled;
};
#endif
struct Context {
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
#if HART_DEBUG_INFO
    DebugMenuHandle addDebugMenu(char const* name, DebugMenuCallback debug_menu) {
        for (uint32_t i =0, n=(uint32_t)debugMenus.size(); i<n; ++i) {
            if (!debugMenus[i].renderFn) {
                debugMenus[i].name = name;
                debugMenus[i].renderFn = debug_menu;
                debugMenus[i].enabled = false;
                return i;
            }
        }

        debugMenus.resize(debugMenus.size()+1);
        debugMenus.back().name = name;
        debugMenus.back().renderFn = debug_menu;
        debugMenus.back().enabled = false;
        return uint32_t(debugMenus.size()-1);
    }
#endif
    void removeDebugMenu(DebugMenuHandle handle) {
        hdbassert(handle >= debugMenus.size(), "Inavalid handle.\n");
        debugMenus[handle].renderFn = DebugMenuCallback();
        debugMenus[handle].enabled = false;
    }

    static void imguiRenderStatic(ImDrawData* draw_data) {
        ImGuiIO& io = ImGui::GetIO();
        ((Context*)io.UserData)->imguiRender(draw_data);
    }

    void imguiRender(ImDrawData* draw_data) {
        const ImGuiIO& io = ImGui::GetIO();
        const float width  = io.DisplaySize.x;
        const float height = io.DisplaySize.y;

        hMat44 ortho;
        ortho = hMat44::orthographic(0.0f, width, height, 0.0f, -1.0f, 1.0f);
        hrnd::begin(hrnd::View_Debug, hrnd::TechniqueType::TechniqueType_Main, nullptr, &ortho);
        hrnd::setMaterialSetup(imgui.material);

        // Render command lists
        for (int32_t ii = 0, num = draw_data->CmdListsCount; ii < num; ++ii)
        {
            const ImDrawList* drawList = draw_data->CmdLists[ii];
            uint32_t numVertices = (uint32_t)drawList->VtxBuffer.size();
            uint32_t numIndices  = (uint32_t)drawList->IdxBuffer.size();
            hrnd::beginInlineBatch(imgui.vDecl, (void*)drawList->IdxBuffer.begin(), numIndices, (void*)drawList->VtxBuffer.begin(), numVertices);

            uint32_t offset = 0;
            for (const ImDrawCmd* cmd = drawList->CmdBuffer.begin(), *cmdEnd = drawList->CmdBuffer.end(); cmd != cmdEnd; ++cmd)
            {
                if (cmd->UserCallback)
                {
                    cmd->UserCallback(drawList, cmd);
                }
                else if (0 != cmd->ElemCount)
                {
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

                    uint16_t xx = uint16_t(hutil::tmax(cmd->ClipRect.x, 0.0f) );
                    uint16_t yy = uint16_t(hutil::tmax(cmd->ClipRect.y, 0.0f) );
                    uint16_t ww = uint16_t(hutil::tmin(cmd->ClipRect.z, 65535.0f)-xx);
                    uint16_t hh = uint16_t(hutil::tmin(cmd->ClipRect.w, 65535.0f)-yy);
                    hrnd::setScissor(xx, yy, ww, hh);
                    //bgfx::setTexture(0, imgui.textureUniform, th);
                    imgui.material->setParameter(imgui.textureInputHandle, &imgui.texture);
                    hrnd::inlineBatchSubmit(offset, cmd->ElemCount, 0, numVertices);
                }

                offset += cmd->ElemCount;
            }
            hrnd::endInlineBatch();
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
        m_aspectRatio = float(m_width)/float(m_height);

        SDL_Init(SDL_INIT_GAMECONTROLLER);

        m_window = SDL_CreateWindow(hconfigopt::getStr("window", "title", "---")
                        , SDL_WINDOWPOS_UNDEFINED
                        , SDL_WINDOWPOS_UNDEFINED
                        , m_width
                        , m_height
                        , SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

        hrnd::initialise(m_window);
        htime::initialise();
        hresmgr::initialise();
        hart::tasks::scheduler::initialise(
            hconfigopt::getInt("taskgraph", "workercount", 4), 
            hconfigopt::getUint("taskgraph", "jobqueuesize", 256));

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
        hobjfact::objectFactoryRegister(hrnd::Shader::getObjectDefinition(), nullptr);
        hobjfact::objectFactoryRegister(hrnd::Material::getObjectDefinition(), nullptr);
        hobjfact::objectFactoryRegister(hrnd::MaterialSetup::getObjectDefinition(), nullptr);
        hobjfact::objectFactoryRegister(hrnd::TextureRes::getObjectDefinition(), nullptr);
		hobjfact::objectFactoryRegister(entity::EntityTemplate::getObjectDefinition(), nullptr);
		hobjfact::objectFactoryRegister(entity::Entity::getObjectDefinition(), nullptr);
        hobjfact::objectFactoryRegister(hresmgr::Collection::getObjectDefinition(), nullptr);

        game->postObjectFactoryRegister();

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

            hrnd::VertexElement elements[] = {
                {hrnd::Semantic::Position,  hrnd::SemanticType::Float, 2, false},
                {hrnd::Semantic::TexCoord0, hrnd::SemanticType::Float, 2, false},
                {hrnd::Semantic::Color0,    hrnd::SemanticType::Uint8, 4,  true},
            };
            imgui.vDecl = hrnd::createVertexDecl(elements, (uint16_t)HART_ARRAYSIZE(elements));


            uint8_t* data;
            int32_t width;
            int32_t height;
            io.Fonts->GetTexDataAsRGBA32(&data, &width, &height);
            imgui.texture = hrnd::createTexture2D(width, height, 1, hrnd::TextureFormat_BGRA8, 0, data, width*height*4);

            //TODO: maybe not hardcode this? could come from the ini file...
            static huuid::uuid_t imgui_material = huuid::fromDwords(0x013d177963434046,0xbc851eda6667af12);

            // get the loaded pointers
            imgui.materialHdl = hresmgr::loadResource(imgui_material);
            imgui.materialHdl.loaded();
            imgui.material = imgui.materialHdl.getData<hrnd::MaterialSetup>();
            hdbassert(imgui.material, "ImGui material isn't loaded. It should be loaded during startup.\n");
            imgui.textureInputHandle = imgui.material->getInputParameterHandle("s_tex");
            hdbassert(imgui.textureInputHandle.isValid(), "ImGui material is missing s_tex texture input.\n");

            hrnd::ViewDef views;
            views.id = hrnd::View_Debug;
            views.clearColour = true;
            views.colourValue = 0x303030ff;
            hrnd::resetViews(&views, 1);
        }

        // Info the game that main engine assets are loaded.
        game->postSystemAssetLoad();

        htime::update();
        /* -- bgfx resources need to be loaded on the main thread, so must call update there.
        taskGraph.addTask("hresmgr::update", [&](htasks::Info*) {
            hresmgr::update();
        });
        */

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
            
            hprofile_start(game_pretick);
            game->preTick(&taskGraph);
            hprofile_end();
            hprofile_start(game_tick);
            hresmgr::update();
            taskGraph.kick();
            game->tick(htime::deltaSec());
            //TODO: renderDebugMenus call & game->renderDebugMenus
#if HART_DEBUG_INFO
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("Debug Menus"))
                {
                    for (auto& i : debugMenus) {
                        ImGui::MenuItem(i.name.c_str(), nullptr, &i.enabled);
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
            for (auto const& i : debugMenus) {
                if (i.renderFn && i.enabled) {
                    i.renderFn();
                }
            }
#endif
            //ImGui::ShowTestWindow(&test_wnd_open);
            hprofile_end();
            hprofile_start(game_posttick);
            game->postTick();
            taskGraph.wait();
            hprofile_end();

            hprofile_start(RenderFrame);
            game->render();
            ImGui::Render();
            bgfx::frame();
            hprofile_end();

            wheelDelta = 0;
        }

        // Engine shutdown

        SDL_DestroyWindow(m_window);
        SDL_Quit();
        hprofile_shutdown();
        return 0;
    }

    struct {
        hrnd::VertexDecl*    vDecl;
        hrnd::MaterialSetup* material;
        bgfx::TextureHandle texture;
        hrnd::MaterialInputHandle textureInputHandle;
        hresmgr::Handle     materialHdl;
    } imgui;

    //Only support one window currently
    SDL_Window* m_window = nullptr;
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

    hstd::vector<EventHandler> eventHandlers[Event::Max];
#if HART_DEBUG_INFO
    hstd::vector<DebugMenu> debugMenus;
#endif
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
#if HART_DEBUG_INFO
DebugMenuHandle addDebugMenu(char const* name, DebugMenuCallback debug_menu) {
    return s_ctx.addDebugMenu(name, debug_menu);
}

void removeDebugMenu(DebugMenuHandle handle) {
    s_ctx.removeDebugMenu(handle);
}
#endif
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

