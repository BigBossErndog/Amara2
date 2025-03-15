namespace Amara {
    class Game {
    public:
        sol::state lua;

        SceneManager scenes;
        EntityFactory factory;
        ScriptFactory scripts;

        FileManager files;
        MessageQueue messages;

        sol::table props;
        sol::object asLuaObject;

        std::vector<nlohmann::json> arguments;

        Game() {
            prepare();
        }

        Game(int argv, char** args): Game() {
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

        Game(nlohmann::json config): Game() {
            if (config.is_string()) {
                std::string path = config.get<std::string>();
                if (string_endsWith(path, ".json")) configure(files.readJSON(path));
                else if (string_endsWith(path, ".lua") || string_endsWith(path, ".luac")) scripts.run(path);
            }
            else if (config.is_object()) configure(config);
        }   
        Game(sol::object obj): Game(lua_to_json(obj)) {}

        void prepare() {
            GameProperties::lua = &lua;

            GameProperties::game = this;
            GameProperties::factory = &factory;
            GameProperties::scenes = &scenes;
            GameProperties::scripts = &scripts;
            GameProperties::files = &files;
            GameProperties::messages = &messages;

            factory.prepareEntities();

            lua_checkstack(lua.lua_state(), GameProperties::lua_stack_size);

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
            init_build();

            Game::bindLua(lua);

            lua["game"] = this;
        }

        Amara::Game* init_build() {
            props = GameProperties::lua->create_table();

            sol::table props_meta = GameProperties::lua->create_table();
            props_meta["__newindex"] = [this](sol::table tbl, sol::object key, sol::object value) {
                if (value.is<sol::function>()) {
                    sol::function callback = value.as<sol::function>();
                    sol::function func = sol::make_object(*GameProperties::lua, [this, callback](sol::variadic_args va)->sol::object {
                        return callback(this, sol::as_args(va));
                    });
                    tbl.raw_set(key, func);
                }
                else tbl.raw_set(key, value);
            };
            props[sol::metatable_key] = props_meta;

            asLuaObject = make_lua_object();

            return this;
        }

        Amara::Game* configure(nlohmann::json config) {
            std::cout << config << std::endl;
            return this;
        }
        Amara::Game* configure(std::string key, nlohmann::json value) {
            nlohmann::json obj;
            obj[key] = value;
            return configure(obj);
        }
        sol::object super_configure(sol::object config) {
            configure(lua_to_json(config));
            return asLuaObject;
        }

        sol::function configure_override;
        sol::object luaConfigure(sol::object config) {
            if (configure_override.valid()) {
                try {
                    configure_override(asLuaObject, config);
                }
                catch (const sol::error& e) {
                    c_style_log("%s: error on configure().", "Game");
                }
            }
            else configure(lua_to_json(config));
            return asLuaObject;
        }
        sol::object luaConfigure(std::string key, sol::object val) {
            sol::table config = GameProperties::lua->create_table();
            config[key] = val;
            return luaConfigure(config);
        }

        sol::object make_lua_object() {
            return sol::make_object(*GameProperties::lua, this);
        }

        void execute(std::string command) {
            std::system(command.c_str());
        }
        
        static void bindLua(sol::state& lua) {
            bindLuaUtilityFunctions(lua);
            bindLuaGeometry(lua);

            FileManager::bindLua(lua);

            Entity::bindLua(lua);
            
            ScriptFactory::bindLua(lua);
            SceneManager::bindLua(lua);
            EntityFactory::bindLua(lua);
            
            lua.new_usertype<Game>("Game",
                sol::constructors<Game(), Game(sol::object)>(),
                "props", &Game::props,
                "scenes", &Game::scenes,
                "scripts", &Game::scripts,
                "factory", &Game::factory,
                "configure", sol::overload(
                    sol::resolve<sol::object(sol::object)>(Game::luaConfigure),
                    sol::resolve<sol::object(std::string, sol::object)>(&Game::luaConfigure)
                ),
                "configure_override", &Game::configure_override,
                "super_configure", &Game::super_configure,
                "execute", &Game::execute,
                "arguments", sol::property([](const Game& g) -> sol::object {
                    return json_to_lua(g.arguments);
                })
            );
        }

        ~Game() {
            lua["game"] = sol::nil;
            lua.collect_garbage();
        }
    };
}