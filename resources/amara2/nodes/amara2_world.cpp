namespace Amara {
    class Demiurge;

    class World: public Node {
    public:
        Amara::Demiurge* demiurge = nullptr;

        std::string base_dir_path;

        SDL_Window* window = nullptr;
        Uint32 windowID = 0;
        std::string windowTitle;

        bool create_window_on_start = false;
        WindowEnum fullscreen_mode = Amara::WindowEnum::Windowed;

        float windowWidth = 640;
        float windowHeight = 360;
        Rectangle window_dim = { pos.x, pos.y, windowWidth, windowHeight };

        SDL_GLContext glContext;
        SDL_Renderer* renderer = nullptr;
        SDL_GPUDevice* gpuDevice = nullptr;

        GraphicsEnum graphics;

        Rectangle display;
        Uint32 displayID = 0;
        Uint32 rec_displayID = 0;

        Vector2 rec_pos = { -1, -1 };

        float virtualWidth = -1;
        float virtualHeight = -1;

        Rectangle viewport;

        int resizable = 0;
        int vsync = 0;
        bool headless = true;

        bool exception_thrown = false;

        std::vector<Amara::GraphicsEnum> graphics_priority = Amara_Default_Graphics_Priority;

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
                        display = {
                            static_cast<float>(displayBounds.x), 
                            static_cast<float>(displayBounds.y),
                            static_cast<float>(displayBounds.w), 
                            static_cast<float>(displayBounds.h)
                        };
                    }
                }
                Props::display = display;
                Props::displayID = displayID;
                
                Props::current_window = window;
                viewport = { 0, 0, windowWidth, windowHeight };
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
                if (window_dim.w != windowWidth || window_dim.h != windowHeight) {
                    fww += windowWidth - window_dim.w;
                    fwh += windowHeight - window_dim.h;
                    SDL_SetWindowSize(window, fww, fwh);
                }
                windowWidth = fww;
                windowHeight = fwh;
                window_dim = { pos.x, pos.y, windowWidth, windowHeight };
            }
            else {
                viewport = { pos.x, pos.y, windowWidth, windowHeight };
                display = Props::display;
            }
        }

        virtual void update_properties() override {
            Props::world = this;
            Props::lua()["World"] = get_lua_object();

            if (window) {
                Props::glContext = NULL;
                Props::renderer = nullptr;
                Props::gpuDevice = nullptr;
                if (graphics == GraphicsEnum::OpenGL && glContext != NULL) {
                    Props::glContext = glContext;
                }
                if (graphics == GraphicsEnum::Render2D && renderer != nullptr) {
                    Props::renderer = renderer;
                }
                if (gpuDevice != nullptr) {
                    Props::gpuDevice = gpuDevice;
                }
            }
            update_window();
            
            Node::update_properties();
        }

        virtual void make_configurables() override {
            Amara::Node::make_configurables();
            configurables["window"] = [this](nlohmann::json config) {
                bool resizeWindow = false;
                headless = false;

                create_window_on_start = !Props::integrate_new_windows;

                if (json_has(config, "width")) {
                    windowWidth = config["width"];
                    resizeWindow = true;
                }
                if (json_has(config, "height")) {
                    windowHeight = config["height"];
                    resizeWindow = true;
                }
                if (json_has(config, "resizable")) {
                    bool r = config["resizable"];
                    if (r) resizable = SDL_WINDOW_RESIZABLE;
                    else resizable = 0;
                    if (window) SDL_SetWindowResizable(window, r);
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
                    SDL_SetWindowSize(window, windowWidth, windowHeight);
                }
            };
        }
        
        void setup_new_window() {
            if (window == nullptr) return;

            windowID = SDL_GetWindowID(window);
            Props::current_window = window;
        }

        void create_graphics_window(int flags) {
            if (window != nullptr) return;
            window = SDL_CreateWindow(
                windowTitle.c_str(),
                windowWidth, windowHeight,
                resizable | flags
            );
            setup_new_window();
        }

        void create_graphics_window() {
            create_graphics_window(0);
        }

        bool create_gpu_device(Amara::GraphicsEnum g) {
            create_graphics_window();
            int shaderFlags = 0;

            switch (g) {
                case GraphicsEnum::Vulkan:
                    shaderFlags |= SDL_GPU_SHADERFORMAT_SPIRV;
                    break;
            }

            gpuDevice = SDL_CreateGPUDevice(
                shaderFlags,
                false,
                NULL
            );
            if (!gpuDevice) {
                printf("Error: SDL_CreatedGPUDevice failed: %s\n", SDL_GetError());
            }
            else if (SDL_ClaimWindowForGPUDevice(gpuDevice, window) != 0) {
                debug_log("Error: Unable to associate window with gpu device.");
                SDL_DestroyGPUDevice(gpuDevice);
                gpuDevice = NULL;
            }
        }

        void createWindowRenderer() {
            bool renderer_created = false;
            for (GraphicsEnum g: graphics_priority) {
                if (renderer_created) break;
                switch (g) {
                    case Amara::GraphicsEnum::Render2D:
                        if (window == nullptr) {
                            if (!SDL_CreateWindowAndRenderer(
                                windowTitle.c_str(),
                                windowWidth, windowHeight,
                                resizable,
                                &window,
                                &renderer
                            )) {
                                if (renderer) SDL_DestroyRenderer(renderer);
                                if (window) SDL_DestroyWindow(window);
                                window = nullptr;
                                renderer = nullptr;
                            }
                            windowID = SDL_GetWindowID(window);
                            Props::current_window = window;
                        }
                        else {
                            renderer = SDL_CreateRenderer(window, NULL);
                        }
                        if (window == nullptr || renderer == nullptr) {
                            debug_log("Error: Failed to create 2D Renderer. ", SDL_GetError());
                        }
                        else {
                            Props::renderer = renderer;
                            renderer_created = true;
                            graphics = g;
                        }
                        break;
                    case Amara::GraphicsEnum::OpenGL:
                        create_graphics_window(SDL_WINDOW_OPENGL);
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
                        }
                        break;
                    case Amara::GraphicsEnum::None:
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
            if (window == nullptr) {
                create_graphics_window();
            }
            if (window) {
                int wx, wy;
                SDL_GetWindowPosition(window, &wx, &wy);
                pos.x = static_cast<int>(wx);
                pos.y = static_cast<int>(wy);
                rec_pos = pos;

                Props::graphics = graphics;

                debug_log("Info: ", *this, " rendering to window using ", graphics_to_string(graphics));
            }
            else {
                pos.x = (Props::master_viewport.w - windowWidth) / 2.0f;
                pos.y = (Props::master_viewport.h - windowHeight) / 2.0f;
            }
        }

        virtual void preload() override {
            if (create_window_on_start) {
                if (window == nullptr) {
                    createWindowRenderer();
                }
            }
            else if (demiurge) {
                debug_log("Note: Demiurgic presence. Rendering Mode Overridden: ", graphics_to_string(Props::graphics));
                debug_log("Control will be handed over in target builds.");
            }
            update_properties();

            Amara::Node::preload();
        }

        virtual void run(double deltaTime) override {
            if (!base_dir_path.empty()) {
                Props::files->setBasePath(base_dir_path);
            }
            Amara::Node::run(deltaTime);
            if (window) Props::display = viewport;
            
            if (Props::lua_exception_thrown) {
                exception_thrown = true;
            }
        }

        virtual void draw(const Rectangle& v) {
            update_properties();

            if (window) {
                Props::graphics = graphics;
            }

            if (virtualWidth == -1 || virtualHeight == -1) {
                float viewport_factor = viewport.w / viewport.h;
                float virtual_factor = virtualWidth / virtualHeight;
                float zoom;
                if (viewport_factor > virtual_factor) {
                    float zoom = viewport.h / virtualHeight;
                }
                else {
                    float zoom = viewport.w / virtualWidth;
                }
                Props::passOn.zoom = { 
                    Props::passOn.zoom.x * zoom, 
                    Props::passOn.zoom.y * zoom
                };
            }
            Props::passOn.anchor = { 
                static_cast<float>(viewport.w/2.0), 
                static_cast<float>(viewport.h/2.0), 
                0 
            };
            Props::passOn.rotation = 0;
            Props::passOn.scroll = { 0, 0 };
            Props::passOn.zoom = { 1, 1 };
            Props::passOn.scale = { 1, 1 };

            passOn = Props::passOn;

            if (renderer) {
                SDL_RenderClear(renderer);
            }    
            
            Amara::Node::draw(viewport);

            if (Props::renderer) {
                SDL_RenderPresent(Props::renderer);
            }
        }

        virtual void destroy() override {
            Amara::Node::destroy();
            if (glContext != NULL) {
                SDL_GL_DestroyContext(glContext);
                glContext = NULL;
            }
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
        
        static void bindLua(sol::state& lua) {
            lua.new_usertype<World>("World",
                sol::base_classes, sol::bases<Node>(),
                "w", &World::windowWidth,
                "h", &World::windowHeight,
                "vw", &World::virtualWidth,
                "vh", &World::virtualHeight,
                "base_dir_path", sol::readonly(&World::base_dir_path),
                "display", sol::readonly(&World::display),
                "displayID", sol::readonly(&World::displayID),
                "graphics", sol::readonly(&World::graphics)
            );

            sol::usertype<Node> node_type = lua["Node"];
            node_type["world"] = sol::readonly(&Node::world);
        }
    };
}