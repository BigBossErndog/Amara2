namespace Amara {
    class ScriptFactory {
    public:
        std::unordered_map<std::string, std::function<Script*()>> factory;
        std::unordered_map<std::string, std::string> scriptScripts;

        void add(std::string key, std::string script) {
            scriptScripts[key] = script;
        }

        Amara::Script* get(std::string key) {
            auto it = factory.find(key);
            if (it != factory.end() && it->second) return (it->second());

            try {
                return (GameProperties::files->run(scriptScripts[key]).as<Amara::Script*>());
            }
            catch (const sol::error& e) {
                c_style_log("EntityFactory error: Failed to create script \'%s\'", key.c_str());
            }
            return nullptr;
        }

        static void bindLua(sol::state& lua) {
            Script::bindLua(lua);

            lua.new_usertype<ScriptFactory>("ScriptFactory",
                "add", &ScriptFactory::add,
                "get", &ScriptFactory::get
            );
        }
    };
}