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
        Cross,
        Circle,
        Triangle,
        Square,
        
        LeftStick,
        RightStick,

        Start,
        Select,
        Guide
    };

    class Gamepad {
    public:
        SDL_JoystickID* gamepad = nullptr;

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

        void press(SDL_GamepadButton _sdlbuttoncode) {
            switch (_sdlbuttoncode) {
                // TODO parse all possible buttons.
            };
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

        int connectedGamepadsCount = 0;

        void connectGamepad(SDL_JoystickID* _gamepad) {
            for (auto it = gamepads.begin(); it != gamepads.end(); it++) {
                if (it->gamepad == _gamepad || it->gamepad == nullptr) {
                    it->gamepad = _gamepad;
                    it->active = true;
                    it->reset();
                    connectedGamepadsCount += 1;
                    return;
                }
            }

            gamepads.push_back(Gamepad());
            Amara::Gamepad& gamepad = gamepads.back();
            gamepad.gamepad = _gamepad;
            gamepad.active = true;
            gamepad.reset();
            connectedGamepadsCount += 1;
        }

        void disconnectGamepad(SDL_JoystickID* _gamepad) {
            for (auto it = gamepads.begin(); it != gamepads.end(); it++) {
                if (it->gamepad == _gamepad) {
                    it->active = false;
                    it->gamepad = nullptr;
                    connectedGamepadsCount -= 1;
                    return;
                }
            }
        }

        Amara::Gamepad* getGamepad(int index) {
            if (index < gamepads.size()) {
                return &gamepads[index];
            }
            return nullptr;
        }
        
        void update(double deltaTime) {
            for (auto it = gamepads.begin(); it != gamepads.end(); it++) {
                if (it->active) {
                    it->manage(deltaTime);
                }
            }
        }
        
        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::GamepadManager>("GamepadManager",
                "getGamepad", &Amara::GamepadManager::getGamepad,
                "count", &Amara::GamepadManager::connectedGamepadsCount
            );
        }
    };
}