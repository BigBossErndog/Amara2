namespace Amara {
    enum class GamepadButton {
        // Generic face buttons

        North,
        South,
        East,
        West,

        A,
        B,
        X,
        Y,

        //
        PSCross,
        PSCircle,
        PSTriangle,
        PSSquare,
        
        LeftStick,
        RightStick,

        Start,
        Select,
        Guide
    };

    class Gamepad {
    public:
        SDL_JoystickID gamepad = 0;

        bool active = false;

        std::unordered_map<Amara::GamepadButton, Button> buttons;

        Amara::GameProps* gameProps = nullptr;

        bool buttonPressed = false;

        sol::object luaobject;

        void reset() {
            for (auto it = buttons.begin(); it != buttons.end(); it++) {
                it->second.reset();
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
                buttons[_buttoncode] = Button();
            }
            buttons[_buttoncode].release();
        }

        void sdl_press(SDL_GamepadButton _sdlbuttoncode) {
            switch (_sdlbuttoncode) {
                // TODO parse all possible buttons.
            };
        }
        void sdl_release(SDL_GamepadButton _sdlbuttoncode) {
            switch (_sdlbuttoncode) {
                // TODO parse all possible buttons
            }
        }

        bool isDown(Amara::GamepadButton _buttoncode) {
            if (buttons.find(_buttoncode) != buttons.end()) {
                buttons[_buttoncode].isDown;
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

        void manage(double deltaTime) {
            for (auto it = buttons.begin(); it != buttons.end(); it++) {
                Button& button = it->second;
                button.manage(deltaTime);
            }
        }

        static void bind_lua(sol::state& lua) {
            lua.new_enum("GamepadButton",
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
                "Start", GamepadButton::Start,
                "Select", GamepadButton::Select,
                "Guide", GamepadButton::Guide
            );

            lua.new_usertype<Amara::Gamepad>("Gamepad",
                "isDown", &Amara::Gamepad::isDown,
                "justPressed", &Amara::Gamepad::justPressed,
                "justReleased", &Amara::Gamepad::justReleased,
                "timeHeld", &Amara::Gamepad::timeHeld
            );
        }
    };

    class GamepadManager {
    public:
        std::vector<Amara::Gamepad> gamepads;
        std::unordered_map<SDL_JoystickID, Amara::Gamepad*> gamepadMap;

        int connectedGamepadsCount = 0;

        void connectGamepad(SDL_JoystickID _gamepad) {
            for (auto it = gamepads.begin(); it != gamepads.end(); it++) {
                if (it->gamepad == _gamepad || it->gamepad == 0) {
                    it->gamepad = _gamepad;
                    it->active = true;
                    it->reset();
                    gamepadMap[_gamepad] = &(*it);
                    connectedGamepadsCount += 1;
                    return;
                }
            }

            gamepads.push_back(Gamepad());
            Amara::Gamepad& gamepad = gamepads.back();
            gamepad.gamepad = _gamepad;
            gamepad.active = true;
            gamepad.reset();
            gamepadMap[_gamepad] = &gamepad;
            connectedGamepadsCount += 1;
        }

        void disconnectGamepad(SDL_JoystickID _gamepad) {
            for (auto it = gamepads.begin(); it != gamepads.end(); it++) {
                if (it->gamepad == _gamepad) {
                    it->active = false;
                    it->gamepad = 0;
                    if (gamepadMap.find(_gamepad) != gamepadMap.end()) {
                        gamepadMap.erase(_gamepad);
                    }
                    connectedGamepadsCount -= 1;
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

        Amara::Gamepad* getGamepadByID(SDL_JoystickID _gamepad) {
            if (gamepadMap.find(_gamepad) != gamepadMap.end()) {
                return gamepadMap[_gamepad];
            }
            return nullptr;
        }
        
        void manage(double deltaTime) {
            for (auto it = gamepads.begin(); it != gamepads.end(); it++) {
                if (it->active) {
                    it->manage(deltaTime);
                }
            }
        }
        
        static void bind_lua(sol::state& lua) {
            Amara::Gamepad::bind_lua(lua);
            
            lua.new_usertype<Amara::GamepadManager>("GamepadManager",
                "getGamepad", &Amara::GamepadManager::getGamepad,
                "count", &Amara::GamepadManager::connectedGamepadsCount
            );
        }
    };
}