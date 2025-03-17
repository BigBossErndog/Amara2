namespace Amara {
    class GameManager {
    public:
        int fps = 1;
        int targetFPS = -1;
        double deltaTime = 1;

        std::string platform;

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
            Properties::platform = platform;
        }

        void setTargetFPS(int _fps) {
            if (_fps < 1) {
                log("Error: Target FPS cannot be set lower than 1.");
                return;
            }
            targetFPS = _fps;
        }
        void uncapFPS() {
            targetFPS = -1;
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<GameManager>("GameManager",
                "fps", sol::readonly(&GameManager::fps),
                "targetFPS", sol::readonly(&GameManager::targetFPS),
                "deltaTime", sol::readonly(&GameManager::deltaTime),
                "platform", sol::readonly(&GameManager::platform),
                "setFPS", &GameManager::setTargetFPS,
                "uncapFPS", &GameManager::uncapFPS
            );
        }
    };
}