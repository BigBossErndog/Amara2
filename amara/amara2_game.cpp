namespace Amara {
    class Game {
    public:
        sol::state lua;
        sol::table props;

        SceneManager scenes;
        EntityFactory factory;
        ScriptFactory scripts;

        FileManager files;
        MessageQueue messages;

        std::string id = "Game";

        Game() {
            prepare();
        }
        Game(nlohmann::json config): Game() {
            if (config.is_string()) configure(files.readJSON((std::string)config));
            else if (config.is_object()) configure(config);
        }

        void prepare() {
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

            GameProperties::lua = &lua;
            Game::bindLua(lua);

            lua["game"] = this;
            lua["files"] = &files;
            lua["factory"] = &factory;
            lua["scenes"] = &scenes;
            lua["scripts"] = &scripts;
        }

        void configure(nlohmann::json config) {
            std::cout << config << std::endl;
        }

        void luaConfigure(sol::object config) {
            configure(lua_to_json(config));
        }

        sol::object run(std::string path) {
            return files.run(path);
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
                "id", &Game::id,
                "scenes", &Game::scenes,
                "factory", &Game::factory,
                "props", &Game::props,
                "configure", &Game::luaConfigure,
                "run", &Game::run
            );
        }
    };
}