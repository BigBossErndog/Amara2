namespace Amara {
    class ScriptFactory {
    public:
        std::unordered_map<std::string, std::string> readScripts;
        std::unordered_map<std::string, sol::function> compiledScripts;

        Amara::GameProps* gameProps = nullptr;

        ScriptFactory() = default;
        
        bool load(std::string key, std::string path) {
            std::string script_path = gameProps->system->getScriptPath(path);
            if (!gameProps->system->exists(script_path)) {
                debug_log("Failed to load script \"", key, "\" from \"", path, "\". File not found.");
                gameProps->lua_exception_thrown = true;
                gameProps->breakWorld();
                return false;
            }
            if (String::endsWith(script_path, ".lua")) {
                readScripts[key] = script_path;
            }
            else {
                sol::load_result loadResult = gameProps->system->load_script(script_path);
                if (!loadResult.valid()) {
                    compiledScripts[key] = loadResult;
                }
                else {
                    fatal_error("Error: Invalid script file at \"", script_path, "\"");
                    gameProps->lua_exception_thrown = true;
                    gameProps->breakWorld();
                    return false;
                }
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
                    gameProps->lua_exception_thrown = true;
                }
            }
            else if (readScripts.find(key) != readScripts.end()) {
                try {
                    return gameProps->system->load_script(readScripts[key]);
                }
                catch (const sol::error& e) {
                    debug_log("Failed to create script \"", key, "\" from script \"", gameProps->system->getScriptPath(readScripts[key]), "\".");
                    gameProps->lua_exception_thrown = true;
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
                    gameProps->lua_exception_thrown = true;
                    return sol::nil;
                }
            }
            else if (readScripts.find(path) != readScripts.end()) {
                try {
                    sol::object result = gameProps->system->run(readScripts[path]);
                    return result;
                }
                catch (const sol::error& e) {
                    debug_log("Failed to run script \"", path, "\" from file \"", gameProps->system->getScriptPath(readScripts[path]), "\".");
                    gameProps->lua_exception_thrown = true;
                    return sol::nil;
                }
            }
            path = gameProps->system->getScriptPath(path);
            return gameProps->system->run(path);
        }

        static void bind_lua(sol::state& lua) {
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