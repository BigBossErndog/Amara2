namespace Amara {
    class Demiurge;
    
    class World: public Node {
    public:
        Amara::Demiurge* demiurge = nullptr;
        
        AssetManager assets;
        ShaderManager shaders;

        std::string base_dir_path = Props::files->getBasePath();

        SDL_Window* window = nullptr;
        Uint32 windowID = 0;
        std::string windowTitle;
        
        bool create_window_on_start = false;

        float windowW = 640;
        float windowH = 360;
        Rectangle window_dim = { pos.x, pos.y, windowW, windowH };

        ScreenModeEnum screenMode = ScreenModeEnum::Windowed;

        #ifdef AMARA_OPENGL
            SDL_GLContext glContext;
        #endif
        SDL_Renderer* renderer = nullptr;
        SDL_GPUDevice* gpuDevice = nullptr;

        GraphicsEnum graphics;
        
        Amara::Color backgroundColor = Amara::Color::Transparent;

        Rectangle display;
        Uint32 displayID = 0;
        Uint32 rec_displayID = 0;

        Vector2 rec_pos = { -1, -1 };

        float virtualWidth = -1;
        float virtualHeight = -1;

        Rectangle viewport;
        
        int vsync = 0;
        bool headless = true;
        bool resizable = 0;
        bool transparent = false;
        bool alwaysOnTop = false;

        bool clickThrough = false;
        bool clickThroughState = false;

        bool exception_thrown = false;
        
        std::vector<std::string> entryScenes;
        bool created_entry_scenes = false;

        std::vector<Amara::GraphicsEnum> graphics_priority = Amara_Default_Graphics_Priority;

        GPUHandler gpuHandler;
        RenderBatch renderBatch;

        ShaderProgram* defaultShaderProgram = nullptr;

        World(): Node() {
            set_base_node_id("World");
            world = this;
            passOnPropsEnabled = false;
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
                Props::display = display;
                Props::displayID = displayID;
                
                Props::current_window = window;
                viewport = Rectangle( 0, 0, windowW, windowH );
                Props::master_viewport = viewport;

                int wx, wy;
                SDL_GetWindowPosition(window, &wx, &wy);
                float fwx = static_cast<int>(wx), fwy = static_cast<int>(wy);
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
                float fww = static_cast<int>(ww), fwh = static_cast<int>(wh);
                if (window_dim.w != windowW || window_dim.h != windowH) {
                    fww += windowW - window_dim.w;
                    fwh += windowH - window_dim.h;
                    SDL_SetWindowSize(window, fww, fwh);
                }
                windowW = fww;
                windowH = fwh;
                window_dim = Rectangle( pos.x, pos.y, windowW, windowH );
                Props::window_dim = window_dim;
            }
            else {
                viewport = Rectangle( pos.x, pos.y, windowW, windowH );
                display = Props::display;
            }
        }

        virtual void update_properties() override {
            Props::world = this;
            Props::assets = &assets;
            Props::shaders = &shaders;

            Props::lua()["Assets"] = &assets;
            Props::lua()["Shaders"] = &shaders;

            if (window) {
                #ifdef AMARA_OPENGL
                Props::glContext = NULL;
                #endif
                Props::renderer = nullptr;
                Props::gpuDevice = nullptr;

                #ifdef AMARA_OPENGL
                if (graphics == GraphicsEnum::OpenGL && glContext != NULL) {
                    Props::glContext = glContext;
                }
                #endif
                if (graphics == GraphicsEnum::Render2D && renderer != nullptr) {
                    Props::renderer = renderer;
                }
                if (gpuDevice != nullptr) {
                    Props::gpuDevice = gpuDevice;
                }
                Props::graphics = graphics;
                Props::gpuHandler = &gpuHandler;
                Props::renderBatch = &renderBatch;

                Props::transparent_window = transparent;
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
            window_data["screenMode"] = screenMode;
            window_data["graphics"] = graphics_to_string(graphics);
            window_data["backgroundColor"] = backgroundColor.toJSON();

            data["entryScenes"] = entryScenes;

            return data;
        }

        void configure_window(nlohmann::json config) {
            bool resizeWindow = false;
            headless = false;

            create_window_on_start = !Props::integrate_new_windows;

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
            if (json_has(config, "resizable")) {
                bool resizable = config["resizable"];
                if (window) SDL_SetWindowResizable(window, resizable);
            }
            if (json_is(config, "singleWindowApplication")) {
                Props::integrate_new_windows = true;
            }
            if (json_has(config, "headless")) {
                create_window_on_start = !config["headless"].get<bool>();
            }
            if (json_has(config, "windowTitle")) {
                windowTitle = config["windowTitle"];
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
                    Props::breakWorld();
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
                    if (string_equal(val, "adaptive")) {
                        vsync = -1;
                    }
                }
                if (renderer) SDL_SetRenderVSync(renderer, vsync);
            }
            if (resizeWindow && window != nullptr) {
                SDL_SetWindowSize(window, windowW, windowH);
            }
            if (json_has(config, "screenMode")) {
                if (demiurge) {
                    debug_log("Note: Demiurgic presence. Screen Mode Overridden: ", graphics_to_string(Props::graphics));
                    debug_log("Control will be handed over in target builds.");
                }
                else if (config["screenMode"].is_null()) {
                    debug_log("Error: Invalid screen mode setting.");
                    Props::breakWorld();
                }
                else {
                    setScreenMode(config["screenMode"]);
                }
            }
            if (json_has(config, "transparent")) {
                if (window == nullptr) transparent = config["transparent"];
            }
            if (json_has(config, "alwaysOnTop")) {
                setAlwaysOnTop(config["alwaysOnTop"]);
            }
            if (json_has(config, "clickThrough")) {
                clickThrough = config["clickThrough"];
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

            if (json_has(config, "entryScene")) {
                entryScenes.push_back(config["entryScene"]);
            }
            if (json_has(config, "entryScenes")) {
                nlohmann::json keys = config["entryScenes"];
                if (keys.is_string()) {
                    for (int i = 0; i < keys.size(); i++) {
                        entryScenes.push_back(keys[i]);
                    }
                }
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

        void setScreenMode(ScreenModeEnum _sm) {
            screenMode = _sm;

            if (window) {
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
                        Props::breakWorld();
                        break;
                }
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

        void setClickThroughState(bool enabled) {
            if (clickThroughState == enabled || window == nullptr) return;

            clickThroughState = enabled;

            if (enabled) {
                SDL_PropertiesID window_props = SDL_GetWindowProperties(window);
                if (!window_props) {
                    debug_log("Warning: Failed to get window properties: ", SDL_GetError());
                    return;
                }

                #ifdef _WIN32
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
                #endif
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

        bool create_graphics_window(int flags) {
            if (window != nullptr) return false;

            if (resizable) flags |= SDL_WINDOW_RESIZABLE;
            if (transparent) flags |= SDL_WINDOW_TRANSPARENT;
            if (alwaysOnTop) flags |= SDL_WINDOW_ALWAYS_ON_TOP;

            window = SDL_CreateWindow(
                windowTitle.c_str(),
                windowW, windowH,
                flags
            );
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
                            Props::renderer = renderer;
                            if (vsync != 0) SDL_SetRenderVSync(renderer, vsync);

                            renderer_created = true;
                            graphics = g;
                        }
                        break;
                    case Amara::GraphicsEnum::OpenGL:
                        #ifdef AMARA_OPENGL
                        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
                        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
                        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
                        
                        if (create_graphics_window(SDL_WINDOW_OPENGL)) {
                            glContext = SDL_GL_CreateContext(window);
                            renderer_created = true;
                            if (glContext == NULL) {
                                renderer_created = false;
                                debug_log("Error: Failed to create GL Context. ", SDL_GetError());
                            }
                            else {
                                try {
                                    if (!Props::glFunctionsLoaded) LoadOpenGLFunctions();
                                }
                                catch (...) {
                                    debug_log("Error: Failed to load OpenGL API.");
                                    SDL_GL_DestroyContext(glContext);
                                    if (window) SDL_DestroyWindow(window);
                                }
                            }
                            if (renderer_created) {
                                Props::glFunctionsLoaded = true;
                                Props::render_origin = this;
                                Props::glContext = glContext;

                                graphics = g;
                                Props::graphics = g;

                                glEnable(GL_BLEND);
                                glEnable(GL_TEXTURE_2D);

                                Props::renderBatch = &renderBatch;
                                renderBatch.init();

                                prepareGLShaders();
                                setShaderProgram("default");
                                Props::currentShaderProgram = shaderProgram;
                                Props::defaultShaderProgram = shaderProgram;
                                defaultShaderProgram = shaderProgram;
                                shaderProgram->applyShader();
                            }
                        }
                        #endif
                        break;
                    case Amara::GraphicsEnum::None:
                        graphics = g;
                        renderer_created = true;
                        break;
                    default:
                        if (create_gpu_device(g)) {
                            renderer_created = true;
                            Props::render_origin = this;
                            Props::gpuDevice = gpuDevice;
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

                Props::graphics = graphics;

                if (clickThrough) {
                    clickThrough = false;
                    setClickThrough(true);
                }
                setAlwaysOnTop(alwaysOnTop);

                debug_log("Info: ", *this, " rendering to window using ", graphics_to_string(graphics));
            }
            else if (graphics == GraphicsEnum::None && Props::current_window != nullptr) {
                pos.x = (Props::master_viewport.w - windowW) / 2.0f;
                pos.y = (Props::master_viewport.h - windowH) / 2.0f;
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
                Props::breakWorld();
                return;
            }
        }

        #ifdef AMARA_OPENGL
        void prepareGLShaders() {
            Props::shaders->compileGLShader("defaultVert", defaultVertexShader, ShaderTypeEnum::Vertex);
            Props::shaders->compileGLShader("defaultFrag", defaultFragmentShader, ShaderTypeEnum::Fragment);
            Props::shaders->createShaderProgram("default", {
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
                debug_log("Note: Demiurgic presence. Rendering Mode Overridden: ", graphics_to_string(Props::graphics));
                debug_log("Control will be handed over in target builds.");
            }
            
            update_properties();
            basePassOnProps();

            Amara::Node::preload();
        }

        virtual void run(double deltaTime) override {
            if (!base_dir_path.empty()) {
                Props::files->setBasePath(base_dir_path);
            }
            Amara::Node::run(deltaTime);

            if (actuated && !created_entry_scenes) {
                for (std::string key: entryScenes) {
                    createChild(key);        
                }
                created_entry_scenes = true;
            }

            if (window) Props::display = viewport;
            
            if (Props::lua_exception_thrown) {
                exception_thrown = true;
            }
        }

        void basePassOnProps() {
            reset_pass_on_props();

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
                Props::passOn.window_zoom = Vector2( zoom, zoom );
            }

            passOn = Props::passOn;
            passOnPropsEnabled = false;  
        }
        
        virtual void draw(const Rectangle& v) override {
            update_properties();
            basePassOnProps();

            passOn = Props::passOn;
            passOnPropsEnabled = false;  

            #ifdef AMARA_OPENGL
            if (graphics == GraphicsEnum::OpenGL && glContext != NULL) {
                Props::currentShaderProgram = nullptr;
            }
            #endif
            
            Amara::Node::draw(viewport);

            renderBatch.flush();
        }

        void prepareRenderer() {
            if (graphics == GraphicsEnum::Render2D && renderer != nullptr) {
                SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
                SDL_RenderClear(renderer);
                Props::master_viewport = viewport;
                Props::graphics = graphics;
            }
            #ifdef AMARA_OPENGL
            else if (graphics == GraphicsEnum::OpenGL && glContext != NULL) {
                Props::gpuHandler = &gpuHandler;
                Props::glContext = glContext;
                Props::renderBatch = &renderBatch;
                Props::master_viewport = viewport;
                Props::graphics = graphics;

                Props::renderBatch->newCycle();

                SDL_GL_MakeCurrent(window, glContext);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glClearColor(
                    backgroundColor.r / 255.0f,
                    backgroundColor.g / 255.0f,
                    backgroundColor.b / 255.0f,
                    backgroundColor.a / 255.0f
                );
                glClear(GL_COLOR_BUFFER_BIT);

                Props::defaultShaderProgram = defaultShaderProgram;
            }
            #endif
            else if (gpuDevice) {
                Props::gpuHandler = &gpuHandler;
                Props::master_viewport = viewport;
                Props::graphics = graphics;

                gpuHandler.commandBuffer = SDL_AcquireGPUCommandBuffer(Props::gpuDevice);
                if (gpuHandler.commandBuffer == NULL) {
                    debug_log("Error: AcquireGPUCommandBuffer failed: ", SDL_GetError());
                    Props::breakWorld();
                }
                if (!SDL_WaitAndAcquireGPUSwapchainTexture(gpuHandler.commandBuffer, Props::current_window, &gpuHandler.swapChainTexture, NULL, NULL)) {
                    debug_log("Error: WaitAndAcquireGPUSwapchainTexture failed: ", SDL_GetError());
                    Props::breakWorld();
                }
            }
        }

        void presentRenderer() {
            if (graphics == GraphicsEnum::Render2D && renderer) {
                SDL_RenderPresent(renderer);
            }
            #ifdef AMARA_OPENGL
            else if (graphics == GraphicsEnum::OpenGL && glContext != NULL) {
                SDL_GL_SwapWindow(window);
            }
            #endif
            else if (gpuDevice) {
                Props::gpuHandler = &gpuHandler;
                SDL_SubmitGPUCommandBuffer(gpuHandler.commandBuffer);
            }
        }

        virtual void destroy() override {
            Amara::Node::destroy();

            assets.clear();
            shaders.clear();
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
        }

        void destroyDemiurge();
        
        static void bindLua(sol::state& lua) {
            lua.new_usertype<World>("World",
                sol::base_classes, sol::bases<Node>(),
                "w", &World::windowW,
                "h", &World::windowH,
                "vw", &World::virtualWidth,
                "vh", &World::virtualHeight,
                "assets", &World::assets,
                "base_dir_path", sol::readonly(&World::base_dir_path),
                "display", sol::readonly(&World::display),
                "displayID", sol::readonly(&World::displayID),
                "graphics", sol::readonly(&World::graphics),
                "centerWindow", &World::centerWindow,
                "resizeWindow", &World::resizeWindow,
                "fitToDisplay", &World::fitToDisplay,
                "screenMode", sol::readonly(&World::screenMode),
                "setScreenMode", &World::setScreenMode,
                "transparent", sol::readonly(&World::transparent),
                "alwaysOnTop", sol::readonly(&World::alwaysOnTop),
                "setAlwaysOnTop", &World::setAlwaysOnTop,
                "clickThrough", sol::readonly(&World::clickThrough),
                "setClickThrough", &World::setClickThrough,
                "windowTitle", sol::readonly(&World::windowTitle),
                "setWindowTitle", &World::setWindowTitle,
                "backgroundColor", &World::backgroundColor
            );

            sol::usertype<Node> node_type = lua["Node"];
            node_type["world"] = sol::readonly(&Node::world);
        }
    };

    void Props::breakWorld() {
        if (Props::world) {
            Props::world->destroy();
        }
    }
}