namespace Amara {
    class ScriptFactory {
    public:
        std::unordered_map<std::string, std::string> readScripts;
        std::unordered_map<std::string, sol::function> compiledScripts;

        Amara::GameProps* gameProps = nullptr;
        sol::object luaobject;

        ScriptFactory() = default;
        
        bool load(std::string key, std::string path) {
            std::string script_path = gameProps->system->getScriptPath(path);
            if (!gameProps->system->exists(script_path)) {
                fatal_error("Failed to load script \"", key, "\" from \"", path, "\". File not found.");
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
                    fatal_error(e.what());
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
                    fatal_error(e.what());
                    gameProps->lua_exception_thrown = true;
                    return sol::nil;
                }
            }
            return gameProps->system->run(path);
        }

        sol::object execute(std::string code) {
            sol::load_result loadResult = gameProps->lua.load(code, "inline_script", sol::load_mode::text);
            if (!loadResult.valid()) {
                sol::error err = loadResult;
                fatal_error(err.what());
                gameProps->lua_exception_thrown = true;
                return sol::nil;
            }
            sol::protected_function scriptFunc = loadResult;
            sol::protected_function_result execResult = scriptFunc();
            if (!execResult.valid()) {
                sol::error err = execResult;
                fatal_error(err.what());
                gameProps->lua_exception_thrown = true;
                return sol::nil;
            }
            
            return execResult;
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<ScriptFactory>("ScriptFactory",
                "load", &ScriptFactory::load,
                "get", &ScriptFactory::get,
                "run", &ScriptFactory::run,
                "include", &ScriptFactory::run
            );
        }

        void clear() {
            readScripts.clear();
            compiledScripts.clear();
        }
    };
}