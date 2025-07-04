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

        std::vector<std::string> starting_scripts;

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

            bind_lua();

            setup(&gameProps);

            lua["Keyboard"] = &(eventHandler.keyboard);
            gameProps.keyboard = &(eventHandler.keyboard);

            lua["Gamepads"] = &(eventHandler.gamepads);
            gameProps.gamepads = &(eventHandler.gamepads);

            override_existence();

            gameProps.world_list = &worlds;
        }

        Creator(int argv, char** args): Creator() {
            game.executable = system.getRelativePath(system.getFileName(std::string(args[0])));
            if (argv > 1) {
                for (int i = 1; i < argv; i++) {
                    if (nlohmann::json::accept(args[i])) game.arguments.push_back(nlohmann::json::parse(args[i]));
                    else game.arguments.push_back(std::string(args[i]));
                }
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
                        #ifndef AMARA_DISABLE_EXTERNAL_SCRIPTS
                        if (String::equal(arg, "-script")) {
                            ++it;
                            if (it == game.arguments.end()) break;
                            nlohmann::json& path = *it;
                            if (path.is_string()) {
                                starting_scripts.push_back(path);
                            }
                        }
                        if (String::equal(arg, "-display")) {
                            ++it;
                            if (it == game.arguments.end()) break;
                            nlohmann::json& path = *it;
                            if (path.is_number()) {
                                gameProps.targetDisplayID = path;
                            }
                        }
                        #endif
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
                fatal_error("Error: World with \"", key, "\" has not been registered.");
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

            new_world->preload();
            if (!new_world->destroyed) new_world->create();
            new_world->actuated = true;

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

        virtual void override_existence() override {
            gameProps.lua["Creator"] = this;
            gameProps.lua["Game"] = &game;
            gameProps.lua["System"] = &system;
            gameProps.lua["Nodes"] = &factory;
            gameProps.lua["Scripts"] = &scripts;
            gameProps.lua["Controls"] = &controls;
            
            gameProps.system = &system;
            gameProps.factory = &factory;
            gameProps.scripts = &scripts;
            gameProps.controls = &controls;

            if (!base_dir_path.empty()) {
                gameProps.system->setBasePath(base_dir_path);
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

        int startCreation(std::string path) {
            if (starting_scripts.size() == 0) {
                starting_scripts.push_back(path);
            }
            return startCreation();
        }

        int startCreation() {
            if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK)) {
                debug_log("Error: SDL_Init failed: ", SDL_GetError());
            }

            eventHandler.init(&gameProps);

            #ifndef AMARA_DISABLE_EXTERNAL_SCRIPTS
            for (auto it = starting_scripts.begin(); it != starting_scripts.end(); it++) {
                scripts.run(*it);
            }
            #endif

            game.hasQuit = gameProps.lua_exception_thrown || gameProps.error_code != 0;

            cleanDestroyedWorlds();
            std::stable_sort(worlds.begin(), worlds.end(), sort_entities_by_depth());
            
            bool vsync = false;

            rec_tick = SDL_GetPerformanceCounter();
            Uint64 freq = SDL_GetPerformanceFrequency();
            double frameTarget = 0;
            double elapsedTime = 0;

            while (!game.hasQuit && worlds.size() != 0) { // Creation cannot exist without any worlds.
                messages.update();
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

            if (gameProps.lua_exception_thrown) return 1;
            return gameProps.error_code;
        }

        void bind_lua() {
            sol::state& lua = gameProps.lua;

            bind_lua_Enums(lua);
            bind_lua_LuaUtilityFunctions(lua);
            bind_lua_Vectors(lua);
            bind_lua_Shapes(lua);
            bind_lua_Easing(lua);
            bind_lua_Geometry(lua);

            Color::bind_lua(lua);
            ShaderProgram::bind_lua(lua);

            MessageBox::bind_lua(lua);
            NodeInput::bind_lua(lua);
            FunctionManager::bind_lua(lua);
            
            GameManager::bind_lua(lua);
            ControlManager::bind_lua(lua);
            SystemManager::bind_lua(lua);
            ScriptFactory::bind_lua(lua);
            ShaderManager::bind_lua(lua);
            AnimationFactory::bind_lua(lua);
            AssetManager::bind_lua(lua);
            NodeFactory::bind_lua(lua);
            
            Demiurge::bind_lua(lua);

            lua.new_usertype<Creator>("CreatorClass",
                sol::base_classes, sol::bases<Demiurge>(),
                "worlds", sol::property([](Creator& self) { return sol::as_table(self.worlds); }),
                "new_worlds", sol::property([](Creator& self) { return sol::as_table(self.new_worlds); }),
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