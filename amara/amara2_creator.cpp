namespace Amara {    
    class Creator {
    public:
        sol::state lua;

        std::deque<World*> worlds;
        std::vector<nlohmann::json> arguments;

        GameManager game;

        FileManager files;
        ScriptFactory scripts;
        EntityFactory factory;
        MessageQueue messages;

        Creator() {
            Properties::set_lua(lua);
            Properties::game = &game;
            Properties::files = &files;
            Properties::factory = &factory;
            Properties::scripts = &scripts;
            Properties::messages = &messages;
            
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

            lua["creator"] = this;
            lua["game"] = &game;
            lua["files"] = &files;
            lua["factory"] = &factory;
            lua["scripts"] = &scripts;
        }

        Creator(int argv, char** args): Creator() {
            if (argv > 1) {
                std::cout << "Arguments: ";
                for (int i = 1; i < argv; i++) {
                    std::cout << args[i];
                    if (i < argv-1) std::cout << ", ";
                    if (nlohmann::json::accept(args[i])) arguments.push_back(nlohmann::json::parse(args[i]));
                    else arguments.push_back(std::string(args[i]));
                }
                std::cout << std::endl;
            }
        }

        void bindLua() {
            bindLuaUtilityFunctions(lua);
            bindLuaGeometry(lua);
            
            GameManager::bindLua(lua);

            FileManager::bindLua(lua);

            World::bindLua(lua);

            lua.new_usertype<Creator>("Creator",
                "createWorld", [this](Amara::Creator& c) -> sol::object {
                    return c.createWorld().make_lua_object();
                },
                "arguments", sol::property([](const Creator& g) -> sol::object {
                    if (g.arguments.size() == 0) return sol::nil;
                    return json_to_lua(g.arguments);
                })
            );
        }

        sol::object init(std::string path) {
            return files.run(path);
        }

        World& createWorld() {
            World* new_world = factory.create("World")->as<World*>();
            worlds.push_back(new_world);
            return *new_world;
        }
    };
}