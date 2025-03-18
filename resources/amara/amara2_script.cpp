namespace Amara {
    class Script {
    public:
        sol::function luaPrepare;
        sol::function luaScript;

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Script>("Script",
                "prepare", &Script::luaPrepare,
                "script", &Script::luaScript
            );
        }
    };
}