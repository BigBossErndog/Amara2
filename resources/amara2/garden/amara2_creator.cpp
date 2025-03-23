namespace Amara {    
    class Creator: public Demiurge {
    public:
        sol::state lua;

        std::vector<World*> worlds;
        std::vector<World*> copy_worlds_list;

        std::vector<World*> new_worlds;

        MessageQueue messages;
        GarbageCollector garbageCollector;
        InputManager inputManager;

        Uint64 rec_tick = 0;
        Uint64 current_tick = 0;
        double frameTarget = 0;

        World* currentWorld = nullptr;

        Creator(): Demiurge() {
            demiurgic = false;

            Props::set_lua(lua);

            Props::messages = &messages;
            Props::garbageCollector = &garbageCollector;

            worlds.clear();
            
            lua_checkstack(lua.lua_state(), Props::lua_stack_size);

            lua.open_libraries(
                sol::lib::base,
                sol::lib::package,
                sol::lib::coroutine,
                sol::lib::string,
                sol::lib::os,
                sol::lib::math,
                sol::lib::table,
                sol::lib::debug,
                sol::lib::io,
                sol::lib::bit32,
                sol::lib::ffi,
                sol::lib::jit,
                sol::lib::utf8
            );

            bindLua();

            setup();

            lua["Game"] = &game;
            override_existence();
        }

        Creator(int argv, char** args): Creator() {
            if (argv > 1) {
                std::cout << "Arguments: ";
                for (int i = 1; i < argv; i++) {
                    std::cout << args[i];
                    if (i < argv-1) std::cout << ", ";
                    if (nlohmann::json::accept(args[i])) game.arguments.push_back(nlohmann::json::parse(args[i]));
                    else game.arguments.push_back(std::string(args[i]));
                }
                std::cout << std::endl;
            }
        }

        virtual World* createWorld(std::string key) override {
            World* new_world = factory.create(key)->as<World*>();

            if (new_world == nullptr) {
                debug_log("Error: World with \"", key, "\" has not been registered.");
                return nullptr;
            }

            worlds.push_back(new_world);
            new_worlds.push_back(new_world);

            new_world->init();

            return new_world;
        }
        virtual World* createWorld() override {
            return createWorld("World");
        }

        void cleanDestroyedWorlds() {
            World* world;
            for (auto it = worlds.begin(); it != worlds.end();) {
                world = *it;
                if (world->isDestroyed) {
                    if (world->demiurge) {
                        Amara::Demiurge* demiurge = world->demiurge;
                        delete demiurge;
                        world->demiurge = nullptr;
                    }
                    it = worlds.erase(it);
                    continue;
                }
                ++it;
            }
            new_worlds.clear();
        }

        void destroyAllWorlds() {
            for (auto w: worlds) {
                w->destroy();
            }
            worlds.clear();
        }

        void update_properties() {
            if (currentWorld && currentWorld->demiurge) {
                currentWorld->demiurge->override_existence();
            }
            else {
                override_existence();
            }
        }

        void makeDemiurgic(Amara::World* world, Amara::Demiurge* demiurge) {
            world->demiurge = demiurge;
        }

        void makeDemiurgic(Amara::World* world) {
            if (world->demiurge) return;
            makeDemiurgic(world, createDemiurge());
        }

        Amara::Demiurge* createDemiurge() {
            Amara::Demiurge* new_demiurge = new Demiurge();
            new_demiurge->setup();
            new_demiurge->true_creator = this;
            return new_demiurge;
        }

        void startCreation(std::string path) {
            if (!SDL_Init(SDL_INIT_VIDEO)) {
                debug_log("Error: SDL_Init failed: ", SDL_GetError());
            }

            rec_tick = SDL_GetPerformanceCounter();
            Uint64 freq = SDL_GetPerformanceFrequency();
            double frameTarget = 0;
            double elapsedTime = 0;

            scripts.run(path);
            game.hasQuit = Props::lua_exception_thrown;
            
            
            while (!game.hasQuit && worlds.size() != 0) { // Creation cannot exist without any worlds.
                inputManager.handleEvents(worlds, game.hasQuit);
                if (game.hasQuit) {
                    break;
                }

                if (!inputManager.logicBlocking) {
                    std::stable_sort(worlds.begin(), worlds.end(), sort_entities_by_depth());
                    
                    copy_worlds_list = worlds;
                    for (auto it = copy_worlds_list.begin(); it != copy_worlds_list.end(); it++) {
                        currentWorld = *it;
                        update_properties();

                        Props::lua_exception_thrown = false;
                        currentWorld->run(game.deltaTime);
                        if (currentWorld->exception_thrown) {
                            currentWorld->destroy();
                        } 
                        else if (currentWorld->window) {
                            currentWorld->draw();
                        }
                    }
                    cleanDestroyedWorlds();
                    currentWorld = nullptr;

                    if (game.targetFPS != -1) {
                        frameTarget = 1.0 / (double)game.targetFPS;
                        elapsedTime = (double)(SDL_GetPerformanceCounter() - rec_tick) / (double)freq;
                        if (elapsedTime < frameTarget) {
                            SDL_Delay((frameTarget - elapsedTime)*1000);
                        }
                    }
                    current_tick = SDL_GetPerformanceCounter();
                    Props::deltaTime = game.deltaTime = (double)(current_tick - rec_tick) / (double)freq;
                    game.fps = 1 / game.deltaTime;
                    rec_tick = current_tick;
                }
            }

            destroyAllWorlds();
            garbageCollector.clearImmediately();

            if (Props::gpuDevice) SDL_DestroyGPUDevice(Props::gpuDevice);
            SDL_Quit();
        }

        void bindLua() {
            bindLua_UtilityFunctions(lua);
            bindLua_LuaUtilityFunctions(lua);
            bindLua_Vectors(lua);
            bindLua_Shapes(lua);
            bindLua_Easing(lua);
            
            GameManager::bindLua(lua);

            FileManager::bindLua(lua);
            EntityFactory::bindLua(lua);
            ScriptFactory::bindLua(lua);

            Demiurge::bindLua(lua);

            lua.new_usertype<Creator>("Creator",
                sol::base_classes, sol::bases<Demiurge>(),
                "worlds", sol::readonly(&Creator::worlds),
                "new_worlds", sol::readonly(&Creator::new_worlds),
                "makeDemiurgic", sol::overload(
                    sol::resolve<void(Amara::World*, Amara::Demiurge*)>(&Creator::makeDemiurgic),
                    sol::resolve<void(Amara::World*, Amara::Demiurge*)>(&Creator::makeDemiurgic)
                ),
                "createDemiurge", &Creator::createDemiurge
            );
        }

        ~Creator() {}
    };

    World* Demiurge::createWorld(std::string key) {
        if (true_creator) return true_creator->createWorld(key);
        return nullptr;
    };
    World* Demiurge::createWorld() {
        if (true_creator) return true_creator->createWorld();
        return nullptr;
    };
}