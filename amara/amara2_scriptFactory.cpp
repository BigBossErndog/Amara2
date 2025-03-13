namespace Amara {
    class ScriptFactory {
    public:
        sol::state* lua;

        std::unordered_map<std::string, std::string> scriptScripts;

        void init(sol::state& gLua) {
            lua = &gLua;
            luaBind(gLua);
        }

        void add(std::string key, std::string script) {
            scriptScripts[key] = script;
        }

        Amara::Script* create(std::string key) {
            Amara::Script* script = nullptr;
            try {
                #ifndef AMARA_FINAL_BUILD
                    script = (*lua).script_file(string_concat(std::vector<std::string>{
                        "./lua/",
                        scriptScripts[key],
                        ".luac"
                    }));
                #else 
                    script = (*lua).script(string_concat(std::vector<std::string>{
                        "../lua/",
                        scriptScripts[key],
                        ".lua"
                    }));
                #endif
            }
            catch (const sol::error& e) {
                SDL_Log("%s error: %s", key.c_str(), e.what());
                script = nullptr;
            }
            return script;
        }

        void luaBind(sol::state& lua) {
            lua.new_usertype<ScriptFactory>("ScriptFactory",
                "add", &ScriptFactory::add,
                "create", &ScriptFactory::create
            );

            Script::luaBind(lua);
        }
    };
}