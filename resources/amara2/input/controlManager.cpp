namespace Amara {
    class ControlManager {
    public:
        std::unordered_map<std::string, ControlScheme> controls;
        KeyboardManager keyboard;

        ControlScheme* createScheme(std::string key) {
            controls[key] = ControlScheme(key, &keyboard);
            ControlScheme& scheme = controls[key];

            return &scheme;
        }

        ControlScheme* getScheme(std::string key) {
            if (controls.find(key) != controls.end()) {
                return &(controls[key]);
            }
            return nullptr;
        }

        void removeScheme(std::string key) {
            if (controls.find(key) != controls.end()) {
                controls.erase(key);
            }
        }

        void clearAllSchemes() {
            controls.clear();
        }

        void manage(double deltaTime) {
            keyboard.manage(deltaTime);
        }

        static void bindLua(sol::state& lua) {
            KeyboardManager::bindLua(lua);
            ControlScheme::bindLua(lua);

            lua.new_usertype<ControlManager>("ControlManager",
                "createScheme", &ControlManager::createScheme,
                "getScheme", &ControlManager::getScheme,
                "removeScheme", &ControlManager::removeScheme,
                "clearAllSchemes", &ControlManager::clearAllSchemes
            );
        }
    };
}