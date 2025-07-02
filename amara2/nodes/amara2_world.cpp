namespace Amara {
    class Demiurge;
    
    class World: public Node {
    public:
        Amara::Demiurge* demiurge = nullptr;

        Amara::InputManager inputManager;
        AssetManager assets;
        AnimationFactory animations;
        ShaderManager shaders;

        AudioMaster* audio = nullptr;

        std::string base_dir_path;

        SDL_Window* window = nullptr;
        Uint32 windowID = 0;
        std::string windowTitle;
        
        bool create_window_on_start = false;

        float windowW = 640;
        float windowH = 360;
        Rectangle window_dim = { pos.x, pos.y, windowW, windowH };

        bool windowMoved = false;
        bool windowResized = false;
        bool windowFocused = false;

        ScreenModeEnum screenMode = ScreenModeEnum::Windowed;

        #ifdef AMARA_OPENGL
        SDL_GLContext glContext = NULL;
        #endif
        SDL_Renderer* renderer = nullptr;
        SDL_GPUDevice* gpuDevice = nullptr;

        GraphicsEnum graphics;
        
        Amara::Color backgroundColor = Amara::Color::Black;

        Rectangle display;
        Uint32 displayID = 0;
        Uint32 rec_displayID = 0;

        Vector2 rec_pos = { -1, -1 };

        float virtualWidth = -1;
        float virtualHeight = -1;

        Vector2 virtualScale = Vector2( 1, 1 );

        Rectangle viewport;
        Rectangle rec_windowed_size;

        Rectangle view = { 0, 0, 0, 0 };
        
        int vsync = 0;
        bool headless = true;
        bool resizable = 0;
        bool transparent = false;
        bool alwaysOnTop = false;

        bool clickThrough = false;
        bool clickThroughState = false;
        bool forcedClickThrough = false;

        bool exception_thrown = false;
        
        std::vector<std::string> entryScenes;
        bool created_entry_scenes = false;

        std::vector<Amara::GraphicsEnum> graphics_priority = Amara_Default_Graphics_Priority;

        GPUHandler gpuHandler;
        RenderBatch renderBatch;

        #ifdef AMARA_OPENGL
        ShaderProgram* defaultShaderProgram = nullptr;
        #endif

        bool update_mouse = false;

        float left = 0;
        float right = 0;
        float top = 0;
        float bottom = 0;
        
        World(): Node() {
            set_base_node_id("World");
            world = this;
            passOnPropsEnabled = false;
            is_world = true;
        }

        virtual void init() override {
            renderBatch.gameProps = gameProps;
            assets.gameProps = gameProps;
            animations.gameProps = gameProps;
            shaders.gameProps = gameProps;
            
            inputManager.init(gameProps, this);

            base_dir_path = gameProps->system->getBasePath();
            
            Amara::Node::init();
        }

        void update_window() {
            if (window != nullptr) {
                displayID = SDL_GetDisplayForWindow(window);
                if (displayID != rec_displayID) {
                    SDL_Rect displayBounds;
                    if (SDL_GetDisplayUsableBounds(displayID, &displayBounds)) {
                        display = Rectangle(
                            static_cast<float>(displayBounds.x), 
                            static_cast<float>(displayBounds.y),
                            static_cast<float>(displayBounds.w), 
                            static_cast<float>(displayBounds.h)
                        );
                    }
                }
                gameProps->display = display;
                gameProps->displayID = displayID;
                
                gameProps->current_window = window;
                viewport = Rectangle( 0, 0, windowW, windowH );
                gameProps->master_viewport = viewport;

                int wx, wy;
                SDL_GetWindowPosition(window, &wx, &wy);
                float fwx = static_cast<int>(wx), fwy = static_cast<int>(wy);
                if (fwx != rec_pos.x || fwy != rec_pos.y) windowMoved = true;
                
                if (rec_pos.x != pos.x || rec_pos.y != pos.y) {
                    fwx += pos.x - rec_pos.x;
                    fwy += pos.y - rec_pos.y;
                    SDL_SetWindowPosition(window, fwx, fwy);
                }
                pos.x = fwx;
                pos.y = fwy;
                rec_pos = pos;

                int ww, wh;
                SDL_GetWindowSize(window, &ww, &wh);
                float fww = static_cast<float>(ww), fwh = static_cast<float>(wh);
                if (window_dim.w != fww || window_dim.h != fwh) windowResized = true;

                if (window_dim.w != windowW || window_dim.h != windowH) {
                    fww += windowW - window_dim.w;
                    fwh += windowH - window_dim.h;
                    SDL_SetWindowSize(window, fww, fwh);
                }
                windowW = fww;
                windowH = fwh;
                window_dim = Rectangle( pos.x, pos.y, windowW, windowH );
                gameProps->window_dim = window_dim;
            }
            else {
                viewport = Rectangle( pos.x, pos.y, windowW, windowH );
                display = gameProps->display;
            }
        }

        virtual void update_properties() override {
            gameProps->world = this;
            gameProps->inputManager = &inputManager;
            gameProps->assets = &assets;
            gameProps->animations = &animations;
            gameProps->shaders = &shaders;
            gameProps->audio = audio;

            gameProps->lua["Mouse"] = &inputManager.mouse;
            gameProps->lua["Touch"] = &inputManager.touch;
            
            if (window) {
                #ifdef AMARA_OPENGL
                gameProps->glContext = NULL;
                #endif
                gameProps->renderer = nullptr;
                gameProps->gpuDevice = nullptr;

                #ifdef AMARA_OPENGL
                if (graphics == GraphicsEnum::OpenGL && glContext != NULL) {
                    gameProps->glContext = glContext;
                }
                #endif
                if (graphics == GraphicsEnum::Render2D && renderer != nullptr) {
                    gameProps->renderer = renderer;
                }
                if (gpuDevice != nullptr) {
                    gameProps->gpuDevice = gpuDevice;
                }
                gameProps->graphics = graphics;
                gameProps->gpuHandler = &gpuHandler;
                gameProps->renderBatch = &renderBatch;

                gameProps->transparent_window = transparent;
            }
            update_window();

            Node::update_properties();
        }

        virtual nlohmann::json toJSON() override {
            nlohmann::json data = Amara::Node::toJSON();

            data["window"] = nlohmann::json::object();
            nlohmann::json& window_data = data["window"];
            
            window_data["windowTitle"] = windowTitle;
            window_data["width"] = windowW;
            window_data["height"] = windowH;
            window_data["virtualWidth"] = virtualWidth;
            window_data["virtualHeight"] = virtualHeight;
            window_data["resizable"] = resizable;
            window_data["vsync"] = vsync;
            window_data["headless"] = headless;
            window_data["transparent"] = transparent;
            window_data["alwaysOnTop"] = alwaysOnTop;
            window_data["clickThrough"] = clickThrough;
            window_data["forcedClickThrough"] = forcedClickThrough;
            window_data["screenMode"] = screenMode;
            window_data["graphics"] = graphics_to_string(graphics);
            window_data["backgroundColor"] = backgroundColor.toJSON();

            data["basePath"] = base_dir_path;
            data["entryScenes"] = entryScenes;

            return data;
        }

        void configure_window(nlohmann::json config) {
            bool resizeWindow = false;
            headless = false;

            create_window_on_start = !gameProps->integrate_new_windows;
            if (json_has(config, "w")) {
                windowW = config["w"];
                resizeWindow = true;
            }
            if (json_has(config, "h")) {
                windowH = config["h"];
                resizeWindow = true;
            }
            if (json_has(config, "width")) {
                windowW = config["width"];
                resizeWindow = true;
            }
            if (json_has(config, "height")) {
                windowH = config["height"];
                resizeWindow = true;
            }
            if (json_has(config, "virtualWidth")) {
                virtualWidth = config["virtualWidth"];
            }
            if (json_has(config, "virtualHeight")) {
                virtualHeight = config["virtualHeight"];
            }
            if (json_has(config, "virtualSize")) {
                if (config["virtualSize"].is_number()) {
                    virtualWidth = config["virtualSize"];
                    virtualHeight = config["virtualSize"];
                }
                else if (config["virtualSize"].is_object()) {
                    nlohmann::json size = config["virtualSize"];
                    if (json_has(config, "w")) virtualWidth = config["virtualSize"]["w"];
                    if (json_has(config, "h")) virtualHeight = config["virtualSize"]["h"];
                    if (json_has(config, "width")) virtualWidth = config["virtualSize"]["width"];
                    if (json_has(config, "height")) virtualHeight = config["virtualSize"]["height"];
                }
                else if (config["virtualSize"].is_array())  {
                    virtualWidth = config["virtualSize"][0];
                    virtualHeight = config["virtualSize"][1];
                }
            }
            if (json_has(config, "resizable")) {
                resizable = config["resizable"];
                if (window) SDL_SetWindowResizable(window, resizable);
            }
            if (json_is(config, "singleWindowApplication")) {
                gameProps->integrate_new_windows = true;
            }
            if (json_has(config, "headless")) {
                create_window_on_start = !config["headless"].get<bool>();
            }
            if (json_has(config, "title")) {
                windowTitle = config["title"];
                if (window) SDL_SetWindowTitle(window, windowTitle.c_str());
                if (id.empty()) id = windowTitle;
            }
            if (json_has(config, "graphics")) {
                if (config["graphics"].is_array()) {
                    nlohmann::json list = config["graphics"];
                    graphics_priority.clear();
                    for (int i = 0; i < list.size(); i++) {
                        graphics_priority.push_back(list[i]);
                    }
                }
                else if (config["graphics"].is_number()) {
                    graphics_priority = { config["graphics"] };
                }
                else {
                    debug_log("Error: Invalid graphics setting.");
                    gameProps->breakWorld();
                }
            }
            if (json_has(config, "vsync")) {
                nlohmann::json val = config["vsync"];
                if (val.is_boolean()) {
                    vsync = val.get<bool>() ? 1 : 0;
                }
                else if (val.is_number()) {
                    vsync = val;
                }
                else if (val.is_string()) {
                    if (String::equal(val, "adaptive")) {
                        vsync = -1;
                    }
                }
                setVsync(vsync);
            }
            if (resizeWindow && window != nullptr) {
                SDL_SetWindowSize(window, windowW, windowH);
            }
            if (json_has(config, "screenMode")) {
                if (demiurge) {
                    debug_log("Note: Demiurgic presence. Screen Mode Overridden: ", graphics_to_string(gameProps->graphics));
                    debug_log("Control will be handed over in target builds.");
                }
                else if (config["screenMode"].is_null()) {
                    debug_log("Error: Invalid screen mode setting.");
                    gameProps->breakWorld();
                }
                else {
                    setScreenMode(config["screenMode"]);
                }
            }
            if (json_has(config, "transparent")) {
                if (window == nullptr) {
                    transparent = config["transparent"];
                    backgroundColor = Amara::Color::Transparent;
                }
            }
            if (json_has(config, "alwaysOnTop")) {
                setAlwaysOnTop(config["alwaysOnTop"]);
            }
            if (json_has(config, "clickThrough")) {
                setClickThrough(config["clickThrough"]);
            }
            if (json_has(config, "forcedClickThrough")) {
                setForcedClickThrough(config["forcedClickThrough"]);
            }
            if (json_has(config, "backgroundColor")) {
                backgroundColor = config["backgroundColor"];
            }
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            Amara::Node::configure(config);

            if (json_has(config, "window")) {
                configure_window(config["window"]);
            }

            if (json_has(config, "basePath")) {
                base_dir_path = config["basePath"];
                gameProps->system->setBasePath(base_dir_path);
            }

            if (json_has(config, "scene")) {
                entryScenes.push_back(config["scene"]);
            }
            if (json_has(config, "scenes")) {
                nlohmann::json keys = config["scenes"];
                if (keys.is_string()) {
                    for (int i = 0; i < keys.size(); i++) {
                        entryScenes.push_back(keys[i]);
                    }
                }
            }
            if (json_has(config, "backgroundColor")) {
                backgroundColor = config["backgroundColor"];
            }

            return this;
        }
       
        void centerWindow() {
            pos.x = display.x + (display.w-windowW)/2;
            pos.y = display.y + (display.h-windowH)/2;
        }

        void resizeWindow(float _w, float _h) {
            windowW = _w;
            windowH = _h;
        }

        void fitToDisplay() {
            resizeWindow(display.w, display.h);
            centerWindow();
        }

        void fitToDisplay(int _displayID) {
            displayID = _displayID;
            display = gameProps->game->getDisplayBounds(displayID);
            fitToDisplay();
        }
        
        void setScreenMode(ScreenModeEnum _sm) {
            if (window) {
                screenMode = _sm;
                switch (screenMode) {
                    case ScreenModeEnum::Windowed: {
                        SDL_SetWindowFullscreen(window, false);
                        SDL_SetWindowBordered(window, true);
                        break;
                    }
                    case ScreenModeEnum::BorderlessWindowed: {
                        SDL_SetWindowFullscreen(window, false);
                        SDL_SetWindowBordered(window, false);
                        break;
                    }
                    case ScreenModeEnum::Fullscreen: {
                        SDL_SetWindowBordered(window, true);
                        SDL_SetWindowFullscreen(window, true);
                        
                        const SDL_DisplayMode* displayMode = SDL_GetCurrentDisplayMode(displayID);
                        if (displayMode) SDL_SetWindowFullscreenMode(window, displayMode);
                        break;
                    }
                    case ScreenModeEnum::BorderlessFullscreen: {
                        SDL_SetWindowBordered(window, false);
                        SDL_SetWindowFullscreen(window, true);
                        break;
                    }
                    default:
                        debug_log("Error: Invalid graphics renderer setting given.");
                        gameProps->breakWorld();
                        break;
                }
            }
            else {
                if (screenMode == ScreenModeEnum::Fullscreen || screenMode == ScreenModeEnum::BorderlessFullscreen) {
                    if (_sm == ScreenModeEnum::Windowed || _sm == ScreenModeEnum::BorderlessWindowed) {
                        pos.x = rec_windowed_size.x;
                        pos.y = rec_windowed_size.y;
                        windowW = rec_windowed_size.w;
                        windowH = rec_windowed_size.h;
                        window_dim = Rectangle( pos.x, pos.y, windowW, windowH );
                    }
                }
                else if (screenMode == ScreenModeEnum::Windowed || screenMode == ScreenModeEnum::BorderlessWindowed) {
                    if (_sm == ScreenModeEnum::Fullscreen || _sm == ScreenModeEnum::BorderlessFullscreen) {
                        rec_windowed_size = Rectangle( pos.x, pos.y, windowW, windowH );
                        pos.x = display.x;
                        pos.y = display.y;
                        windowW = display.w;
                        windowH = display.h;
                        window_dim = Rectangle( pos.x, pos.y, windowW, windowH );
                    }
                }
                screenMode = _sm;
            }
        }

        void setAlwaysOnTop(bool _t) {
            alwaysOnTop = _t;
            if (window) SDL_SetWindowAlwaysOnTop(window, _t);
        }

        void setClickThrough(bool enabled) {
            clickThrough = enabled;
            setClickThroughState(enabled);
        }

        void setVsync(int _vsync) {
            vsync = _vsync;
            if (graphics == GraphicsEnum::Render2D && renderer != nullptr) {
                if (SDL_SetRenderVSync(renderer, vsync) < 0) {
                    debug_log("Warning: Failed to set VSync for SDL_Renderer: ", SDL_GetError());
                }
            }
            #ifdef AMARA_OPENGL
            else if (graphics == GraphicsEnum::OpenGL && glContext != NULL) {
                SDL_GL_MakeCurrent(window, glContext);
                if (SDL_GL_SetSwapInterval(vsync) < 0) {
                    debug_log("Warning: Failed to set VSync mode ", vsync, " for OpenGL: ", SDL_GetError());
                }
            }
            #endif
        }

        void setClickThroughState(bool enabled) {
            if (clickThroughState == enabled || window == nullptr) return;

            clickThroughState = enabled;

            SDL_PropertiesID window_props = SDL_GetWindowProperties(window);
            if (!window_props) {
                debug_log("Warning: Failed to get window properties: ", SDL_GetError());
                return;
            }

            #if defined(_WIN32)
                HWND hwnd = (HWND)SDL_GetPointerProperty(window_props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
                if (hwnd) {
                    // Ensure SetWindowLongPtr is available (might need different versions for 32/64 bit)
                    #ifdef _WIN64
                        LONG_PTR currentStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
                    #else
                        LONG_PTR currentStyle = GetWindowLong(hwnd, GWL_EXSTYLE); // Use GetWindowLong for 32-bit
                    #endif

                    LONG_PTR newStyle;
                    if (enabled) {
                        newStyle = currentStyle | WS_EX_LAYERED | WS_EX_TRANSPARENT;
                    } else {
                        newStyle = currentStyle & ~(WS_EX_LAYERED | WS_EX_TRANSPARENT);
                    }

                    if (newStyle != currentStyle) {
                        #ifdef _WIN64
                            SetWindowLongPtr(hwnd, GWL_EXSTYLE, newStyle);
                        #else
                            SetWindowLong(hwnd, GWL_EXSTYLE, newStyle); // Use SetWindowLong for 32-bit
                        #endif
                    }
                } else {
                    debug_log("Warning: Failed to get HWND for click-through setup.");
                }
            #elif defined(__linux__)
                Display* display = (Display*)SDL_GetPointerProperty(window_props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
                Window xwindow = (Window)SDL_GetNumberProperty(window_props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);

                if (display && xwindow != 0) {
                    int shape_event_base, shape_error_base;
                    if (XShapeQueryExtension(display, &shape_event_base, &shape_error_base)) {
                        if (enabled) {
                            Region region = XCreateRegion();
                            if (region) {
                                XShapeCombineRegion(display, xwindow, ShapeInput, 0, 0, region, ShapeSet);
                                XDestroyRegion(region);
                            } else {
                                debug_log("Warning: Failed to create X region for click-through.");
                            }
                        } else {
                            XShapeCombineRegion(display, xwindow, ShapeInput, 0, 0, None, ShapeSet);
                        }
                        XFlush(display);
                    } else {
                        debug_log("Warning: X Shape Extension not available. Cannot set click-through.");
                    }
                } else {
                    debug_log("Warning: Failed to get X11 Display or Window for click-through setup.");
                }
                
            #elif defined(__APPLE__) // Cocoa Implementation
                NSWindow* nsWindow = (NSWindow*)SDL_GetPointerProperty(window_props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
                if (nsWindow) {
                    [nsWindow setIgnoresMouseEvents:enabled];
                } else {
                    debug_log("Warning: Failed to get NSWindow for click-through setup.");
                }

            #else
                debug_log("Warning: Click-through not implemented for this platform.");
                return;
            #endif
        }

        void checkClickThrough() {
            if (clickThrough) {
                if (forcedClickThrough) {
                    setClickThroughState(true);
                }
                else {
                    Vector2 mousePos;
                    float mx, my;
                    int wx, wy;
                    SDL_MouseButtonFlags mouseFlags = SDL_GetGlobalMouseState(&mx, &my);
                    SDL_GetWindowPosition(window, &wx, &wy);
                    mousePos.x = static_cast<float>(mx) - static_cast<float>(wx);
                    mousePos.y = static_cast<float>(my) - static_cast<float>(wy);

                    if (inputManager.checkPointerHover(mousePos)) {
                        if (clickThroughState) {
                            setClickThroughState(false);
                            SDL_RaiseWindow(window);
                            
                            update_mouse = true;
                        }
                    }
                    else {
                        setClickThroughState(true);
                    }
                }
            }
        }

        void setForcedClickThrough(bool enabled) {
            forcedClickThrough = enabled;
            if (clickThrough && forcedClickThrough) {
                setClickThroughState(true);
            }
        }

        void setup_new_window() {
            if (window == nullptr) return;

            windowID = SDL_GetWindowID(window);
            window_dim = Rectangle( pos.x, pos.y, windowW, windowH );
            
            setScreenMode(screenMode);
        }

        void setWindowTitle(std::string _t) {
            if (window) SDL_SetWindowTitle(window, _t.c_str());
            windowTitle = _t;
        }

        void minimizeWindow() {
            if (window) SDL_MinimizeWindow(window);
        }

        void maximizeWindow() {
            if (!resizable) {
                debug_log("Warning: Non-resizable windows cannot be maximized.");
            }
            else if (window) SDL_MaximizeWindow(window);
        }

        void restoreWindow() {
            if (window) {
                SDL_RestoreWindow(window);
                SDL_RaiseWindow(window);
            }
        }

        void showWindow() {
            if (window) SDL_ShowWindow(window); 
        }

        void hideWindow() {
            if (window) SDL_HideWindow(window); 
        }

        bool create_graphics_window(int flags) {
            if (window != nullptr) return false;

            flags |= SDL_WINDOW_HIDDEN;

            if (resizable) flags |= SDL_WINDOW_RESIZABLE;
            if (transparent) flags |= SDL_WINDOW_TRANSPARENT;
            if (alwaysOnTop) flags |= SDL_WINDOW_ALWAYS_ON_TOP;

            window = SDL_CreateWindow(
                windowTitle.c_str(),
                windowW, windowH,
                flags
            );

            if (!windowTitle.empty()) {
                SDL_SetWindowTitle(window, windowTitle.c_str());
            }

            windowFocused = true;

            if (window) {
                setup_new_window();
                return true;
            }
            debug_log("Error: Failed to create window. ", SDL_GetError());
            return false;
        }

        bool create_graphics_window() {
            return create_graphics_window(0);
        }

        bool create_gpu_device(Amara::GraphicsEnum g) {
            if (!create_graphics_window()) return false;

            int shaderFlags = 0;

            switch (g) {
                case GraphicsEnum::VulkanMetalDirectX:
                    shaderFlags |= SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL;
                    break;
                case GraphicsEnum::Vulkan:
                    shaderFlags |= SDL_GPU_SHADERFORMAT_SPIRV;
                    break;
                case GraphicsEnum::DirectX:
                    shaderFlags |= SDL_GPU_SHADERFORMAT_DXIL;
                    break;
                case GraphicsEnum::DirectX_Legacy:
                    shaderFlags |= SDL_GPU_SHADERFORMAT_DXBC;
                    break;
                default:
                    break;
            }
            gpuDevice = SDL_CreateGPUDevice(
                SDL_GPU_SHADERFORMAT_DXIL,
                false,
                NULL
            );
            if (!gpuDevice) {
                debug_log("Error: SDL_CreatedGPUDevice failed: ", SDL_GetError());
                if (window) {
                    SDL_DestroyWindow(window);
                    window = nullptr;
                }
                return false;
            }
            if (!SDL_ClaimWindowForGPUDevice(gpuDevice, window)) {
                debug_log("Error: Unable to associate window with gpu device. ", SDL_GetError(), ", Renderer: ", graphics_to_string(g));
                SDL_DestroyGPUDevice(gpuDevice);
                gpuDevice = NULL;
                if (window) {
                    SDL_DestroyWindow(window);
                    window = nullptr;
                }
                return false;
            }
            return true;
        }

        void createWindowAndRenderer() {
            bool renderer_created = false;

            for (GraphicsEnum g: graphics_priority) {
                if (renderer_created) break;
                switch (g) {
                    case Amara::GraphicsEnum::Render2D:
                        if (window == nullptr) {
                            int flags = 0;

                            if (resizable) flags |= SDL_WINDOW_RESIZABLE;
                            if (transparent) flags |= SDL_WINDOW_TRANSPARENT;
                            if (alwaysOnTop) flags |= SDL_WINDOW_ALWAYS_ON_TOP;

                            if (!SDL_CreateWindowAndRenderer(
                                windowTitle.c_str(),
                                windowW, windowH,
                                flags,
                                &window,
                                &renderer
                            )) {
                                if (renderer) SDL_DestroyRenderer(renderer);
                                if (window) SDL_DestroyWindow(window);
                                window = nullptr;
                                renderer = nullptr;
                            }
                            setup_new_window();
                        }
                        else {
                            renderer = SDL_CreateRenderer(window, NULL);
                        }
                        if (window == nullptr || renderer == nullptr) {
                            debug_log("Error: Failed to create 2D Renderer. ", SDL_GetError());
                        }
                        else {
                            gameProps->renderer = renderer;

                            renderer_created = true;
                            graphics = g;
                        }
                        break;
                    case Amara::GraphicsEnum::OpenGL:
                        #ifdef AMARA_OPENGL
                        if (gameProps->glContext != NULL) {
                            debug_log("Error: Multiple OpenGL worlds is currently not supported.");
                            continue;
                        }

                        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
                        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
                        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
                        
                        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
                        SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);

                        if (create_graphics_window(SDL_WINDOW_OPENGL)) {
                            glContext = SDL_GL_CreateContext(window);
                            renderer_created = true;
                            if (glContext == NULL) {
                                renderer_created = false;
                                debug_log("Error: Failed to create GL Context. ", SDL_GetError());
                            }
                            else {
                                try {
                                    if (!gameProps->glFunctionsLoaded) LoadOpenGLFunctions();
                                }
                                catch (...) {
                                    debug_log("Error: Failed to load OpenGL API.");
                                    SDL_GL_DestroyContext(glContext);
                                    if (window) SDL_DestroyWindow(window);
                                    glContext = NULL;
                                    window = nullptr;
                                    renderer_created = false;
                                    continue;
                                }
                            }
                            if (renderer_created) {
                                gameProps->glFunctionsLoaded = true;
                                gameProps->render_origin = this;
                                gameProps->glContext = glContext;

                                graphics = g;
                                gameProps->graphics = g;

                                SDL_GL_MakeCurrent(window, glContext);
                                glViewport(0, 0, static_cast<GLsizei>(windowW), static_cast<GLsizei>(windowH));

                                glEnable(GL_BLEND);
                                glEnable(GL_TEXTURE_2D);

                                gameProps->renderBatch = &renderBatch;
                                renderBatch.init();

                                update_properties();

                                prepareGLShaders();
                                
                                setShaderProgram("default");
                                gameProps->currentShaderProgram = shaderProgram;
                                gameProps->defaultShaderProgram = shaderProgram;
                                defaultShaderProgram = shaderProgram;
                                shaderProgram->applyShader();
                            }
                        }
                        #else
                        debug_log("Error: OpenGL is not enabled in this build. Please rebuild project with OpenGL enabled.");
                        #endif
                        break;
                    case Amara::GraphicsEnum::None:
                        graphics = g;
                        renderer_created = true;
                        break;
                    default:
                        if (create_gpu_device(g)) {
                            renderer_created = true;
                            gameProps->render_origin = this;
                            gameProps->gpuDevice = gpuDevice;
                            graphics = g;
                        }
                        break;
                }
            }

            if (window) {
                int wx, wy;
                SDL_GetWindowPosition(window, &wx, &wy);
                pos.x = static_cast<int>(wx);
                pos.y = static_cast<int>(wy);
                rec_pos = pos;

                gameProps->graphics = graphics;

                if (clickThrough) {
                    clickThrough = false;
                    setClickThrough(true);
                }
                setAlwaysOnTop(alwaysOnTop);

                if (vsync != 0) setVsync(vsync);

                assets.init();

                debug_log("Info: ", *this, " rendering to window using ", graphics_to_string(graphics));

                showWindow();
                restoreWindow();
            }
            else if (graphics == GraphicsEnum::None && gameProps->current_window != nullptr) {
                pos.x = (gameProps->master_viewport.w - windowW) / 2.0f;
                pos.y = (gameProps->master_viewport.h - windowH) / 2.0f;
            }
            else {
                debug_log("Error: Failed to create window. ", SDL_GetError());
                if (window) {
                    SDL_DestroyWindow(window);
                    window = nullptr;
                }
                if (renderer) {
                    SDL_DestroyRenderer(renderer);
                    renderer = nullptr;
                }
                if (gpuDevice) {
                    SDL_DestroyGPUDevice(gpuDevice);
                    gpuDevice = nullptr;
                }
                #ifdef AMARA_OPENGL
                if (glContext) {
                    SDL_GL_DestroyContext(glContext);
                    glContext = NULL;
                }
                #endif
                gameProps->breakWorld();
                return;
            }
        }

        #ifdef AMARA_OPENGL
        void prepareGLShaders() {
            gameProps->shaders->compileGLShader("defaultVert", defaultVertexShader, ShaderTypeEnum::Vertex);
            gameProps->shaders->compileGLShader("defaultFrag", defaultFragmentShader, ShaderTypeEnum::Fragment);
            gameProps->shaders->createShaderProgram("default", {
                { "vertex", "defaultVert" },
                { "fragment", "defaultFrag" }
            });
        }
        #endif

        virtual void preload() override {
            if (create_window_on_start) {
                if (window == nullptr) {
                    createWindowAndRenderer();
                }
            }
            else if (demiurge) {
                debug_log("Note: Demiurgic presence. Rendering Mode Overridden: ", graphics_to_string(gameProps->graphics));
                debug_log("Control will be handed over in target builds.");
            }
            
            update_properties();
            basePassOnProps();

            Amara::Node::preload();
        }

        virtual void create() override {
            if (!created_entry_scenes) {
                for (std::string key: entryScenes) {
                    createChild(key);
                }
                created_entry_scenes = true;
            }

            Amara::Node::create();

            audio = createChild("AudioMaster")->as<Amara::AudioMaster*>();
        }

        virtual void run(double deltaTime) override {
            checkClickThrough();

            if (!base_dir_path.empty()) {
                gameProps->system->setBasePath(base_dir_path);
            }
            if (audio) {
                if (!audio->destroyed) audio->update_properties();
                else audio = nullptr;
            }

            Amara::Node::run(deltaTime);

            inputManager.update(deltaTime);
            inputManager.clearQueue();

            if (window) gameProps->display = viewport;
            
            if (gameProps->lua_exception_thrown) {
                exception_thrown = true;
            }
        }

        void basePassOnProps() {
            reset_pass_on_props();

            viewport = Rectangle( 0, 0, windowW, windowH );

            if (virtualWidth > 0 || virtualHeight > 0) {
                if (virtualWidth <= 0) virtualWidth = windowW;
                if (virtualHeight <= 0) virtualHeight = windowH;
                float viewport_factor = viewport.w / viewport.h;
                float virtual_factor = virtualWidth / virtualHeight;
                float zoom;
                if (viewport_factor > virtual_factor) {
                    zoom = viewport.h / virtualHeight;
                }
                else {
                    zoom = viewport.w / virtualWidth;
                }
                gameProps->passOn.window_zoom = Vector2( zoom, zoom );
            }

            passOn = gameProps->passOn;
            passOnPropsEnabled = false;

            left = -(windowW/2) / passOn.window_zoom.x;
            right = (windowW/2) / passOn.window_zoom.x;
            top = -(windowH/2) / passOn.window_zoom.y;
            bottom = (windowH/2) / passOn.window_zoom.y;
            
            view = Rectangle(
                left, top,
                right - left,
                bottom - top
            );

            virtualScale.x = passOn.window_zoom.x;
            virtualScale.y = passOn.window_zoom.y;
        }
        
        virtual void draw(const Rectangle& v) override {
            update_properties();
            basePassOnProps();

            passOn = gameProps->passOn;
            passOnPropsEnabled = false;

            #ifdef AMARA_OPENGL
            if (graphics == GraphicsEnum::OpenGL && glContext != NULL) {
                gameProps->currentShaderProgram = nullptr;
            }
            #endif

            if (input.active) {
                input.queueInput(Quad(v), v, nullptr);
            }
            
            Amara::Node::draw(viewport);

            renderBatch.flush();
        }

        void prepareRenderer() {
            if (graphics == GraphicsEnum::Render2D && renderer != nullptr) {
                SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
                SDL_RenderClear(renderer);
                SDL_SetRenderTarget(gameProps->renderer, NULL);
                SDL_SetRenderViewport(gameProps->renderer, NULL);
                gameProps->master_viewport = viewport;
                gameProps->graphics = graphics;
            }
            #ifdef AMARA_OPENGL
            else if (graphics == GraphicsEnum::OpenGL && glContext != NULL) {
                gameProps->gpuHandler = &gpuHandler;
                gameProps->glContext = glContext;
                gameProps->renderBatch = &renderBatch;
                gameProps->master_viewport = viewport;
                gameProps->graphics = graphics;

                gameProps->renderBatch->newCycle();

                SDL_GL_MakeCurrent(window, glContext);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(0, 0, static_cast<GLsizei>(windowW), static_cast<GLsizei>(windowH));
                glClearColor(
                    backgroundColor.r / 255.0f,
                    backgroundColor.g / 255.0f,
                    backgroundColor.b / 255.0f,
                    backgroundColor.a / 255.0f
                );
                glClear(GL_COLOR_BUFFER_BIT);

                gameProps->defaultShaderProgram = defaultShaderProgram;
            }
            #endif
            else if (gpuDevice) {
                gameProps->gpuHandler = &gpuHandler;
                gameProps->master_viewport = viewport;
                gameProps->graphics = graphics;

                gpuHandler.commandBuffer = SDL_AcquireGPUCommandBuffer(gameProps->gpuDevice);
                if (gpuHandler.commandBuffer == NULL) {
                    debug_log("Error: AcquireGPUCommandBuffer failed: ", SDL_GetError());
                    gameProps->breakWorld();
                }
                if (!SDL_WaitAndAcquireGPUSwapchainTexture(gpuHandler.commandBuffer, gameProps->current_window, &gpuHandler.swapChainTexture, NULL, NULL)) {
                    debug_log("Error: WaitAndAcquireGPUSwapchainTexture failed: ", SDL_GetError());
                    gameProps->breakWorld();
                }
            }
        }

        void presentRenderer() {
            update_properties();

            if (graphics == GraphicsEnum::Render2D && renderer) {
                SDL_RenderPresent(renderer);
            }
            #ifdef AMARA_OPENGL
            else if (graphics == GraphicsEnum::OpenGL && glContext != NULL) {
                SDL_GL_SwapWindow(window);
            }
            #endif
            else if (gpuDevice) {
                gameProps->gpuHandler = &gpuHandler;
                SDL_SubmitGPUCommandBuffer(gpuHandler.commandBuffer);
            }
        }

        void handleMouseMovement(const Vector2& pos, const Vector2& movement) {
            Vector2 virtualPos = Vector2(
                (pos.x - viewport.w * 0.5f) / passOn.window_zoom.x,
                (pos.y - viewport.h * 0.5f) / passOn.window_zoom.y
            );
            inputManager.mouse.handleMovement(
                pos, virtualPos,
                Vector2(
                    movement.x / viewport.w,
                    movement.y / viewport.h
                ),
                Vector2(
                    movement.x / passOn.window_zoom.x,
                    movement.y / passOn.window_zoom.y
                )
            );
            inputManager.handleMouseMovement(pos);
        }

        void handleFingerEvent(const Vector2& pos, SDL_FingerID fingerID, SDL_EventType eventType) {
            Vector2 real_pos = Vector2(
                pos.x * viewport.w,
                pos.y * viewport.h
            );
            Vector2 virtualPos = Vector2(
                (real_pos.x - viewport.w * 0.5f) / passOn.window_zoom.x,
                (real_pos.y - viewport.h * 0.5f) / passOn.window_zoom.y
            );
            Pointer* pointer = inputManager.touch.activateAnyFinger(fingerID);
            if (pointer) {
                switch (eventType) {
                    case SDL_EVENT_FINGER_MOTION: {
                        pointer->handleMovement(
                            real_pos, virtualPos,
                            Vector2(
                                (real_pos.x - pointer->real_pos.x),
                                (real_pos.y - pointer->real_pos.y)
                            ),
                            Vector2(
                                (virtualPos.x - pointer->x),
                                (virtualPos.y - pointer->y)
                            )
                        );
                        break;
                    }
                    case SDL_EVENT_FINGER_DOWN: {
                        pointer->state.press();
                        pointer->real_pos = real_pos;
                        pointer->x = virtualPos.x;
                        pointer->y = virtualPos.y;
                        pointer->rec_position();
                        break;
                    }
                    case SDL_EVENT_FINGER_UP: {
                        pointer->state.release();
                        inputManager.touch.deactivateFinger(fingerID);
                        break;
                    }
                }
                inputManager.handleFingerEvent(real_pos, pointer, eventType);
            }
            else {
                debug_log("Error: Could not create pointer finger.");
            }
        }

        virtual void destroy() override {
            Amara::Node::destroy();
            
            assets.clear();
            shaders.clear();
            animations.clear();
            renderBatch.destroy();

            #ifdef AMARA_OPENGL
            if (glContext != NULL) {
                SDL_GL_DestroyContext(glContext);
                glContext = NULL;
            }
            #endif
            if (gpuDevice) {
                SDL_DestroyGPUDevice(gpuDevice);
                gpuDevice = nullptr;
            }
            if (renderer) {
                SDL_DestroyRenderer(renderer);
                renderer = nullptr;
            }
            if (window != nullptr) {
                SDL_DestroyWindow(window);
                window = nullptr;
            }
            
            if (demiurge) {
                removeFromDemiurge();
            }
        }

        void removeFromDemiurge();

        Rectangle getRect() {
            return Rectangle( pos.x, pos.y, windowW, windowH );
        }
        
        static void bind_lua(sol::state& lua) {
            lua.new_usertype<World>("World",
                sol::base_classes, sol::bases<Node>(),
                "w", &World::windowW,
                "h", &World::windowH,
                "width", &World::windowW,
                "height", &World::windowH,
                "rect", sol::property(
                    &Amara::World::getRect,
                    [](Amara::World& world, sol::object val) {
                        Amara::Rectangle rect = val;
                        world.pos.x = rect.x;
                        world.pos.y = rect.y;
                        world.windowW = rect.w;
                        world.windowH = rect.h;
                    }
                ),
                "viewport", sol::readonly(&World::viewport),
                "vw", &World::virtualWidth,
                "vh", &World::virtualHeight,
                "virtualWidth", &World::virtualWidth,
                "virtualHeight", &World::virtualHeight,
                "virtualScale", sol::readonly(&World::virtualScale),
                "left", sol::readonly(&World::left),
                "right", sol::readonly(&World::right),
                "top", sol::readonly(&World::top),
                "bottom", sol::readonly(&World::bottom),
                "view", sol::readonly(&World::view),
                "assets", sol::readonly(&World::assets),
                "shaders", sol::readonly(&World::shaders),
                "base_dir_path", sol::readonly(&World::base_dir_path),
                "display", sol::readonly(&World::display),
                "displayID", sol::readonly(&World::displayID),
                "graphics", sol::readonly(&World::graphics),
                "centerWindow", &World::centerWindow,
                "resizeWindow", &World::resizeWindow,
                "fitToDisplay", sol::overload(
                    sol::resolve<void()>( &World::fitToDisplay ),
                    sol::resolve<void(int)>( &World::fitToDisplay )
                ),
                "screenMode", sol::property([](const Amara::World& world) { return world.screenMode; }, &World::setScreenMode),
                "transparent", sol::property([](const Amara::World& world) { return world.transparent; }, [](Amara::World& world, sol::object value) {
                    debug_log("Error: Transparency can only be set in World configuration table.");
                    world.gameProps->breakWorld();
                }),
                "vsync", sol::property([](const Amara::World& world) { return world.vsync; }, &World::setVsync),
                "alwaysOnTop", sol::property([](const Amara::World& world) { return world.alwaysOnTop; }, &World::setAlwaysOnTop),
                "clickThrough", sol::property([](const Amara::World& world) { return world.clickThrough; }, &World::setClickThrough),
                "forcedClickThrough", sol::property([](const Amara::World& world) { return world.forcedClickThrough; }, &World::setForcedClickThrough),
                "windowTitle", sol::property([](const Amara::World& world) { return world.windowTitle; }, &World::setWindowTitle),
                "backgroundColor", sol::property([](const Amara::World& world) { return world.backgroundColor; }, [](Amara::World& world, sol::object value) {
                    world.backgroundColor = value;
                }),
                "windowMoved", sol::readonly(&World::windowMoved),
                "windowResized", sol::readonly(&World::windowResized),
                "windowFocused", sol::readonly(&World::windowFocused),
                "resizable", sol::property([](const Amara::World& world) { return world.resizable; }, [](Amara::World& world, bool value) {
                    world.resizable = value;
                    if (world.window) {
                        SDL_SetWindowResizable(world.window, value);
                    }
                }),
                "minimizeWindow", &World::minimizeWindow,
                "maximizeWindow", &World::maximizeWindow,
                "hideWindow", &World::hideWindow,
                "showWindow", &World::showWindow,
                "restoreWindow", &World::restoreWindow,
                "depth", sol::property(
                    [](const Amara::World& world) {
                        return world.depth;
                    },
                    [](Amara::World& world, float value) {
                        if (world.demiurge) {
                            debug_log("Note: Demiurgic presence. World Depth Overridden.");
                            debug_log("Control will be handed over in target builds.");
                        }
                        else world.depth = value;
                    }    
                )
            );

            sol::usertype<Node> node_type = lua["Node"];
            node_type["world"] = sol::property([](Amara::Node& e) {
                return e.world ? e.world->get_lua_object() : sol::nil;
            });
        }
    };

    void GameProps::breakWorld() {
        if (world) world->destroy();
        if (error_code == 0) error_code = 1;
    }
    void GameProps::breakWorld(int _error_code) {
        breakWorld();
        error_code = _error_code;
    }
}