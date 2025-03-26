namespace Amara {
    class ScriptFactory {
    public:
        std::unordered_map<std::string, std::string> readScripts;
        std::unordered_map<std::string, sol::function> compiledScripts;

        bool load(std::string key, std::string path) {
            std::string script_path = Props::files->getScriptPath(path);
            if (!Props::files->fileExists(script_path)) {
                debug_log("Failed to load script \"", key, "\" from \"", path, "\". File not found.");
                Props::lua_exception_thrown = true;
                return false;
            }
            if (string_endsWith(script_path, ".lua")) {
                readScripts[key] = script_path;
            }
            else {
                compiledScripts[key] = Props::files->load_script(script_path);
            }
            return true;
        }

        sol::object get(std::string key) {
            if (compiledScripts.find(key) != compiledScripts.end()) {
                try {
                    return compiledScripts[key];
                }
                catch (const sol::error& e) {
                    debug_log("Failed to create script \"", key, "\".");
                    Props::lua_exception_thrown = true;
                }
            }
            else if (readScripts.find(key) != readScripts.end()) {
                try {
                    return Props::files->load_script(readScripts[key]);
                }
                catch (const sol::error& e) {
                    debug_log("Failed to create script \"", key, "\" from script \"", Props::files->getScriptPath(readScripts[key]), "\".");
                    Props::lua_exception_thrown = true;
                }
            }
            else debug_log("Script \"", key, "\" was not found.");
            return sol::nil;
        }

        sol::object run(std::string path) {
            if (compiledScripts.find(path) != compiledScripts.end()) {
                try {
                    sol::object result = compiledScripts[path]();
                    return result;
                }
                catch (const sol::error& e) {
                    debug_log("Failed to run cached script \"", path, "\".");
                    Props::lua_exception_thrown = true;
                    return sol::nil;
                }
            }
            else if (readScripts.find(path) != readScripts.end()) {
                try {
                    sol::object result = Props::files->run(readScripts[path]);
                    return result;
                }
                catch (const sol::error& e) {
                    debug_log("Failed to run script \"", path, "\" from file \"", Props::files->getScriptPath(readScripts[path]), "\".");
                    Props::lua_exception_thrown = true;
                    return sol::nil;
                }
            }
            path = Props::files->getScriptPath(path);
            return Props::files->run(path);
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<ScriptFactory>("ScriptFactory",
                "load", &ScriptFactory::load,
                "get", &ScriptFactory::get,
                "run", &ScriptFactory::run
            );
        }

        void clear() {
            readScripts.clear();
            compiledScripts.clear();
        }
    };
}