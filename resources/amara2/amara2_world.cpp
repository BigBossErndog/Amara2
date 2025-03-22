namespace Amara {
    class Demiurge;

    class World: public Entity {
    public:
        Amara::Demiurge* demiurge = nullptr;

        std::string base_dir_path;

        SDL_Window* window = nullptr;
        Uint32 windowID = 0;
        std::string windowTitle;

        int windowWidth = 640;
        int windowHeight = 360;

        int resizable = 0;

        bool initiated = false;
        bool create_window_on_start = !Props::integrate_new_windows;

        World(): Entity() {
            set_base_entity_id("World");
            world = this;
        }

        virtual void update_properties() override {
            Props::world = this;
            if (window != nullptr) {
                Props::current_window = window;
            }
            Entity::update_properties();
        }

        virtual void make_configurables() override {
            Amara::Entity::make_configurables();
            configurables["window"] = [this](nlohmann::json val) {
                bool resizeWindow = false;
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
                }
                if (resizeWindow && window != nullptr) {
                    SDL_SetWindowSize(window, windowWidth, windowHeight);
                }
            };
        }

        virtual void create() override {
            if (create_window_on_start && window == nullptr) {
                window = SDL_CreateWindow(
                    windowTitle.c_str(),
                    windowWidth, windowHeight,
                    resizable
                );
                windowID = SDL_GetWindowID(window);

                // if (Props::gpuDevice == nullptr) {
                //     Props::gpuDevice = SDL_CreateGPUDevice(
                //         SDL_GPU_SHADERFORMAT_SPIRV |
                //         SDL_GPU_SHADERFORMAT_DXBC |
                //         SDL_GPU_SHADERFORMAT_DXIL |
                //         SDL_GPU_SHADERFORMAT_DXIL |
                //         SDL_GPU_SHADERFORMAT_METALLIB,
                //         false, NULL
                //     );
                //     if (!Props::gpuDevice) {
                //         printf("Error: SDL_CreatedGPUDevice failed: %s\n", SDL_GetError());
                //     }
                // }

                if (Props::gpuDevice != nullptr && SDL_ClaimWindowForGPUDevice(Props::gpuDevice, window) != 0) {
                    debug_log("Error: Unable to associate window with gpu device.");
                }
            }
        }

        virtual void run(double deltaTime) override {
            if (!initiated) {
                create();
                initiated = true;
            }
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

        virtual void draw() override {
            if (!initiated) return;
            Amara::Entity::draw();
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