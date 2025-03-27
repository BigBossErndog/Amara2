namespace Amara {
    class ControlManager {
    public:
        std::unordered_map<std::string, ControlScheme> controls;

        ControlScheme* createScheme(std::string key) {
            controls[key] = ControlScheme(key);
            ControlScheme& scheme = controls[key];

            return &scheme;
        }

        ControlScheme* getScheme(std::string key) {
            if (controls.find(key) != controls.end()) {
                return &(controls[key]);
            }
            return nullptr;
        }

        ControlScheme* scheme(std::string key) {
            if (controls.find(key) != controls.end()) {
                return &(controls[key]);
            }
            return createScheme(key);
        }

        void removeScheme(std::string key) {
            if (controls.find(key) != controls.end()) {
                controls.erase(key);
            }
        }

        void clearAllSchemes() {
            controls.clear();
        }

        bool isDown(std::string key) {
            if (controls.find(key) != controls.end()) {
                return controls[key].isDown();
            }
            return false;
        }
        bool justPressed(std::string key) {
            if (controls.find(key) != controls.end()) {
                return controls[key].justPressed();
            }
            return false;
        }
        bool justReleased(std::string key) {
            if (controls.find(key) != controls.end()) {
                return controls[key].justReleased();
            }
            return false;
        }
        double timeHeld(std::string key) {
            if (controls.find(key) != controls.end()) {
                return controls[key].timeHeld();
            }
            return false;
        }

        static void bindLua(sol::state& lua) {
            KeyboardManager::bindLua(lua);
            ControlScheme::bindLua(lua);

            lua.new_usertype<ControlManager>("ControlManager",
                "createScheme", &ControlManager::createScheme,
                "getScheme", &ControlManager::getScheme,
                "scheme", &ControlManager::scheme,
                "removeScheme", &ControlManager::removeScheme,
                "clearAllSchemes", &ControlManager::clearAllSchemes,
                "isDown", &ControlManager::isDown,
                "justPressed", &ControlManager::justPressed,
                "justReleased", &ControlManager::justReleased,
                "timeHeld", &ControlManager::timeHeld
            );
        }
    };
}