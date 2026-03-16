namespace Amara {
    class ControlScheme {
    public:
        Amara::GameProps* gameProps = nullptr;

        std::string id;
        std::vector<SDL_Keycode> keys;
        std::vector<Amara::GamepadButton> gamepadButtons;

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

        void addKeys(sol::variadic_args _keys) {
            for (auto k: _keys) {
                addKey(k.as<SDL_Keycode>());
            }
        }
        void setKeys(sol::variadic_args _keys) {
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

        void addButton(Amara::GamepadButton _b) {
            gamepadButtons.push_back(_b);
        }
        void setButton(Amara::GamepadButton _b) {
            gamepadButtons.clear();
            addButton(_b);
        }

        void addButtons(sol::variadic_args _buttons) {
            for (auto b: _buttons) {
                addButton(b.as<Amara::GamepadButton>());
            }
        }
        void setButtons(sol::variadic_args _buttons) {
            clearButtons();
            addButtons(_buttons);
        }

        bool removeButton(Amara::GamepadButton _b) {
            for (auto it = gamepadButtons.begin(); it != gamepadButtons.end();) {
                Amara::GamepadButton& b = *it;
                if (b == _b) {
                    it = gamepadButtons.erase(it);
                    continue;
                }
                ++it;
            }
        }

        void clearKeys() {
            keys.clear();
        }

        void clearButtons() {
            gamepadButtons.clear();
        }

        void clearAll() {
            clearKeys();
            clearButtons();
        }

        bool isDown() {
            for (SDL_Keycode k: keys) {
                if (gameProps->keyboard->isDown(k)) return true;
            }
            for (Amara::GamepadButton b: gamepadButtons) {
                if (gameProps->gamepads->isDown(b)) return true;
            }
            return false;
        }
        bool justPressed() {
            for (SDL_Keycode k: keys) {
                if (gameProps->keyboard->justPressed(k)) return true;
            }
            for (Amara::GamepadButton b: gamepadButtons) {
                if (gameProps->gamepads->justPressed(b)) return true;
            }
            return false;
        }
        bool justReleased() {
            bool result = false;
            for (SDL_Keycode k: keys) {
                if (gameProps->keyboard->isDown(k)) return false;
                if (gameProps->keyboard->justReleased(k)) result = true;
            }
            for (Amara::GamepadButton b: gamepadButtons) {
                if (gameProps->gamepads->isDown(b)) return false;
                if (gameProps->gamepads->justReleased(b)) result = true;
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
            for (Amara::GamepadButton b: gamepadButtons) {
                check = gameProps->gamepads->timeHeld(b);
                if (check > t) t = check;
            }
            return t;
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<ControlScheme>("ControlScheme", 
                "addKey", &ControlScheme::addKey,
                "setKey", &ControlScheme::setKey,
                "addKeys", &ControlScheme::addKey,
                "setKeys", &ControlScheme::setKeys,
                "removeKey", &ControlScheme::removeKey,
                "clearKeys", &ControlScheme::clearKeys,

                "key", sol::property(
                    [](ControlScheme& t) -> sol::optional<int> {
                        if (!t.keys.empty()) {
                            return t.keys[0];
                        }
                        return sol::nullopt;
                    },
                    [](ControlScheme& t, sol::object v) {
                        t.clearKeys();
                        if (v.is<SDL_Keycode>()) {
                            t.addKey(v.as<int>());
                        }
                        else if (v.is<sol::table>()) {
                            sol::table keys = v.as<sol::table>();
                            for (auto& pair: keys) {
                                sol::object obj = pair.second;
                                if (obj.is<int>()) {
                                    t.addKey(obj.as<int>());
                                }
                            }
                        }
                    }
                ),
                "keys", sol::property(
                    [](ControlScheme& t) -> sol::object {
                        sol::table keys = t.gameProps->lua.create_table();
                        for (SDL_Keycode k: t.keys) {
                            keys.add((int)k);
                        }
                        return keys;
                    },
                    [](ControlScheme& t, sol::object v) {
                        t.clearKeys();
                        if (v.is<int>()) {
                            t.addKey(v.as<int>());
                        }
                        else if (v.is<sol::table>()) {
                            sol::table keys = v.as<sol::table>();
                            for (auto& pair: keys) {
                                sol::object obj = pair.second;
                                if (obj.is<int>()) {
                                    t.addKey(obj.as<int>());
                                }
                            }
                        }
                    }
                ),
                
                "addButton", &ControlScheme::addButton,
                "setButton", &ControlScheme::setButton,
                "addButtons", &ControlScheme::addButtons,
                "setButtons", &ControlScheme::setButtons,
                "removeButton", &ControlScheme::removeButton,
                "clearButtons", &ControlScheme::clearButtons,

                "button", sol::property(
                    [](ControlScheme& t) -> sol::optional<int> {
                        if (!t.gamepadButtons.empty()) {
                            return static_cast<int>(t.gamepadButtons[0]);
                        }
                        return sol::nullopt;
                    },
                    [](ControlScheme& t, sol::object v) {
                        t.clearButtons();
                        if (v.is<int>()) {
                            t.addButton(static_cast<Amara::GamepadButton>(v.as<int>()));
                        }
                        else if (v.is<sol::table>()) {
                            sol::table buttons = v.as<sol::table>();
                            for (auto& pair : buttons) {
                                sol::object obj = pair.second;
                                if (obj.is<int>()) {
                                    t.addButton(static_cast<Amara::GamepadButton>(obj.as<int>()));
                                }
                            }
                        }
                    }
                ),
                "buttons", sol::property(
                    [](ControlScheme& t) -> sol::object {
                        sol::table buttons = t.gameProps->lua.create_table();
                        for (Amara::GamepadButton b : t.gamepadButtons) {
                            buttons.add(static_cast<int>(b));
                        }
                        return buttons;
                    },
                    [](ControlScheme& t, sol::object v) {
                        t.clearButtons();
                        if (v.is<int>()) {
                            t.addButton(static_cast<Amara::GamepadButton>(v.as<int>()));
                        }
                        else if (v.is<sol::table>()) {
                            sol::table buttons = v.as<sol::table>();
                            for (auto& pair : buttons) {
                                sol::object obj = pair.second;
                                if (obj.is<int>()) {
                                    t.addButton(static_cast<Amara::GamepadButton>(obj.as<int>()));
                                }
                            }
                        }
                    }
                ),

                "clearAll", &ControlScheme::clearAll,

                "isDown", &ControlScheme::isDown,
                "justPressed", &ControlScheme::justPressed,
                "justReleased", &ControlScheme::justReleased,
                "timeHeld", &ControlScheme::timeHeld
            );
        }
    };
}