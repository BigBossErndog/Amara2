namespace Amara {
    class ControlScheme {
    public:
        Amara::GameProps* gameProps = nullptr;

        std::string id;
        std::vector<SDL_Keycode> keys;

        ControlScheme() {}
        ControlScheme(std::string _id, Amara::GameProps* _gameProps) {
            id = _id;
            gameProps = _gameProps;
        }
        
        void addKey(SDL_Keycode _k) {
            keys.push_back(_k);
        }
        void setKey(SDL_Keycode _k) {
            keys.clear();
            addKey(_k);
        }

        void addKeys(sol::object _keys) {
            nlohmann::json jkeys = lua_to_json(_keys);
            if (jkeys.is_array()) {
                for (int i = 0; i < jkeys.size(); i++) {
                    addKey(jkeys[i]);
                }
            }
        }
        void setKeys(sol::object _keys) {
            clearKeys();
            addKeys(_keys);
        }

        bool removeKey(SDL_Keycode _k) {
            for (auto it = keys.begin(); it != keys.end();) {
                SDL_Keycode& k = *it;
                if (k == _k) {
                    it = keys.erase(it);
                    continue;
                }
                ++it;
            }
        }

        void clearKeys() {
            keys.clear();
        }

        void clearAll() {
            clearKeys();
        }

        bool isDown() {
            for (SDL_Keycode k: keys) {
                if (gameProps->keyboard->isDown(k)) return true;
            }
            return false;
        }
        bool justPressed() {
            for (SDL_Keycode k: keys) {
                if (gameProps->keyboard->justPressed(k)) return true;
            }
            return false;
        }
        bool justReleased() {
            bool result = false;
            for (SDL_Keycode k: keys) {
                if (gameProps->keyboard->isDown(k)) return false;
                if (gameProps->keyboard->justReleased(k)) result = true;
            }
            return result;
        }
        double timeHeld() {
            double t = 0;
            double check;
            for (SDL_Keycode k: keys) {
                check = gameProps->keyboard->timeHeld(k);
                if (check > t) t = check;
            }
            return t;
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<ControlScheme>("ControlScheme", 
                "addKey", &ControlScheme::addKey,
                "setKey", &ControlScheme::setKey,
                "addKeys", &ControlScheme::addKey,
                "setKeys", &ControlScheme::setKeys,
                "removeKey", &ControlScheme::removeKey,
                "clearKeys", &ControlScheme::clearKeys,
                "clearAll", &ControlScheme::clearAll,
                "isDown", &ControlScheme::isDown,
                "justPressed", &ControlScheme::justPressed,
                "justReleased", &ControlScheme::justReleased,
                "timeHeld", &ControlScheme::timeHeld
            );
        }
    };
}