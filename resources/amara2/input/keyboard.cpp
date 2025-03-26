namespace Amara {
    class KeyboardManager {
    public:
        std::unordered_map<SDL_Keycode, Button> keys;

        void press(SDL_Keycode code) {
            if (keys.find(code) == keys.end()) {
                keys[code] = Button();
            }
            keys[code].press();
        }

        void release(SDL_Keycode code) {
            if (keys.find(code) == keys.end()) {
                keys[code] = Button();
            }
            keys[code].release();
        }

        void manage(double deltaTime) {
            for (auto it = keys.begin(); it != keys.end(); it++) {
                Button& key = it->second;
                key.manage(deltaTime);
            }
        }

        bool isDown(SDL_Keycode code) {
            if (keys.find(code) != keys.end()) {
                return keys[code].isDown;
            }
            return false;
        }

        bool justPressed(SDL_Keycode code) {
            if (keys.find(code) != keys.end()) {
                return keys[code].justPressed;
            }
            return false;
        }

        bool justReleased(SDL_Keycode code) {
            if (keys.find(code) != keys.end()) {
                return keys[code].justReleased;
            }
            return false;
        }

        double timeHeld(SDL_Keycode code) {
            if (keys.find(code) != keys.end()) {
                return keys[code].timeHeld;
            }
            return 0;
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<KeyboardManager>("Keyboard",
                "isDown", &KeyboardManager::isDown,
                "justPressed", &KeyboardManager::justPressed,
                "justReleased", &KeyboardManager::justReleased,
                "timeHeld", &KeyboardManager::timeHeld
            );
    
            lua.new_enum("Key",
                "UNKNOWN", SDLK_UNKNOWN,
                "RETURN", SDLK_RETURN,
                "ESCAPE", SDLK_ESCAPE,
                "BACKSPACE", SDLK_BACKSPACE,
                "TAB", SDLK_TAB,
                "SPACE", SDLK_SPACE,
                "EXCLAIM", SDLK_EXCLAIM,
                "DBLAPOSTROPHE", SDLK_DBLAPOSTROPHE,
                "HASH", SDLK_HASH,
                "PERCENT", SDLK_PERCENT,
                "DOLLAR", SDLK_DOLLAR,
                "AMPERSAND", SDLK_AMPERSAND,
                "APOSTROPHE", SDLK_APOSTROPHE,
                "LEFTPAREN", SDLK_LEFTPAREN,
                "RIGHTPAREN", SDLK_RIGHTPAREN,
                "ASTERISK", SDLK_ASTERISK,
                "PLUS", SDLK_PLUS,
                "COMMA", SDLK_COMMA,
                "MINUS", SDLK_MINUS,
                "PERIOD", SDLK_PERIOD,
                "SLASH", SDLK_SLASH,
                "COLON", SDLK_COLON,
                "SEMICOLON", SDLK_SEMICOLON,
                "LESS", SDLK_LESS,
                "EQUALS", SDLK_EQUALS,
                "GREATER", SDLK_GREATER,
                "QUESTION", SDLK_QUESTION,
                "AT", SDLK_AT,
                "LEFTBRACKET", SDLK_LEFTBRACKET,
                "BACKSLASH", SDLK_BACKSLASH,
                "RIGHTBRACKET", SDLK_RIGHTBRACKET,
                "CARET", SDLK_CARET,
                "UNDERSCORE", SDLK_UNDERSCORE,
                "GRAVE", SDLK_GRAVE,
                "DELETE", SDLK_DELETE,
                "CAPSLOCK", SDLK_CAPSLOCK,
                "NUMLOCK", SDLK_NUMLOCKCLEAR,
                "SCROLLLOCK", SDLK_SCROLLLOCK,
                "PAUSE", SDLK_PAUSE,
                "INSERT", SDLK_INSERT,
                "HOME", SDLK_HOME,
                "PAGEUP", SDLK_PAGEUP,
                "PAGEDOWN", SDLK_PAGEDOWN,
                "END", SDLK_END,
                "UP", SDLK_UP,
                "DOWN", SDLK_DOWN,
                "LEFT", SDLK_LEFT,
                "RIGHT", SDLK_RIGHT,
                "PRINTSCREEN", SDLK_PRINTSCREEN
            );
        
            // Function keys (F1 - F24)
            sol::table keycode_enum = lua["SDL_Keycode"];
            for (int i = 1; i <= 24; ++i) {
                keycode_enum["F" + std::to_string(i)] = SDL_Keycode(SDLK_F1 + (i - 1));
            }
        
            // Number and letter keys
            for (char c = '0'; c <= '9'; ++c) {
                keycode_enum[std::string(1, c)] = SDL_Keycode(SDLK_0 + (c - '0'));
            }
            for (char c = 'A'; c <= 'Z'; ++c) {
                keycode_enum[std::string(1, c)] = SDL_Keycode(SDLK_A + (c - 'A'));
            }
        
            // Keypad Keys
            keycode_enum["KP_0"] = SDLK_KP_0;
            keycode_enum["KP_1"] = SDLK_KP_1;
            keycode_enum["KP_2"] = SDLK_KP_2;
            keycode_enum["KP_3"] = SDLK_KP_3;
            keycode_enum["KP_4"] = SDLK_KP_4;
            keycode_enum["KP_5"] = SDLK_KP_5;
            keycode_enum["KP_6"] = SDLK_KP_6;
            keycode_enum["KP_7"] = SDLK_KP_7;
            keycode_enum["KP_8"] = SDLK_KP_8;
            keycode_enum["KP_9"] = SDLK_KP_9;
            keycode_enum["KP_PLUS"] = SDLK_KP_PLUS;
            keycode_enum["KP_MINUS"] = SDLK_KP_MINUS;
            keycode_enum["KP_MULTIPLY"] = SDLK_KP_MULTIPLY;
            keycode_enum["KP_DIVIDE"] = SDLK_KP_DIVIDE;
            keycode_enum["KP_ENTER"] = SDLK_KP_ENTER;
            keycode_enum["KP_EQUALS"] = SDLK_KP_EQUALS;
            keycode_enum["KP_DECIMAL"] = SDLK_KP_DECIMAL;
        
            // Modifier Keys
            keycode_enum["LSHIFT"] = SDLK_LSHIFT;
            keycode_enum["RSHIFT"] = SDLK_RSHIFT;
            keycode_enum["LCTRL"] = SDLK_LCTRL;
            keycode_enum["RCTRL"] = SDLK_RCTRL;
            keycode_enum["LALT"] = SDLK_LALT;
            keycode_enum["RALT"] = SDLK_RALT;
            keycode_enum["LGUI"] = SDLK_LGUI;
            keycode_enum["RGUI"] = SDLK_RGUI;
            keycode_enum["APP"] = SDLK_APPLICATION;
        
            // System & Media Keys
            keycode_enum["MEDIA_PLAY"] = SDLK_MEDIA_PLAY;
            keycode_enum["MEDIA_STOP"] = SDLK_MEDIA_STOP;
            keycode_enum["MUTE"] = SDLK_MUTE;
            keycode_enum["MEDIA_NEXT_TRACK"] = SDLK_MEDIA_NEXT_TRACK;
            keycode_enum["MEDIA_SELECT"] = SDLK_MEDIA_SELECT;
        }
     };
}