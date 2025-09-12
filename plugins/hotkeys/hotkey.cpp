class HotkeyConfig {
public:
    std::string name;
    std::vector<SDL_Keycode> keys;

    bool pressed() {
        for (SDL_Keycode k: keys) {
            if (!keyPressed(k)) return false;
        }
        return true;
    }
    
    #ifdef _WIN32
    static bool is_key_pressed(int val) {
        return (GetAsyncKeyState(val) & 0x8000) != 0;
    }

    static int SDLKeyToWindowsVK(SDL_Keycode sdlKey) {
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

    static bool keyPressed(SDL_Keycode _k) {
        #ifdef _WIN32
        return is_key_pressed(SDLKeyToWindowsVK(_k));
        #elif __APPLE__
        return is_key_pressed(SDLKeyToMacKeyCode(_k));
        #endif
    }
    #elif __APPLE__
    static bool is_key_pressed(CGKeyCode val) {
        return CGEventSourceKeyState(kCGEventSourceStateHIDSystemState, val);
    }

    static CGKeyCode SDLKeyToMacKeyCode(SDL_Keycode sdlKey) {
        switch (sdlKey) {
            case SDLK_A: return kVK_ANSI_A;
            case SDLK_B: return kVK_ANSI_B;
            case SDLK_C: return kVK_ANSI_C;
            case SDLK_D: return kVK_ANSI_D;
            case SDLK_E: return kVK_ANSI_E;
            case SDLK_F: return kVK_ANSI_F;
            case SDLK_G: return kVK_ANSI_G;
            case SDLK_H: return kVK_ANSI_H;
            case SDLK_I: return kVK_ANSI_I;
            case SDLK_J: return kVK_ANSI_J;
            case SDLK_K: return kVK_ANSI_K;
            case SDLK_L: return kVK_ANSI_L;
            case SDLK_M: return kVK_ANSI_M;
            case SDLK_N: return kVK_ANSI_N;
            case SDLK_O: return kVK_ANSI_O;
            case SDLK_P: return kVK_ANSI_P;
            case SDLK_Q: return kVK_ANSI_Q;
            case SDLK_R: return kVK_ANSI_R;
            case SDLK_S: return kVK_ANSI_S;
            case SDLK_T: return kVK_ANSI_T;
            case SDLK_U: return kVK_ANSI_U;
            case SDLK_V: return kVK_ANSI_V;
            case SDLK_W: return kVK_ANSI_W;
            case SDLK_X: return kVK_ANSI_X;
            case SDLK_Y: return kVK_ANSI_Y;
            case SDLK_Z: return kVK_ANSI_Z;

            case SDLK_0: return kVK_ANSI_0;
            case SDLK_1: return kVK_ANSI_1;
            case SDLK_2: return kVK_ANSI_2;
            case SDLK_3: return kVK_ANSI_3;
            case SDLK_4: return kVK_ANSI_4;
            case SDLK_5: return kVK_ANSI_5;
            case SDLK_6: return kVK_ANSI_6;
            case SDLK_7: return kVK_ANSI_7;
            case SDLK_8: return kVK_ANSI_8;
            case SDLK_9: return kVK_ANSI_9;

            case SDLK_KP_0: return kVK_ANSI_Keypad0;
            case SDLK_KP_1: return kVK_ANSI_Keypad1;
            case SDLK_KP_2: return kVK_ANSI_Keypad2;
            case SDLK_KP_3: return kVK_ANSI_Keypad3;
            case SDLK_KP_4: return kVK_ANSI_Keypad4;
            case SDLK_KP_5: return kVK_ANSI_Keypad5;
            case SDLK_KP_6: return kVK_ANSI_Keypad6;
            case SDLK_KP_7: return kVK_ANSI_Keypad7;
            case SDLK_KP_8: return kVK_ANSI_Keypad8;
            case SDLK_KP_9: return kVK_ANSI_Keypad9;

            case SDLK_LCTRL: return kVK_Control;
            case SDLK_RCTRL: return kVK_RightControl;
            case SDLK_LSHIFT: return kVK_Shift;
            case SDLK_RSHIFT: return kVK_RightShift;
            case SDLK_LALT: return kVK_Option;
            case SDLK_RALT: return kVK_RightOption;

            case SDLK_UP: return kVK_UpArrow;
            case SDLK_DOWN: return kVK_DownArrow;
            case SDLK_LEFT: return kVK_LeftArrow;
            case SDLK_RIGHT: return kVK_RightArrow;

            case SDLK_F1: return kVK_F1;
            case SDLK_F2: return kVK_F2;
            case SDLK_F3: return kVK_F3;
            case SDLK_F4: return kVK_F4;
            case SDLK_F5: return kVK_F5;
            case SDLK_F6: return kVK_F6;
            case SDLK_F7: return kVK_F7;
            case SDLK_F8: return kVK_F8;
            case SDLK_F9: return kVK_F9;
            case SDLK_F10: return kVK_F10;
            case SDLK_F11: return kVK_F11;
            case SDLK_F12: return kVK_F12;
            case SDLK_F13: return kVK_F13;
            case SDLK_F14: return kVK_F14;
            case SDLK_F15: return kVK_F15;
            case SDLK_F16: return kVK_F16;
            case SDLK_F17: return kVK_F17;
            case SDLK_F18: return kVK_F18;
            case SDLK_F19: return kVK_F19;
            case SDLK_F20: return kVK_F20;

            case SDLK_RETURN: return kVK_Return;
            case SDLK_ESCAPE: return kVK_Escape;
            case SDLK_BACKSPACE: return kVK_Delete;
            case SDLK_TAB: return kVK_Tab;
            case SDLK_SPACE: return kVK_Space;
            case SDLK_DELETE: return kVK_ForwardDelete;
            case SDLK_INSERT: return kVK_Help;
            case SDLK_HOME: return kVK_Home;
            case SDLK_END: return kVK_End;
            case SDLK_PAGEUP: return kVK_PageUp;
            case SDLK_PAGEDOWN: return kVK_PageDown;

            case SDLK_COMMA: return kVK_ANSI_Comma;
            case SDLK_PERIOD: return kVK_ANSI_Period;
            case SDLK_SLASH: return kVK_ANSI_Slash;
            case SDLK_SEMICOLON: return kVK_ANSI_Semicolon;
            case SDLK_APOSTROPHE: return kVK_ANSI_Quote;
            case SDLK_LEFTBRACKET: return kVK_ANSI_LeftBracket;
            case SDLK_RIGHTBRACKET: return kVK_ANSI_RightBracket;
            case SDLK_BACKSLASH: return kVK_ANSI_Backslash;
            case SDLK_MINUS: return kVK_ANSI_Minus;
            case SDLK_EQUALS: return kVK_ANSI_Equal;
            case SDLK_GRAVE: return kVK_ANSI_Grave;

            case SDLK_KP_PERIOD: return kVK_ANSI_KeypadDecimal;
            case SDLK_KP_DIVIDE: return kVK_ANSI_KeypadDivide;
            case SDLK_KP_MULTIPLY: return kVK_ANSI_KeypadMultiply;
            case SDLK_KP_MINUS: return kVK_ANSI_KeypadMinus;
            case SDLK_KP_PLUS: return kVK_ANSI_KeypadPlus;
            case SDLK_KP_ENTER: return kVK_ANSI_KeypadEnter;

            case SDLK_CAPSLOCK: return kVK_CapsLock;

            case SDLK_LGUI: return kVK_Command;
            case SDLK_RGUI: return kVK_Command;

            default:
                return 0;
        }
    }

    static bool keyPressed(SDL_Keycode _k) {
        return is_key_pressed(SDLKeyToMacKeyCode(_k));
    }
    #endif 
};

class Hotkey: public Amara::Action {
public:
    bool pressed = false;
    bool rec_pressed = false;

    bool justPressed = false;
    bool justReleased = false;

    double timeHeld = 0;

    std::vector<HotkeyConfig> configs;

    Hotkey(): Amara::Action() {
        set_base_node_id("Hotkey");
    }

    virtual Amara::Node* configure(nlohmann::json config) {
        if (Amara::json_has(config, "keys")) {
            HotkeyConfig new_config;
            nlohmann::json keys_json = config["keys"];
            if (keys_json.is_array()) {
                for (int i = 0; i < keys_json.size(); i++) {
                    new_config.keys.push_back((SDL_Keycode)keys_json[i].get<int>());
                }
            }
            else {
                new_config.keys.push_back((SDL_Keycode)keys_json.get<int>());
            }
            config.clear();
            configs.push_back(new_config);
        }
        else if (Amara::json_has(config, "config")) {
            nlohmann::json configs = config["config"];
            if (configs.is_object()) {
                
            }
        }
        else if (Amara::json_has(config, "key")) {
            HotkeyConfig new_config;
            new_config.keys.push_back((SDL_Keycode)config["key"].get<int>());
            config.clear();
            configs.push_back(new_config);
        }

        return Amara::Action::configure(config);
    }

    virtual void act(double deltaTime) {
        rec_pressed = pressed;
        pressed = true;

        justPressed = false;
        justReleased = false;

        if (configs.size() > 0) {
            for (HotkeyConfig& config: configs) {
                if (!config.pressed()) pressed = false;
            }
        }
        else pressed = false;

        if (pressed) {
            timeHeld += deltaTime;
            if (!rec_pressed) {
                timeHeld = 0;
                justPressed = true;
                if (funcs.hasFunction("onPress")) funcs.callFunction(actor, "onPress");
            }
            if (funcs.hasFunction("whilePressed")) funcs.callFunction(actor, "whilePressed");
        }
        if (rec_pressed) {
            if (funcs.hasFunction("onRelease")) funcs.callFunction(actor, "onRelease");
        }
    }
    
    static void bind_lua(sol::state& lua) {
        lua.new_usertype<Hotkey>("Hotkey",
            "pressed", sol::readonly(&Hotkey::pressed),
            "isDown", sol::readonly(&Hotkey::pressed),
            "justPressed", sol::readonly(&Hotkey::justPressed),
            "justReleased", sol::readonly(&Hotkey::justReleased),
            "timeHeld", sol::readonly(&Hotkey::timeHeld)
        );
    }
};