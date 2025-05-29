namespace Amara {
    class Hotkey: public Amara::Action {
    public:
        sol::function onPress;

        bool pressed = false;
        bool rec_pressed = false;

        std::vector<SDL_Keycode> keys;

        Hotkey(): Amara::Action() {
            set_base_node_id("Hotkey");
        }

        virtual Amara::Node* configure(nlohmann::json config) {
            if (json_has(config, "keys")) {
                nlohmann::json keys_json = config["keys"];
                for (int i = 0; i < keys_json.size(); i++) {
                    keys.push_back((SDL_Keycode)keys_json[i].get<int>());
                }
            }

            return Amara::Action::configure(config);
        }

        virtual sol::object luaConfigure(std::string key, sol::object val) override {
            if (val.is<sol::function>()) {
                if (String::equal(key, "onPress")) onPress = val.as<sol::function>();
            }
            return Amara::Action::luaConfigure(key, val);
        }

        bool is_key_pressed(int val) {
            return (GetAsyncKeyState(val) & 0x8000) != 0;
        }

        int SDLKeyToWindowsVK(SDL_Keycode sdlKey) {
            if (sdlKey >= SDLK_A && sdlKey <= SDLK_Z) {
                return 'A' + (sdlKey - SDLK_A);
            }
            if (sdlKey >= SDLK_0 && sdlKey <= SDLK_9) {
                return '0' + (sdlKey - SDLK_0);
            }

            if (sdlKey >= SDLK_KP_0 && sdlKey <= SDLK_KP_9) {
                return VK_NUMPAD0 + (sdlKey - SDLK_KP_0);
            }

            switch (sdlKey) {
                case SDLK_LCTRL: return VK_LCONTROL;
                case SDLK_RCTRL: return VK_RCONTROL;
                case SDLK_LSHIFT: return VK_LSHIFT;
                case SDLK_RSHIFT: return VK_RSHIFT;
                case SDLK_LALT: return VK_LMENU;
                case SDLK_RALT: return VK_RMENU;

                case SDLK_UP: return VK_UP;
                case SDLK_DOWN: return VK_DOWN;
                case SDLK_LEFT: return VK_LEFT;
                case SDLK_RIGHT: return VK_RIGHT;

                case SDLK_F1: return VK_F1;
                case SDLK_F2: return VK_F2;
                case SDLK_F3: return VK_F3;
                case SDLK_F4: return VK_F4;
                case SDLK_F5: return VK_F5;
                case SDLK_F6: return VK_F6;
                case SDLK_F7: return VK_F7;
                case SDLK_F8: return VK_F8;
                case SDLK_F9: return VK_F9;
                case SDLK_F10: return VK_F10;
                case SDLK_F11: return VK_F11;
                case SDLK_F12: return VK_F12;
                case SDLK_F13: return VK_F13;
                case SDLK_F14: return VK_F14;
                case SDLK_F15: return VK_F15;
                case SDLK_F16: return VK_F16;
                case SDLK_F17: return VK_F17;
                case SDLK_F18: return VK_F18;
                case SDLK_F19: return VK_F19;
                case SDLK_F20: return VK_F20;
                case SDLK_F21: return VK_F21;
                case SDLK_F22: return VK_F22;
                case SDLK_F23: return VK_F23;
                case SDLK_F24: return VK_F24;

                case SDLK_RETURN: return VK_RETURN;
                case SDLK_ESCAPE: return VK_ESCAPE;
                case SDLK_BACKSPACE: return VK_BACK;
                case SDLK_TAB: return VK_TAB;
                case SDLK_SPACE: return VK_SPACE;
                case SDLK_DELETE: return VK_DELETE;
                case SDLK_INSERT: return VK_INSERT;
                case SDLK_HOME: return VK_HOME;
                case SDLK_END: return VK_END;
                case SDLK_PAGEUP: return VK_PRIOR;
                case SDLK_PAGEDOWN: return VK_NEXT;

                case SDLK_COMMA: return VK_OEM_COMMA;
                case SDLK_PERIOD: return VK_OEM_PERIOD;
                case SDLK_SLASH: return VK_OEM_2;
                case SDLK_SEMICOLON: return VK_OEM_1;
                case SDLK_APOSTROPHE: return VK_OEM_7;
                case SDLK_LEFTBRACKET: return VK_OEM_4;
                case SDLK_RIGHTBRACKET: return VK_OEM_6;
                case SDLK_BACKSLASH: return VK_OEM_5;
                case SDLK_MINUS: return VK_OEM_MINUS;
                case SDLK_EQUALS: return VK_OEM_PLUS;
                case SDLK_GRAVE: return VK_OEM_3;

                case SDLK_KP_PERIOD: return VK_DECIMAL;
                case SDLK_KP_DIVIDE: return VK_DIVIDE;
                case SDLK_KP_MULTIPLY: return VK_MULTIPLY;
                case SDLK_KP_MINUS: return VK_SUBTRACT;
                case SDLK_KP_PLUS: return VK_ADD;
                case SDLK_KP_ENTER: return VK_RETURN;

                case SDLK_CAPSLOCK: return VK_CAPITAL;
                case SDLK_NUMLOCKCLEAR: return VK_NUMLOCK;
                case SDLK_SCROLLLOCK: return VK_SCROLL;

                case SDLK_PRINTSCREEN: return VK_SNAPSHOT;
                case SDLK_PAUSE: return VK_PAUSE;
                case SDLK_LGUI: return VK_LWIN;
                case SDLK_RGUI: return VK_RWIN;
                case SDLK_APPLICATION: return VK_APPS;

                case SDLK_UNKNOWN:
                default:
                    return 0;
            }
        }

        virtual void act(double deltaTime) {
            rec_pressed = pressed;
            pressed = true;

            if (keys.size() > 0) {
                for (int i = 0; i < keys.size(); i++) {
                    if (!is_key_pressed(SDLKeyToWindowsVK(keys[i]))) {
                        pressed = false;
                    }
                }
            }
            else pressed = false;

            if (pressed && !rec_pressed) {
                if (onPress.valid()) {
                    try {
                    sol::protected_function_result result = onPress(actor->get_lua_object(), get_lua_object());
                    if (!result.valid()) {
                        sol::error err = result;
                        throw std::runtime_error("Lua Error: " + std::string(err.what()));
                    }
                    } catch (const std::exception& e) {
                        debug_log(e.what());
                        gameProps->breakWorld();
                    }
                }
            }
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Hotkey>("Hotkey",
                "onPress", &Hotkey::onPress
            );
        }
    };
}