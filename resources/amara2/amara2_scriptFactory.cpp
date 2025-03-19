namespace Amara {
    class ScriptFactory {
    public:
        std::unordered_map<std::string, std::function<Script*()>> factory;
        std::unordered_map<std::string, std::string> readScripts;
        std::unordered_map<std::string, sol::function> compiledScripts;

        bool load(std::string key, std::string path) {
            std::string script_path = Properties::files->getScriptPath(path);
            if (!Properties::files->fileExists(script_path)) {
                debug_log("Failed to load script \"", key, "\" from \"", path, "\". File not found.");
                return false;
            }
            if (string_endsWith(script_path, ".lua")) {
                readScripts[key] = script_path;
            }
            else {
                compiledScripts[key] = Properties::files->load_script(script_path);
            }
            return true;
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
                    debug_log("Failed to create script \"", key, "\".");
                }
            }
            else if (readScripts.find(key) != readScripts.end()) {
                try {
                    sol::object result = Properties::files->run(readScripts[key]);
                    return result.as<Amara::Script*>();
                }
                catch (const sol::error& e) {
                    debug_log("Failed to create script \"", key, "\" from script \"", Properties::files->getScriptPath(readScripts[key]), "\".");
                }
            }
            else debug_log("Script \"", key, "\" was not found.");
            return nullptr;
        }

        sol::object run(std::string path) {
            if (compiledScripts.find(path) != compiledScripts.end()) {
                try {
                    sol::object result = compiledScripts[path]();
                    return result;
                }
                catch (const sol::error& e) {
                    debug_log("Failed to run cached script \"", path, "\".");
                    return sol::nil;
                }
            }
            else if (readScripts.find(path) != readScripts.end()) {
                try {
                    sol::object result = Properties::files->run(readScripts[path]);
                    return result;
                }
                catch (const sol::error& e) {
                    debug_log("Failed to run script \"", path, "\" from file \"", Properties::files->getScriptPath(readScripts[path]), "\".");
                    return sol::nil;
                }
            }
            return Properties::files->run(path);
        }

        static void bindLua(sol::state& lua) {
            Script::bindLua(lua);

            lua.new_usertype<ScriptFactory>("ScriptFactory",
                "load", &ScriptFactory::load,
                "get", &ScriptFactory::get,
                "run", &ScriptFactory::run
            );
        }
    };
}