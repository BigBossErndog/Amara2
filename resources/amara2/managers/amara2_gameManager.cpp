namespace Amara {
    class GameManager {
    public:
        bool demiurgic = false;

        float fps = 0;
        float targetFPS = -1;
        double deltaTime = 0;

        std::string platform;

        std::vector<nlohmann::json> arguments;

        bool hasQuit = false;

        GameManager() {
            #if defined(__EMSCRIPTEN__)
                platform = "web";
            #elif defined(_WIN32)
                platform = "windows";
            #elif defined(__linux__)
                platform = "linux";
            #elif defined(__ANDROID__)
                platform = "android";
            #elif defined(__APPLE__)
                #if TARGET_OS_IPHONE
                    platform = "iOS";
                #else
                    platform = "macOS";
                #endif
            #else
                platform = "unknown";
            #endif
            Props::platform = platform;
        }

        void setTargetFPS(float _fps) {
            if (_fps <= 0) {
                debug_log("Error: Target FPS cannot be 0 or less.");
                return;
            }
            if (demiurgic) {
                debug_log("Note: Demiurgic presence. FPS Overridden.");
                debug_log("Control will be handed over in target builds.");
            }
            targetFPS = _fps;
        }
        void uncapFPS() {
            if (demiurgic) {
                debug_log("Note: Demiurgic presence. FPS Overridden.");
                debug_log("Control will be handed over in target builds.");
            }
            targetFPS = -1;
        }

        int get_lua_stack_size() {
            return lua_gettop(Props::lua().lua_state());
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<GameManager>("GameManager",
                "fps", sol::readonly(&GameManager::fps),
                "targetFPS", sol::readonly(&GameManager::targetFPS),
                "setTargetFPS", &GameManager::setTargetFPS,
                "uncapFPS", &GameManager::uncapFPS,
                "deltaTime", sol::readonly(&GameManager::deltaTime),
                "platform", sol::readonly(&GameManager::platform),
                "get_lua_stack_size", &GameManager::get_lua_stack_size,
                "arguments", sol::property([](const GameManager& g) -> sol::object {
                    if (g.arguments.size() == 0) return sol::nil;
                    return json_to_lua(g.arguments);
                })
            );
        }
    };
}