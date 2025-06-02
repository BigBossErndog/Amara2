namespace Amara {
    enum class GamepadButton {
        // Generic face buttons

        None,

        North,
        South,
        East,
        West,

        // Xbox and Nintendo labels
        A,
        B,
        X,
        Y,

        // PlayStation labels
        PSCross,
        PSCircle,
        PSTriangle,
        PSSquare,

        DpadUp,
        DpadDown,
        DpadLeft,
        DpadRight,

        LeftStick,
        RightStick,

        LeftStickUp,
        LeftStickDown,
        LeftStickLeft,
        LeftStickRight,

        RightStickUp,
        RightStickDown,
        RightStickLeft,
        RightStickRight,

        LeftShoulder,
        RightShoulder,

        LeftTrigger,
        RightTrigger,

        Guide,
        Back,
        Menu,

        Start,
        Select
    };

    class Gamepad {
    public:
        SDL_Gamepad* gamepad = nullptr;
        SDL_JoystickID gamepadID = 0;

        bool active = false;

        std::unordered_map<Amara::GamepadButton, Button> buttons;

        Amara::GameProps* gameProps = nullptr;

        bool buttonPressed = false;

        sol::object luaobject;

        Vector2 leftStick;
        Vector2 rightStick;

        double leftTrigger = 0;
        double rightTrigger = 0;

        float leftStickDeadzone = 0.2;
        float rightStickDeadzone = 0.2;

        float leftTriggerDeadzone = 0.2;
        float rightTriggerDeadzone = 0.2;

        Gamepad() {
            reset();
        }

        void reset() {
            gamepadID = 0;
            gamepad = nullptr;
            active = false;

            leftStick = Vector2(0, 0);
            rightStick = Vector2(0, 0);
            leftTrigger = 0;
            rightTrigger = 0;

            for (auto it = buttons.begin(); it != buttons.end(); it++) {
                it->second.reset();
            }
        }

        void setGamepad(SDL_JoystickID _gamepadID) {
            gamepad = nullptr;
            if (SDL_IsGamepad(_gamepadID)) {
                gamepadID = _gamepadID;
                gamepad = SDL_OpenGamepad(gamepadID);
                active = true;
            }

            if (gamepad == nullptr) {
                reset();
            }
            else {
                debug_log("Note: Gamepad connected \"", SDL_GetGamepadName(gamepad), "\" (ID: ", _gamepadID, ")");
            }
        }

        void press(Amara::GamepadButton _buttoncode) {
            if (buttons.find(_buttoncode) == buttons.end()) {
                buttons[_buttoncode] = Button();
            }
            buttons[_buttoncode].press();
        }

        void release(Amara::GamepadButton _buttoncode) {
            if (buttons.find(_buttoncode) == buttons.end()) {
                buttons[_buttoncode] = Button();
            }
            buttons[_buttoncode].release();
        }

        void activateButton(Amara::GamepadButton _buttoncode, bool isDown) {
            if (isDown) press(_buttoncode);
            else release(_buttoncode);
        }

        void activateSDLButton(SDL_GamepadButton _sdlbuttoncode, bool isDown) {
            switch (_sdlbuttoncode) {
                case SDL_GAMEPAD_BUTTON_SOUTH: {
                    activateButton(Amara::GamepadButton::South, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_EAST: {
                    activateButton(Amara::GamepadButton::East, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_NORTH: {
                    activateButton(Amara::GamepadButton::North, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_WEST: {
                    activateButton(Amara::GamepadButton::West, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_DPAD_LEFT: {
                    activateButton(Amara::GamepadButton::DpadLeft, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: {
                    activateButton(Amara::GamepadButton::DpadRight, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_DPAD_UP: {
                    activateButton(Amara::GamepadButton::DpadUp, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_DPAD_DOWN: {
                    activateButton(Amara::GamepadButton::DpadDown, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: {
                    activateButton(Amara::GamepadButton::LeftShoulder, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: {
                    activateButton(Amara::GamepadButton::RightShoulder, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_LEFT_STICK: {
                    activateButton(Amara::GamepadButton::LeftStick, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_RIGHT_STICK: {
                    activateButton(Amara::GamepadButton::RightStick, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_BACK: {
                    activateButton(Amara::GamepadButton::Back, isDown);
                    activateButton(Amara::GamepadButton::Select, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_START: {
                    activateButton(Amara::GamepadButton::Start, isDown);
                    activateButton(Amara::GamepadButton::Menu, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_GUIDE: {
                    activateButton(Amara::GamepadButton::Guide, isDown);
                    break;
                }
            }

            SDL_GamepadButtonLabel label = SDL_GetGamepadButtonLabel(gamepad, _sdlbuttoncode);

            switch (label) {
                case SDL_GAMEPAD_BUTTON_LABEL_A: {
                    activateButton(Amara::GamepadButton::A, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_LABEL_B: {
                    activateButton(Amara::GamepadButton::B, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_LABEL_X: {
                    activateButton(Amara::GamepadButton::X, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_LABEL_Y: {
                    activateButton(Amara::GamepadButton::Y, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_LABEL_CROSS: {
                    activateButton(Amara::GamepadButton::PSCross, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_LABEL_CIRCLE: {
                    activateButton(Amara::GamepadButton::PSCircle, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_LABEL_TRIANGLE: {
                    activateButton(Amara::GamepadButton::PSTriangle, isDown);
                    break;
                }
                case SDL_GAMEPAD_BUTTON_LABEL_SQUARE: {
                    activateButton(Amara::GamepadButton::PSSquare, isDown);
                    break;
                }
            }
        }

        bool isDown(Amara::GamepadButton _buttoncode) {
            if (buttons.find(_buttoncode) != buttons.end()) {
                return buttons[_buttoncode].isDown;
            }
            return false;
        }

        bool justPressed(Amara::GamepadButton _buttoncode) {
            if (buttons.find(_buttoncode) != buttons.end()) {
                return buttons[_buttoncode].justPressed;
            }
            return false;
        }

        bool justReleased(Amara::GamepadButton _buttoncode) {
            if (buttons.find(_buttoncode) != buttons.end()) {
                return buttons[_buttoncode].justReleased;
            }
            return false;
        }

        double timeHeld(Amara::GamepadButton _buttoncode) {
            if (buttons.find(_buttoncode) != buttons.end()) {
                return buttons[_buttoncode].timeHeld;
            }
            return 0;
        }

        void update(double deltaTime) {
            if (gamepad == nullptr) return;

            leftStick.x = (double)SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX) / (double)SDL_JOYSTICK_AXIS_MAX;
            leftStick.y = (double)SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY) / (double)SDL_JOYSTICK_AXIS_MAX;
            
            if (leftStick.x < -leftStickDeadzone) press(Amara::GamepadButton::LeftStickLeft);
            else release(Amara::GamepadButton::LeftStickLeft);
            if (leftStick.x > leftStickDeadzone) press(Amara::GamepadButton::LeftStickRight);
            else release(Amara::GamepadButton::LeftStickRight);

            if (leftStick.y < -leftStickDeadzone) press(Amara::GamepadButton::LeftStickUp);
            else release(Amara::GamepadButton::LeftStickUp);
            if (leftStick.y > leftStickDeadzone) press(Amara::GamepadButton::LeftStickDown);
            else release(Amara::GamepadButton::LeftStickDown);

            rightStick.x = (double)SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX) / (double)SDL_JOYSTICK_AXIS_MAX;
            rightStick.y = (double)SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY) / (double)SDL_JOYSTICK_AXIS_MAX;

            if (rightStick.x < -rightStickDeadzone) press(Amara::GamepadButton::RightStickLeft);
            else release(Amara::GamepadButton::RightStickLeft);
            if (rightStick.x > rightStickDeadzone) press(Amara::GamepadButton::RightStickRight);
            else release(Amara::GamepadButton::RightStickRight);

            if (rightStick.y < -rightStickDeadzone) press(Amara::GamepadButton::RightStickUp);
            else release(Amara::GamepadButton::RightStickUp);
            if (rightStick.y > rightStickDeadzone) press(Amara::GamepadButton::RightStickDown);
            else release(Amara::GamepadButton::RightStickDown);

            leftTrigger = (double)SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) / (double)SDL_JOYSTICK_AXIS_MAX;
            
            if (leftTrigger < -leftTriggerDeadzone) press(Amara::GamepadButton::LeftTrigger);
            else release(Amara::GamepadButton::LeftTrigger);

            rightTrigger = (double)SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) / (double)SDL_JOYSTICK_AXIS_MAX;
            
            if (rightTrigger < -rightTriggerDeadzone) press(Amara::GamepadButton::RightTrigger);
            else release(Amara::GamepadButton::RightTrigger);

            for (auto it = buttons.begin(); it != buttons.end(); it++) {
                Button& button = it->second;
                button.update(deltaTime);
            }
        }

        static void bind_lua(sol::state& lua) {
            lua.new_enum("Button",
                "North", GamepadButton::North,
                "South", GamepadButton::South,
                "East", GamepadButton::East,
                "West", GamepadButton::West,
                "A", GamepadButton::A,
                "B", GamepadButton::B,
                "X", GamepadButton::X,
                "Y", GamepadButton::Y,
                "PSCross", GamepadButton::PSCross,
                "PSCircle", GamepadButton::PSCircle,
                "PSTriangle", GamepadButton::PSTriangle,
                "PSSquare", GamepadButton::PSSquare,
                "LeftStick", GamepadButton::LeftStick,
                "RightStick", GamepadButton::RightStick,
                "LeftStickUp", GamepadButton::LeftStickUp,
                "LeftStickDown", GamepadButton::LeftStickDown,
                "LeftStickLeft", GamepadButton::LeftStickLeft,
                "LeftStickRight", GamepadButton::LeftStickRight,
                "RightStickUp", GamepadButton::RightStickUp,
                "RightStickDown", GamepadButton::RightStickDown,
                "RightStickLeft", GamepadButton::RightStickLeft,
                "RightStickRight", GamepadButton::RightStickRight,
                "LeftShoulder", GamepadButton::LeftShoulder,
                "RightShoulder", GamepadButton::RightShoulder,
                "LeftTrigger", GamepadButton::LeftTrigger,
                "RightTrigger", GamepadButton::RightTrigger,
                "DpadUp", GamepadButton::DpadUp,
                "DpadDown", GamepadButton::DpadDown,
                "DpadLeft", GamepadButton::DpadLeft,
                "DpadRight", GamepadButton::DpadRight,
                "Guide", GamepadButton::Guide,
                "Back", GamepadButton::Back,
                "Menu", GamepadButton::Menu,
                "Start", GamepadButton::Start,
                "Select", GamepadButton::Select
            );
            
            lua.new_usertype<Amara::Gamepad>("Gamepad",
                "isDown", &Amara::Gamepad::isDown,
                "justPressed", &Amara::Gamepad::justPressed,
                "justReleased", &Amara::Gamepad::justReleased,
                "timeHeld", &Amara::Gamepad::timeHeld,
                "leftStick", sol::readonly(&Amara::Gamepad::leftStick),
                "rightStick", sol::readonly(&Amara::Gamepad::rightStick),
                "leftTrigger", sol::readonly(&Amara::Gamepad::leftTrigger),
                "rightTrigger", sol::readonly(&Amara::Gamepad::rightTrigger),
                "leftStickDeadzone", &Amara::Gamepad::leftStickDeadzone,
                "rightStickDeadzone", &Amara::Gamepad::rightStickDeadzone,
                "leftTriggerDeadzone", &Amara::Gamepad::leftTriggerDeadzone,
                "rightTriggerDeadzone", &Amara::Gamepad::rightTriggerDeadzone
            );
        }
    };

    class GamepadManager {
    public:
        std::vector<Amara::Gamepad> gamepads;
        std::unordered_map<SDL_JoystickID, int> gamepadMap;

        int connectedGamepadsCount = 0;

        void connectGamepad(SDL_JoystickID _gamepadID) {
            for (int i = 0; i < gamepads.size(); i++) {
                Amara::Gamepad& gamepad = gamepads[i];
                if (gamepad.gamepadID == _gamepadID || gamepad.gamepadID == 0) {
                    gamepad.reset();
                    gamepad.setGamepad(_gamepadID);
                    if (gamepad.active) {
                        gamepadMap[_gamepadID] = i;
                        connectedGamepadsCount += 1;
                    }
                    return;
                }
            }

            gamepads.push_back(Gamepad());
            Amara::Gamepad& gamepad = gamepads[gamepads.size()-1];
            gamepad.reset();
            gamepad.setGamepad(_gamepadID);
            if (gamepad.active) {
                gamepadMap[_gamepadID] = gamepads.size()-1;
                connectedGamepadsCount += 1;
            }
        }

        void disconnectGamepad(SDL_JoystickID _gamepadID) {
            for (auto it = gamepads.begin(); it != gamepads.end(); it++) {
                if (it->gamepadID == _gamepadID) {
                    it->active = false;
                    it->reset();
                    if (gamepadMap.find(_gamepadID) != gamepadMap.end()) {
                        gamepadMap.erase(_gamepadID);
                    }
                    connectedGamepadsCount -= 1;
                    debug_log("Note: Gamepad disconnected. (ID: ", _gamepadID, ")");
                    return;
                }
            }
        }

        Amara::Gamepad* getGamepad(int index) {
            if (index >= 0 && index < gamepads.size()) {
                return &gamepads[index];
            }
            return nullptr;
        }

        Amara::Gamepad* getGamepadByID(SDL_JoystickID _gamepadID) {
            if (gamepadMap.find(_gamepadID) != gamepadMap.end()) {
                return getGamepad(gamepadMap[_gamepadID]);
            }
            return nullptr;
        }

        bool isDown(Amara::GamepadButton _buttoncode) {
            for (Amara::Gamepad& gamepad : gamepads) {
                if (gamepad.active && gamepad.isDown(_buttoncode)) {
                    return true;
                }
            }
            return false;
        }

        bool justPressed(Amara::GamepadButton _buttoncode) {
            for (Amara::Gamepad& gamepad : gamepads) {
                if (gamepad.active && gamepad.justPressed(_buttoncode)) {
                    return true;
                }
            }
            return false;
        }

        bool justReleased(Amara::GamepadButton _buttoncode) {
            bool result = false;
            for (Amara::Gamepad& gamepad : gamepads) {
                if (gamepad.active) {
                    if (gamepad.isDown(_buttoncode)) return false;
                    if (gamepad.justReleased(_buttoncode)) result = true;
                }
            }
            return result;
        }

        double timeHeld(Amara::GamepadButton _buttoncode) {
            double t = 0;
            double check;
            for (Amara::Gamepad& gamepad : gamepads) {
                if (gamepad.active) {
                    check = gamepad.timeHeld(_buttoncode);
                    if (check > t) t = check;
                }
            }
            return t;
        }
        
        void update(double deltaTime) {
            for (auto it = gamepads.begin(); it != gamepads.end(); it++) {
                if (it->active) {
                    it->update(deltaTime);
                }
            }
        }
        
        static void bind_lua(sol::state& lua) {
            Amara::Gamepad::bind_lua(lua);
            
            lua.new_usertype<Amara::GamepadManager>("GamepadManager",
                "getGamepad", &Amara::GamepadManager::getGamepad,
                "count", sol::readonly(&Amara::GamepadManager::connectedGamepadsCount),
                "isDown", &Amara::GamepadManager::isDown,
                "justPressed", &Amara::GamepadManager::justPressed,
                "justReleased", &Amara::GamepadManager::justReleased,
                "timeHeld", &Amara::GamepadManager::timeHeld
            );
        }
    };
}