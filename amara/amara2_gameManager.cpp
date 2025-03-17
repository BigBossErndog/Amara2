namespace Amara {
    class GameManager {
    public:
        int targetFPS = 60;
        float deltaTime = 0;

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

        static void bindLua(sol::state& lua) {
            lua.new_usertype<GameManager>("GameManager",
                "targetFPS", sol::readonly(&GameManager::targetFPS),
                "deltaTime", sol::readonly(&GameManager::deltaTime),
                "platform", sol::readonly(&GameManager::platform)
            );
        }
    };
}