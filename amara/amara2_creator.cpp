namespace Amara {    
    class Creator {
    public:
        sol::state lua;

        std::deque<World> worlds;
        std::vector<nlohmann::json> arguments;

        Creator() {
            WorldProperties::set_lua(lua);
            
            lua_checkstack(lua.lua_state(), WorldProperties::lua_stack_size);

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

            lua["creation"] = this;
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
        Creator(ProgramArgs args): Creator(args.argv, args.args) {}

        void bindLua() {
            World::bindLua(lua);

            lua.new_usertype<Creator>("Creator",
                "arguments", sol::property([](const Creator& g) -> sol::object {
                    if (g.arguments.size() == 0) return sol::nil;
                    return json_to_lua(g.arguments);
                })
            );
        }

        World& create_world() {
            worlds.push_back(World());
            return worlds.back();
        }
    };
}