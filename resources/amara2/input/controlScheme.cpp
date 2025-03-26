namespace Amara {
    class ControlScheme {
    public:
        std::string id;

        KeyboardManager* keyboard = nullptr;
        std::vector<SDL_Keycode> keys;

        ControlScheme() {}
        ControlScheme(
            std::string _id,
            KeyboardManager* _keyboard
        ) {
            id = _id;
            keyboard = _keyboard;
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

        static void bindLua(sol::state& lua) {
            lua.new_usertype<ControlScheme>("ControlScheme", 
                "addKey", &ControlScheme::addKey,
                "setKey", &ControlScheme::setKey,
                "addKeys", &ControlScheme::addKey,
                "setKeys", &ControlScheme::setKeys,
                "removeKey", &ControlScheme::removeKey,
                "clearKeys", &ControlScheme::clearKeys,
                "clearAll", &ControlScheme::clearAll
            );
        }
    };
}