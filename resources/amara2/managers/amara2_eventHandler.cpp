namespace Amara {
    class EventHandler {
    public:
        bool logicBlocking = false;
        SDL_Event e;

        KeyboardManager keyboard;
        GamepadManager gamepads;

        EventHandler() = default;

        void handleEvents(
            std::vector<Amara::World*>& worlds,
            GameManager& game
        ) {
            logicBlocking = false;

            keyboard.manage(game.deltaTime);
            gamepads.manage(game.deltaTime);
            
            while (SDL_PollEvent(&e) != 0) {
                switch (e.type) {
                    case SDL_EVENT_QUIT:
                        game.hasQuit = true;
                        break;
                    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                        for (auto w: worlds) {
                            if (w->window != nullptr && w->windowID == e.window.windowID) {
                                w->destroy();
                            }
                        }
                        break;
                    case SDL_EVENT_KEY_DOWN:
                        keyboard.press(e.key.key);
                        game.gameProps->messages->send(nullptr, "keyboarddown", sol::make_object(game.gameProps->lua, e.key.key));
                        break;
                    case SDL_EVENT_KEY_UP:
                        keyboard.release(e.key.key);
                        game.gameProps->messages->send(nullptr, "keyboardup", sol::make_object(game.gameProps->lua, e.key.key));
                        break;
                    case SDL_EVENT_GAMEPAD_ADDED:
                        gamepads.connectGamepad(e.gdevice.which);
                        break;
                    case SDL_EVENT_GAMEPAD_REMOVED:
                        gamepads.disconnectGamepad(e.gdevice.which);
                        break;
                    case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
                        Amara::Gamepad* gamepad = gamepads.getGamepadByID(e.gbutton.which);
                        if (gamepad) gamepad->activateSDLButton((SDL_GamepadButton)e.gbutton.button, true);
                        break;
                    }
                    case SDL_EVENT_GAMEPAD_BUTTON_UP: {
                        Amara::Gamepad* gamepad = gamepads.getGamepadByID(e.gbutton.which);
                        if (gamepad) gamepad->activateSDLButton((SDL_GamepadButton)e.gbutton.button, false);
                        break;
                    }
                }
            }
        }
    };
}