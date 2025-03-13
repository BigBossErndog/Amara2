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

        Game() {
            prepare();
        }
        Game(nlohmann::json config): Game() {
            if (config.is_string()) configure(files.readJSON(config));
            else if (config.is_object()) configure(config);
        }

        void prepare() {
            GameProperties::files = &files;
            GameProperties::messages = &messages;

            lua.open_libraries(sol::lib::base);
            GameProperties::lua = &lua;
            // luaBind(lua);
        }

        void configure(nlohmann::json config) {
            
        }

        void luaConfigure(sol::object config) {
            configure(lua_to_json(config));
        }
        
        void luaBind(sol::state& lua) {
            lua.new_usertype<Game>("Game",
                "scenes", &Game::scenes,
                "factory", &Game::factory,
                "configure", &Game::luaConfigure
            );

            scenes.luaBind(lua);
            factory.luaBind(lua);

            bindLuaFunctions(lua);
        }
    };
}