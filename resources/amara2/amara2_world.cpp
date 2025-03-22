namespace Amara {
    class Demiurge;

    class World: public Entity {
    public:
        Amara::Demiurge* demiurge = nullptr;

        std::string base_dir_path;

        SDL_Window* window = nullptr;
        Uint32 windowID = 0;
        std::string windowTitle;

        Vector2 rec_pos = pos;

        float windowWidth = 640;
        float windowHeight = 360;

        int resizable = 0;
        int vsync = 0;

        bool create_window_on_start = false;
        int fullscreen_mode = 0;

        SDL_Renderer* renderer = nullptr;

        std::vector<Amara::Graphics> graphics_priority = {
            Graphics::OpenGL,
            Graphics::Render2D
        };

        World(): Entity() {
            set_base_entity_id("World");
            world = this;
        }

        virtual void update_properties() override {
            Props::world = this;

            if (window != nullptr) {
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

            Props::displayID = SDL_GetDisplayForWindow(window);
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

        bool create_gpu_device(Amara::Graphics g) {
            create_graphics_window();
            Props::gpuDevice = SDL_CreateGPUDevice(
                SDL_GPU_SHADERFORMAT_SPIRV,
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

        virtual void create() override {
            if (create_window_on_start && window == nullptr) {
                bool renderer_created = false;
                for (Graphics g: graphics_priority) {
                    if (renderer_created) break;
                    switch (g) {
                        case Amara::Graphics::Render2D:
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
                                if (Props::graphics == Graphics::None) {
                                    Props::graphics = g;
                                }
                                renderer_created = true;
                            }
                            break;
                        case Amara::Graphics::OpenGL:
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
                        case Amara::Graphics::None:
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
            }
        }

        virtual void run(double deltaTime) override {
            if (!base_dir_path.empty()) {
                Props::files->setBasePath(base_dir_path);
            }
            Amara::Entity::run(deltaTime);
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
                sol::constructors<World()>(),
                sol::base_classes, sol::bases<Amara::Entity>(),
                "base_dir_path", &World::base_dir_path
            );

            sol::usertype<Entity> entity_type = lua["Entity"];
            entity_type["world"] = sol::readonly(&Entity::world);
        }
    };
}