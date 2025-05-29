namespace Amara {
    class KeyboardManager {
    public:
        std::unordered_map<SDL_Keycode, Button> keys;

        SDL_Keycode lastKeyPressed = SDLK_UNKNOWN;
        bool keyPressed = false;

        SDL_Keycode lastKeyReleased = SDLK_UNKNOWN;
        bool keyReleased = false;
        
        void press(SDL_Keycode code) {
            if (keys.find(code) == keys.end()) {
                keys[code] = Button();
            }
            keys[code].press();
            lastKeyPressed = code;
            keyPressed = true;
        }

        void release(SDL_Keycode code) {
            if (keys.find(code) == keys.end()) {
                keys[code] = Button();
            }
            keys[code].release();
            lastKeyReleased = code;
            keyReleased = true;
        }

        void manage(double deltaTime) {
            keyPressed = false;
            keyReleased = false;
            
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

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<KeyboardManager>("Keyboard",
                "isDown", &KeyboardManager::isDown,
                "justPressed", &KeyboardManager::justPressed,
                "justReleased", &KeyboardManager::justReleased,
                "timeHeld", &KeyboardManager::timeHeld,
                "keyPressed", &KeyboardManager::keyPressed,
                "keyReleased", &KeyboardManager::keyReleased,
                "lastKeyPressed", &KeyboardManager::lastKeyPressed,
                "lastKeyReleased", &KeyboardManager::lastKeyReleased
            );

            sol::table keycode_enum = lua.create_table();
    
            keycode_enum["Unknown"] = SDLK_UNKNOWN;
            keycode_enum["Return"] = SDLK_RETURN;
            keycode_enum["Esc"] = SDLK_ESCAPE;
            keycode_enum["Backspace"] = SDLK_BACKSPACE;
            keycode_enum["Tasb"] = SDLK_TAB;
            keycode_enum["Space"] = SDLK_SPACE;
            keycode_enum["Exclaim"] = SDLK_EXCLAIM;
            keycode_enum["DoubleApostrophe"] = SDLK_DBLAPOSTROPHE;
            keycode_enum["Hash"] = SDLK_HASH;
            keycode_enum["Percent"] = SDLK_PERCENT;
            keycode_enum["Dollar"] = SDLK_DOLLAR;
            keycode_enum["Ampersand"] = SDLK_AMPERSAND;
            keycode_enum["Apostrophe"] = SDLK_APOSTROPHE;
            keycode_enum["LeftParenthesis"] = SDLK_LEFTPAREN;
            keycode_enum["RightParenthesis"] = SDLK_RIGHTPAREN;
            keycode_enum["Asterisk"] = SDLK_ASTERISK;
            keycode_enum["Plus"] = SDLK_PLUS;
            keycode_enum["Comma"] = SDLK_COMMA;
            keycode_enum["Minus"] = SDLK_MINUS;
            keycode_enum["Period"] = SDLK_PERIOD;
            keycode_enum["Slash"] = SDLK_SLASH;
            keycode_enum["Colon"] = SDLK_COLON;
            keycode_enum["SemiColon"] = SDLK_SEMICOLON;
            keycode_enum["Less"] = SDLK_LESS;
            keycode_enum["Equals"] = SDLK_EQUALS;
            keycode_enum["Greater"] = SDLK_GREATER;
            keycode_enum["Question"] = SDLK_QUESTION;
            keycode_enum["At"] = SDLK_AT;
            keycode_enum["LeftBracket"] = SDLK_LEFTBRACKET;
            keycode_enum["Backslash"] = SDLK_BACKSLASH;
            keycode_enum["RightBracket"] = SDLK_RIGHTBRACKET;
            keycode_enum["Caret"] = SDLK_CARET;
            keycode_enum["Underscore"] = SDLK_UNDERSCORE;
            keycode_enum["Grave"] = SDLK_GRAVE;
            keycode_enum["Delete"] = SDLK_DELETE;
            keycode_enum["CapsLock"] = SDLK_CAPSLOCK;
            keycode_enum["NumLock"] = SDLK_NUMLOCKCLEAR;
            keycode_enum["ScrollLock"] = SDLK_SCROLLLOCK;
            keycode_enum["Pause"] = SDLK_PAUSE;
            keycode_enum["Insert"] = SDLK_INSERT;
            keycode_enum["Home"] = SDLK_HOME;
            keycode_enum["PageUp"] = SDLK_PAGEUP;
            keycode_enum["PageDown"] = SDLK_PAGEDOWN;
            keycode_enum["End"] = SDLK_END;
            keycode_enum["Up"] = SDLK_UP;
            keycode_enum["Down"] = SDLK_DOWN;
            keycode_enum["Left"] = SDLK_LEFT;
            keycode_enum["Right"] = SDLK_RIGHT;
            keycode_enum["PrintScreen"] = SDLK_PRINTSCREEN;
                    
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
            keycode_enum["KP_Plus"] = SDLK_KP_PLUS;
            keycode_enum["KP_Minus"] = SDLK_KP_MINUS;
            keycode_enum["KP_Multiply"] = SDLK_KP_MULTIPLY;
            keycode_enum["KP_Divide"] = SDLK_KP_DIVIDE;
            keycode_enum["KP_Enter"] = SDLK_KP_ENTER;
            keycode_enum["KP_Equals"] = SDLK_KP_EQUALS;
            keycode_enum["KP_Decimal"] = SDLK_KP_DECIMAL;
        
            // Modifier Keys
            keycode_enum["LeftShift"] = SDLK_LSHIFT;
            keycode_enum["RightShift"] = SDLK_RSHIFT;
            keycode_enum["LeftCtrl"] = SDLK_LCTRL;
            keycode_enum["RightCtrl"] = SDLK_RCTRL;
            keycode_enum["LeftAlt"] = SDLK_LALT;
            keycode_enum["RightAlt"] = SDLK_RALT;
            keycode_enum["LGUI"] = SDLK_LGUI;
            keycode_enum["RGUI"] = SDLK_RGUI;
            keycode_enum["App"] = SDLK_APPLICATION;
        
            // System & Media Keys
            keycode_enum["MediaPlay"] = SDLK_MEDIA_PLAY;
            keycode_enum["MediaStop"] = SDLK_MEDIA_STOP;
            keycode_enum["Mute"] = SDLK_MUTE;
            keycode_enum["MediaNextTrack"] = SDLK_MEDIA_NEXT_TRACK;
            keycode_enum["MediaSelect"] = SDLK_MEDIA_SELECT;

            lua["Key"] = keycode_enum;
        }
     };
}