namespace Amara {
    class ScriptFactory {
    public:
        std::unordered_map<std::string, std::function<Script*()>> factory;
        std::unordered_map<std::string, std::string> readScripts;
        std::unordered_map<std::string, sol::function> compiledScripts;

        void add(std::string key, std::string path) {
            std::string script = WorldProperties::files->getScriptPath(path);
            if (string_endsWith(script, ".lua")) {
                readScripts[key] = path;
            }
            else {
                compiledScripts[key] = WorldProperties::files->load_script(path);
            }
        }

        Amara::Script* get(std::string key) {
            auto it = factory.find(key);
            if (it != factory.end() && it->second) return (it->second());

            if (compiledScripts.find(key) != compiledScripts.end()) {
                try {
                    sol::object result = compiledScripts[key]();
                    return result.as<Amara::Script*>();
                }
                catch (const sol::error& e) {
                    log("Failed to create Script \"", key, "\".");
                }
            }
            else if (readScripts.find(key) != readScripts.end()) {
                try {
                    sol::object result = WorldProperties::files->run(readScripts[key]);
                    return result.as<Amara::Script*>();
                }
                catch (const sol::error& e) {
                    log("Failed to create Script \"", key, "\" from script \"", WorldProperties::files->getScriptPath(readScripts[key]), "\".");
                }
            }
            else log("Script \"", key, "\" was not found.");
            return nullptr;
        }

        sol::object run(std::string path) {
            return WorldProperties::files->run(path);
        }

        static void bindLua(sol::state& lua) {
            Script::bindLua(lua);

            lua.new_usertype<ScriptFactory>("ScriptFactory",
                "add", &ScriptFactory::add,
                "get", &ScriptFactory::get,
                "run", &ScriptFactory::run
            );
        }
    };
}