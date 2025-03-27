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

            sol::table keycode_enum = lua.create_table();
    
            keycode_enum["UNKNOWN"] = SDLK_UNKNOWN;
            keycode_enum["RETURN"] = SDLK_RETURN;
            keycode_enum["ESCAPE"] = SDLK_ESCAPE;
            keycode_enum["BACKSPACE"] = SDLK_BACKSPACE;
            keycode_enum["TAB"] = SDLK_TAB;
            keycode_enum["SPACE"] = SDLK_SPACE;
            keycode_enum["EXCLAIM"] = SDLK_EXCLAIM;
            keycode_enum["DBLAPOSTROPHE"] = SDLK_DBLAPOSTROPHE;
            keycode_enum["HASH"] = SDLK_HASH;
            keycode_enum["PERCENT"] = SDLK_PERCENT;
            keycode_enum["DOLLAR"] = SDLK_DOLLAR;
            keycode_enum["AMPERSAND"] = SDLK_AMPERSAND;
            keycode_enum["APOSTROPHE"] = SDLK_APOSTROPHE;
            keycode_enum["LEFTPAREN"] = SDLK_LEFTPAREN;
            keycode_enum["RIGHTPAREN"] = SDLK_RIGHTPAREN;
            keycode_enum["ASTERISK"] = SDLK_ASTERISK;
            keycode_enum["PLUS"] = SDLK_PLUS;
            keycode_enum["COMMA"] = SDLK_COMMA;
            keycode_enum["MINUS"] = SDLK_MINUS;
            keycode_enum["PERIOD"] = SDLK_PERIOD;
            keycode_enum["SLASH"] = SDLK_SLASH;
            keycode_enum["COLON"] = SDLK_COLON;
            keycode_enum["SEMICOLON"] = SDLK_SEMICOLON;
            keycode_enum["LESS"] = SDLK_LESS;
            keycode_enum["EQUALS"] = SDLK_EQUALS;
            keycode_enum["GREATER"] = SDLK_GREATER;
            keycode_enum["QUESTION"] = SDLK_QUESTION;
            keycode_enum["AT"] = SDLK_AT;
            keycode_enum["LEFTBRACKET"] = SDLK_LEFTBRACKET;
            keycode_enum["BACKSLASH"] = SDLK_BACKSLASH;
            keycode_enum["RIGHTBRACKET"] = SDLK_RIGHTBRACKET;
            keycode_enum["CARET"] = SDLK_CARET;
            keycode_enum["UNDERSCORE"] = SDLK_UNDERSCORE;
            keycode_enum["GRAVE"] = SDLK_GRAVE;
            keycode_enum["DELETE"] = SDLK_DELETE;
            keycode_enum["CAPSLOCK"] = SDLK_CAPSLOCK;
            keycode_enum["NUMLOCK"] = SDLK_NUMLOCKCLEAR;
            keycode_enum["SCROLLLOCK"] = SDLK_SCROLLLOCK;
            keycode_enum["PAUSE"] = SDLK_PAUSE;
            keycode_enum["INSERT"] = SDLK_INSERT;
            keycode_enum["HOME"] = SDLK_HOME;
            keycode_enum["PAGEUP"] = SDLK_PAGEUP;
            keycode_enum["PAGEDOWN"] = SDLK_PAGEDOWN;
            keycode_enum["END"] = SDLK_END;
            keycode_enum["UP"] = SDLK_UP;
            keycode_enum["DOWN"] = SDLK_DOWN;
            keycode_enum["LEFT"] = SDLK_LEFT;
            keycode_enum["RIGHT"] = SDLK_RIGHT;
            keycode_enum["PRINTSCREEN"] = SDLK_PRINTSCREEN;
                    
            // Function keys (F1 - F24)
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

            lua["Key"] = keycode_enum;
        }
     };
}