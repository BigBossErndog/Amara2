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
        SDL_Gamepad* gamepad = nullptr;

        bool active = false;

        std::vectors<GamepadButton, Button> buttons;

        Amara::GameProps* gameProps = nullptr;

        bool buttonPressed = false;

        sol::object luaobject;

        void press(GamepadButton _buttoncode) {
            if (buttons.find(_buttoncode) == buttons.end()) {
                buttons[_buttoncode] = Button();
            }
            buttons[_buttoncode].press();
        }

        void release(GameButton _buttoncode) {
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

        void manage(double deltaTime) {
            for (auto it = keys.begin(); it != keys.end(); it++) {
                Button& key = it->second;
                key.manage(deltaTime);
            }
        }

        static void bind_lua(sol::state& lua) {
            
        }
    };

    class GamepadManager {
        std::vector<Amara::Gamepad> gamepads;

        void connectGamepad(SDL_Gamepad* gamepad) {
            
        }

        void disconnectGamepad(SDL_Gamepad* _gamepad) {
            
        }

        void getGamepad(int index) {

        }
        
        void update() {

        }
    }
}