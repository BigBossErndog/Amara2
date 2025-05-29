namespace Amara {
    class GameManager {
    public:
        bool demiurgic = false;

        float fps = 0;
        float targetFPS = 60;
        double deltaTime = 0;

        double lifeTime = 0;

        std::string platform;

        std::vector<nlohmann::json> arguments;

        bool hasQuit = false;

        Amara::GameProps* gameProps = nullptr;

        std::string executable;
        
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
        }
        
        void setTargetFPS(float _fps) {
            if (_fps < 0) {
                debug_log("Error: Target FPS cannot be less than 0. (You can use 0 for uncapped frame rate).");
                return;
            }
            if (demiurgic) {
                debug_log("Note: Demiurgic presence. FPS Overridden.");
                debug_log("Control will be handed over in target builds.");
            }
            targetFPS = _fps;
        }
        void uncapFPS() {
            setTargetFPS(0);
        }

        int get_lua_stack_size() {
            return lua_gettop(gameProps->lua.lua_state());
        }

        Uint32 getDisplayIDForPoint(Vector2 p) {
            SDL_Point sp = { static_cast<int>(p.x), static_cast<int>(p.y) };
            return SDL_GetDisplayForPoint(&sp);
        }

        Rectangle getDisplayBounds(Uint32 dispID) {
            SDL_Rect rect;
            if (SDL_GetDisplayBounds(dispID, &rect)) {
                return { 
                    static_cast<float>(rect.x),
                    static_cast<float>(rect.y), 
                    static_cast<float>(rect.w),
                    static_cast<float>(rect.h)
                };
            }
            return { 0, 0, 0, 0 };
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<GameManager>("GameManager",
                "fps", sol::readonly(&GameManager::fps),
                "targetFPS", sol::readonly(&GameManager::targetFPS),
                "setTargetFPS", &GameManager::setTargetFPS,
                "uncapFPS", &GameManager::uncapFPS,
                "deltaTime", sol::readonly(&GameManager::deltaTime),
                "platform", sol::readonly(&GameManager::platform),
                "get_lua_stack_size", &GameManager::get_lua_stack_size,
                "executable", sol::readonly(&GameManager::executable),
                "arguments", sol::property([](const GameManager& g) -> sol::object {
                    if (g.arguments.size() == 0) return sol::nil;
                    return json_to_lua(g.gameProps->lua, g.arguments);
                }),
                "getDisplayIDForPoint", &GameManager::getDisplayIDForPoint
            );
        }
    };
}