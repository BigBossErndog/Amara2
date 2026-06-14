namespace Amara {
    class GameManager {
    public:
        bool demiurgic = false;

        float fps = 0;
        float targetFPS = 60;
        double deltaTime = 0;

        double lifeTime = 0;

        std::string platform;
        bool desktop = false;
        bool mobile = false;
        bool console = false;

        std::vector<nlohmann::json> arguments;
        nlohmann::json argmap = nlohmann::json::object();

        bool hasQuit = false;

        Amara::GameProps* gameProps = nullptr;
        sol::object luaobject;
        
        std::string executable;

        bool debugging = false;
        
        GameManager() {
            #if defined(__EMSCRIPTEN__)
                platform = "web";
            #elif defined(_WIN32)
                platform = "windows";
                desktop = true;
            #elif defined(__ANDROID__)
                platform = "android";
                mobile = true;
            #elif defined(__linux__)
                platform = "linux";
                desktop = true;
            #elif defined(__APPLE__)
                #if TARGET_OS_IPHONE
                    platform = "iOS";
                    mobile = true;
                #else
                    platform = "macOS";
                    desktop = true;
                #endif
            #else
                platform = "unknown";
            #endif

            #ifdef AMARA_DEBUGGING
                debugging = true;
            #endif
        }
        
        void setTargetFPS(float _fps) {
            if (_fps < 0) {
                fatal_error("Error: Target FPS cannot be less than 0. (You can use 0 for uncapped frame rate).");
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

        void forceClose() {
            hasQuit = true;
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
            return Rectangle( -1, -1, -1, -1 );
        }

        Rectangle getMainDisplay() {
            return getDisplayBounds(1);
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<GameManager>("GameManager",
                "FPS", sol::readonly(&GameManager::fps),
                "targetFPS", sol::property(
                    [] (GameManager& g) {
                        return g.targetFPS;
                    },
                    [] (GameManager& g, float _fps) {
                        g.setTargetFPS(_fps);
                    }
                ),
                "setTargetFPS", &GameManager::setTargetFPS,
                "uncapFPS", &GameManager::uncapFPS,
                "deltaTime", sol::readonly(&GameManager::deltaTime),
                "platform", sol::readonly(&GameManager::platform),
                "desktop", sol::readonly(&GameManager::desktop),
                "mobile", sol::readonly(&GameManager::mobile),
                "get_lua_stack_size", &GameManager::get_lua_stack_size,
                "executable", sol::readonly(&GameManager::executable),
                "arguments", sol::property([](const GameManager& g) -> sol::object {
                    if (g.arguments.size() == 0) return sol::nil;
                    return json_to_lua(g.gameProps->lua, g.arguments);
                }),
                "argtable", sol::property([](GameManager& g) -> sol::table {
                    return json_to_lua(g.gameProps->lua, g.argmap);
                }),
                "getDisplayIDForPoint", &GameManager::getDisplayIDForPoint,
                "getDisplayBounds", &GameManager::getDisplayBounds,
                "mainDisplay", sol::property(&GameManager::getMainDisplay),
                "debugging", sol::readonly(&GameManager::debugging),
                "forceClose", &GameManager::forceClose
            );
        }
    };
}