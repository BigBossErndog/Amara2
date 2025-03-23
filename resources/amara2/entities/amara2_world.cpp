namespace Amara {
    class Demiurge;

    class World: public Entity {
    public:
        Amara::Demiurge* demiurge = nullptr;

        std::string base_dir_path;

        SDL_Window* window = nullptr;
        Uint32 windowID = 0;
        std::string windowTitle;

        SDL_Renderer* renderer = nullptr;
        Rectangle display;

        Vector2 rec_pos = pos;

        float windowWidth = 640;
        float windowHeight = 360;
        Rectangle window_dim = { pos.x, pos.y, windowWidth, windowHeight };

        int resizable = 0;
        int vsync = 0;

        bool create_window_on_start = false;
        int fullscreen_mode = 0;

        bool headless = true;

        bool exception_thrown = false;

        std::vector<Amara::GraphicsEnum> graphics_priority = {
            GraphicsEnum::OpenGL,
            GraphicsEnum::Render2D
        };

        World(): Entity() {
            set_base_entity_id("World");
            world = this;
        }

        virtual void update_properties() override {
            Props::world = this;
            Props::lua()["World"] = get_lua_object();

            if (window != nullptr) {
                SDL_DisplayID displayID = SDL_GetDisplayForWindow(window);
                if (displayID != Props::displayID) {
                    Props::displayID = displayID;

                    SDL_Rect displayBounds;
                    if (SDL_GetDisplayUsableBounds(displayID, &displayBounds)) {
                        display = {
                            static_cast<float>(displayBounds.x), 
                            static_cast<float>(displayBounds.y),
                            static_cast<float>(displayBounds.w), 
                            static_cast<float>(displayBounds.h)
                        };
                        Props::display = display;
                    }
                }
                
                Props::current_window = window;
                Props::viewport = { 0, 0, windowWidth, windowHeight };
                Props::master_viewport = Props::viewport;

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
                Props::viewport = { pos.x, pos.y, windowWidth, windowHeight };
            }
            if (renderer != nullptr) {
                Props::renderer = renderer;
            }

            Entity::update_properties();
        }

        virtual void make_configurables() override {
            Amara::Entity::make_configurables();
            configurables["window"] = [this](nlohmann::json val) {
                bool resizeWindow = false;
                headless = false;

                create_window_on_start = !Props::integrate_new_windows;

                for (auto it = val.begin(); it != val.end(); it++) {
                    if (string_equal(it.key(), "width")) {
                        windowWidth = it.value();
                        resizeWindow = true;
                        continue;
                    }
                    if (string_equal(it.key(), "height")) {
                        windowHeight = it.value();
                        resizeWindow = true;
                        continue;
                    }
                    if (string_equal(it.key(), "resizable")) {
                        bool r = it.value();
                        if (r) resizable = SDL_WINDOW_RESIZABLE;
                        else resizable = 0;
                        if (window) SDL_SetWindowResizable(window, r); 
                        continue;
                    }
                    if (string_equal(it.key(), "singleWindowApplication")) {
                        Props::integrate_new_windows = true;
                        continue;
                    }
                    if (string_equal(it.key(), "headless")) {
                        create_window_on_start = !it.value().get<bool>();
                        continue;
                    }
                    if (string_equal(it.key(), "windowTitle")) {
                        windowTitle = it.value();
                        if (window) SDL_SetWindowTitle(window, windowTitle.c_str());
                    }
                    if (string_equal(it.key(), "graphics")) {
                        if (it.value().is_array()) {
                            nlohmann::json list = it.value();
                            graphics_priority.clear();
                            for (int i = 0; i < list.size(); i++) {
                                graphics_priority.push_back(it.value());
                            }
                        }
                        else if (it.value().is_number()) {
                            graphics_priority = { it.value() };
                        }
                    }
                    if (string_equal(it.key(), "vsync")) {
                        if (it.value().is_boolean()) {
                            vsync = (it.value()) ? 1 : 0;
                        }
                        else if (it.value().is_number()) {
                            vsync = it.value();
                        }
                        else if (it.value().is_string()) {
                            if (string_equal(it.value(), "adaptive")) {
                                vsync = -1;
                            }
                        }
                        if (renderer) SDL_SetRenderVSync(renderer, vsync);
                    }
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

            Props::gpuDevice = SDL_CreateGPUDevice(
                shaderFlags,
                false,
                NULL
            );
            if (!Props::gpuDevice) {
                printf("Error: SDL_CreatedGPUDevice failed: %s\n", SDL_GetError());
            }
            else if (SDL_ClaimWindowForGPUDevice(Props::gpuDevice, window) != 0) {
                debug_log("Error: Unable to associate window with gpu device.");
                Props::gpuDevice = NULL;
            }
        }

        virtual void preload() override {
            if (create_window_on_start && window == nullptr) {
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
                                if (Props::graphics == GraphicsEnum::None) {
                                    Props::graphics = g;
                                }
                                renderer_created = true;
                            }
                            break;
                        case Amara::GraphicsEnum::OpenGL:
                            if (Props::graphics == g) {
                                renderer_created = true;
                            }
                            else {
                                create_graphics_window(SDL_WINDOW_OPENGL);
                                Props::glContext = SDL_GL_CreateContext(window);
                                if (Props::glContext == NULL) {
                                    debug_log("Error: Failed to create GL Context. ", SDL_GetError());
                                }
                                else {
                                    Props::graphics = g;
                                    renderer_created = true;
                                    Props::render_origin = this;
                                }
                            }
                            break;
                        case Amara::GraphicsEnum::None:
                            break;
                        default:
                            if (Props::graphics == g || create_gpu_device(g)) {
                                Props::graphics = g;
                                renderer_created = true;
                                Props::render_origin = this;
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
                }
            }
            update_properties();

            Amara::Entity::preload();
        }

        virtual void run(double deltaTime) override {
            if (!base_dir_path.empty()) {
                Props::files->setBasePath(base_dir_path);
            }
            Amara::Entity::run(deltaTime);
            if (Props::lua_exception_thrown) {
                exception_thrown = true;
            }
        }

        virtual void destroy() override {
            if (window != nullptr) {
                SDL_DestroyWindow(window);
                window = nullptr;
            }
            Amara::Entity::destroy();
        }
        
        static void bindLua(sol::state& lua) {
            lua.new_usertype<World>("World",
                sol::base_classes, sol::bases<Entity>(),
                "w", sol::property(
                    [](World& w, float _w) { w.windowWidth = _w; },
                    [](World& w) { return w.windowWidth; }
                ),
                "h", sol::property(
                    [](World& w, float _h) { w.windowHeight = _h; },
                    [](World& w) { return w.windowHeight; }
                ),
                "base_dir_path", sol::readonly(&World::base_dir_path),
                "display", sol::readonly(&World::display)
            );

            sol::usertype<Entity> entity_type = lua["Entity"];
            entity_type["world"] = sol::readonly(&Entity::world);
        }
    };
}