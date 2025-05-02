namespace Amara {
    class Creator: public Demiurge {
    public:
        GameProps gameProps;

        std::vector<World*> copy_worlds_list;

        std::vector<World*> new_worlds;

        MessageQueue messages;
        GarbageCollector garbageCollector;
        EventHandler eventHandler;

        Uint64 rec_tick = 0;
        Uint64 current_tick = 0;
        double frameTarget = 0;

        World* currentWorld = nullptr;
        Demiurge* currentDemiurge = nullptr;

        Creator(): Demiurge() {
            demiurgic = false;

            garbageCollector.gameProps = &gameProps;

            gameProps.messages = &messages;
            gameProps.garbageCollector = &garbageCollector;
            
            worlds.clear();
            
            sol::state& lua = gameProps.lua;

            lua_checkstack(lua.lua_state(), gameProps.lua_stack_size);

            lua.open_libraries(
                sol::lib::base,
                sol::lib::package,
                sol::lib::string,
                sol::lib::math,
                sol::lib::table,
                sol::lib::debug,
                sol::lib::bit32,
                sol::lib::utf8
            );

            bindLua();

            setup(&gameProps);

            lua["Keyboard"] = &(eventHandler.keyboard);
            gameProps.keyboard = &(eventHandler.keyboard);

            override_existence();

            gameProps.world_list = &worlds;
        }

        Creator(int argv, char** args): Creator() {
            game.executable = system.getRelativePath(system.getFileName(std::string(args[0])));
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
            if (game.arguments.size() > 0) {
                for (auto it = game.arguments.begin(); it != game.arguments.end();) {
                    nlohmann::json& arg = *it;
                    if (arg.is_string()) {
                        if (String::equal(arg, "-context")) {
                            ++it;
                            if (it == game.arguments.end()) break;
                            nlohmann::json& path = *it;
                            if (path.is_string()) {
                                gameProps.context_path = path;
                                system.resetBasePath();
                            }
                        }
                    }
                    ++it;
                }
            }
        }

        virtual World* createWorld(sol::object config) override {
            std::string key = "World";

            if (config.is<std::string>()) key = config.as<std::string>();
            World* new_world = factory.create(key)->as<World*>();

            if (new_world == nullptr) {
                debug_log("Error: World with \"", key, "\" has not been registered.");
                return nullptr;
            }

            worlds.push_back(new_world);
            new_worlds.push_back(new_world);

            new_world->init();

            if (config.is<sol::table>()) {
                new_world->luaConfigure(config);
            }

            if (currentDemiurge && !currentDemiurge->paused) {
                new_world->demiurge = currentDemiurge;
                currentDemiurge->addWorld(new_world);
            }

            return new_world;
        }
        virtual World* createWorld() override {
            return createWorld(sol::make_object(gameProps.lua, std::string("World")));
        }

        void update_properties() {
            if (currentWorld && currentWorld->demiurge) {
                currentWorld->demiurge->override_existence();
            }
            else {
                override_existence();
                gameProps.system->setBasePath(base_dir_path);
                gameProps.lua["Creator"] = this;
            }
        }

        Amara::Demiurge* createDemiurge() {
            Amara::Demiurge* new_demiurge = new Demiurge();
            new_demiurge->setup(&gameProps);
            new_demiurge->true_creator = this;
            return new_demiurge;
        }

        void startDemiurgicUniverse() {
            destroyDemiurgicUniverse();

            // Causes all future created worlds to be isolated.
            currentDemiurge = createDemiurge();
        }
        void startDemiurgicUniverse(std::string path) {
            startDemiurgicUniverse();
            currentDemiurge->base_dir_path = path;
        }

        void destroyDemiurgicUniverse() {
            if (currentDemiurge) {
                currentDemiurge->destroy();
                delete currentDemiurge;
            }
            currentDemiurge = nullptr;
        }

        void pauseDemiurgicUniverse() {
            if (currentDemiurge) {
                currentDemiurge->paused = true;
            }
        }

        void resumeDemiurgicUniverse() {
            if (currentDemiurge) {
                currentDemiurge->paused = false;
            }
        }

        void cleanDestroyedWorlds() {
            World* world;
            for (auto it = worlds.begin(); it != worlds.end();) {
                world = *it;
                if (world->destroyed) {
                    it = worlds.erase(it);
                    continue;
                }
                if (world->parent) {
                    it = worlds.erase(it);
                    continue;
                }
                ++it;
            }
            new_worlds.clear();
        }

        void startCreation(std::string path) {
            if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
                debug_log("Error: SDL_Init failed: ", SDL_GetError());
            }

            rec_tick = SDL_GetPerformanceCounter();
            Uint64 freq = SDL_GetPerformanceFrequency();
            double frameTarget = 0;
            double elapsedTime = 0;

            scripts.run(path);
            game.hasQuit = gameProps.lua_exception_thrown;

            std::stable_sort(worlds.begin(), worlds.end(), sort_entities_by_depth());

            bool vsync = false;

            while (!game.hasQuit && worlds.size() != 0) { // Creation cannot exist without any worlds.
                eventHandler.handleEvents(worlds, game);

                if (game.hasQuit) {
                    break;
                }
                vsync = false;

                if (!eventHandler.logicBlocking) {
                    copy_worlds_list = worlds;
                    for (auto it = copy_worlds_list.begin(); it != copy_worlds_list.end(); it++) {
                        currentWorld = *it;
                        update_properties();

                        gameProps.lua_exception_thrown = false;

                        currentWorld->run(game.deltaTime);

                        if (currentWorld->exception_thrown) {
                            currentWorld->destroy();
                        }
                    }

                    cleanDestroyedWorlds();
                    std::stable_sort(worlds.begin(), worlds.end(), sort_entities_by_depth());

                    for (auto it = worlds.begin(); it != worlds.end(); it++) {
                        currentWorld = *it;
                        if (currentWorld->headless) continue;
                        update_properties();

                        currentWorld->prepareRenderer();
                        currentWorld->draw(gameProps.master_viewport);
                        if (currentWorld->vsync != 0) vsync = true;
                    }
                    for (Amara::World* world: worlds) {
                        world->presentRenderer();
                    }

                    currentWorld = nullptr;
                    if (game.targetFPS != 0 && !vsync) {
                        frameTarget = 1.0 / (double)game.targetFPS;
                        elapsedTime = (double)(SDL_GetPerformanceCounter() - rec_tick) / (double)freq;
                        if (elapsedTime < frameTarget) {
                            SDL_Delay((frameTarget - elapsedTime)*1000);
                        }
                    }
                    current_tick = SDL_GetPerformanceCounter();
                    gameProps.deltaTime = game.deltaTime = (double)(current_tick - rec_tick) / (double)freq;
                    game.fps = 1 / game.deltaTime;
                    rec_tick = current_tick;
                    
                    game.lifeTime += game.deltaTime;
                    if (currentDemiurge) {
                        currentDemiurge->game.lifeTime += game.lifeTime;
                    }
                }
            }

            destroy();
            cleanDestroyedWorlds();

            garbageCollector.clearImmediately();

            SDL_Quit();
        }

        void bindLua() {
            sol::state& lua = gameProps.lua;

            bindLua_Enums(lua);
            bindLua_LuaUtilityFunctions(lua);
            bindLua_Vectors(lua);
            bindLua_Shapes(lua);
            bindLua_Easing(lua);
            bindLua_Geometry(lua);

            Color::bindLua(lua);
            ShaderProgram::bindLua(lua);
            
            GameManager::bindLua(lua);
            ControlManager::bindLua(lua);
            SystemManager::bindLua(lua);
            ScriptFactory::bindLua(lua);
            ShaderManager::bindLua(lua);
            AnimationFactory::bindLua(lua);
            AssetManager::bindLua(lua);
            NodeFactory::bindLua(lua);
            
            Demiurge::bindLua(lua);

            lua.new_usertype<Creator>("Creator",
                sol::base_classes, sol::bases<Demiurge>(),
                "worlds", sol::readonly(&Creator::worlds),
                "new_worlds", sol::readonly(&Creator::new_worlds),
                "startDemiurgicUniverse", sol::overload(
                    sol::resolve<void(std::string)>( &Creator::startDemiurgicUniverse ),
                    sol::resolve<void()>( &Creator::startDemiurgicUniverse )
                ),
                "makePresenceKnown", &Creator::makePresenceKnown,
                "newDemiurgicUniverse", &Creator::newDemiurgicUniverse,
                "destroyDemiurgicUniverse", &Creator::destroyDemiurgicUniverse,
                "pauseDemuirgicUniverse", &Creator::pauseDemiurgicUniverse,
                "resumeDemiurgicUniverse", &Creator::resumeDemiurgicUniverse
            );
        }
    };

    World* Demiurge::createWorld(sol::object config) {
        if (true_creator) return true_creator->createWorld(config);
        return nullptr;
    };
    World* Demiurge::createWorld() {
        if (true_creator) return true_creator->createWorld();
        return nullptr;
    };
}