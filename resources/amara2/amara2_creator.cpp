namespace Amara {    
    class Creator {
    public:
        sol::state lua;

        std::deque<World*> worlds;

        GameManager game;

        FileManager files;
        ScriptFactory scripts;
        EntityFactory factory;
        MessageQueue messages;

        Uint64 rec_tick = 0;
        Uint64 current_tick = 0;
        double frameTarget = 0;

        World* currentWorld = nullptr;

        Creator() {
            Properties::set_lua(lua);
            Properties::game = &game;
            Properties::files = &files;
            Properties::factory = &factory;
            Properties::scripts = &scripts;
            Properties::messages = &messages;

            worlds.clear();
            
            lua_checkstack(lua.lua_state(), Properties::lua_stack_size);

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

            lua.set_panic([](lua_State* L) -> int {
                return 0;
            });

            factory.prepareEntities();
            factory.registerEntity<World>("World");

            lua["Creator"] = this;
            lua["Game"] = &game;
            lua["Files"] = &files;
            lua["Factory"] = &factory;
            lua["Scripts"] = &scripts;
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

        void bindLua() {
            bindLua_UtilityFunctions(lua);
            bindLua_Vectors(lua);
            bindLua_Shapes(lua);
            bindLua_Easing(lua);
            
            GameManager::bindLua(lua);

            FileManager::bindLua(lua);

            World::bindLua(lua);

            lua.new_usertype<Creator>("Creator",
                "createWorld", [this](Amara::Creator& c, sol::object key) -> sol::object {
                    World* world = nullptr;
                    if (key.is<std::string>()) {
                        world = c.createWorld(key.as<std::string>());
                    }
                    else world = c.createWorld();
                    if (world) return world->get_lua_object();
                    return sol::nil;
                }
            );
        }

        World* createWorld(std::string key) {
            World* new_world = factory.create(key)->as<World*>();

            if (new_world == nullptr) {
                debug_log("Error: World with \"", key, "\" has not been registered.");
                return nullptr;
            }

            worlds.push_back(new_world);

            new_world->init();

            return new_world;
        }
        World* createWorld() {
            return createWorld("World");
        }

        void cleanDestroyedWorlds() {
            World* world;
            for (auto it = worlds.begin(); it != worlds.end();) {
                world = *it;
                if (world->isDestroyed) {
                    it = worlds.erase(it);
                    continue;
                }
                ++it;
            } 
        }

        void startCreation(std::string path) {
            SDL_Init(SDL_INIT_VIDEO);

            files.run(path);

            rec_tick = SDL_GetPerformanceCounter();
            Uint64 freq = SDL_GetPerformanceFrequency();

            while (worlds.size() != 0) { // Creation cannot exist without any worlds.
                
                for (auto it = worlds.begin(); it != worlds.end(); it++) {
                    currentWorld = *it;
                    currentWorld->run(game.deltaTime);
                    currentWorld->draw();
                }
                cleanDestroyedWorlds();
                currentWorld = nullptr;

                if (game.targetFPS != -1) {
                    double frameTarget = 1.0 / (double)game.targetFPS;
                    while ((double)(SDL_GetPerformanceCounter() - rec_tick) / (double)freq < frameTarget) {
                        SDL_Delay(1);
                    }
                }
                current_tick = SDL_GetPerformanceCounter();
                Properties::deltaTime = game.deltaTime = (double)(current_tick - rec_tick) / (double)freq;
                game.fps = 1 / ((double)(current_tick - rec_tick) / (double)freq);
                rec_tick = current_tick;
            }

            SDL_Quit();
        }
    };
}