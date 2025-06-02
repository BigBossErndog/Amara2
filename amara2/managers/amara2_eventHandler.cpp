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

            keyboard.update(game.deltaTime);
            gamepads.update(game.deltaTime);
            
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
                    case SDL_EVENT_MOUSE_MOTION: {
                        for (auto w: worlds) {
                            if (w->window != nullptr && w->windowID == e.motion.windowID) {
                                w->handleMouseMovement(Vector2(e.motion.x, e.motion.y), Vector2(e.motion.xrel, e.motion.yrel));
                            }
                        }
                        break;
                    }
                    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                        for (auto w: worlds) {
                            if (w->window != nullptr && w->windowID == e.button.windowID) {
                                if (e.button.button == SDL_BUTTON_LEFT) {
                                    w->inputManager.mouse.left.press();
                                }
                                else if (e.button.button == SDL_BUTTON_RIGHT) {
                                    w->inputManager.mouse.right.press();
                                }
                                else if (e.button.button == SDL_BUTTON_MIDDLE) {
                                    w->inputManager.mouse.middle.press();
                                }
                                w->inputManager.handleMouseDown(Vector2(e.button.x, e.button.y));
                            }
                        }
                        break;
                    }
                    case SDL_EVENT_MOUSE_BUTTON_UP: {
                        for (auto w: worlds) {
                            if (w->window != nullptr && w->windowID == e.button.windowID) {
                                if (e.button.button == SDL_BUTTON_LEFT) {
                                    w->inputManager.mouse.left.release();
                                }
                                else if (e.button.button == SDL_BUTTON_RIGHT) {
                                    w->inputManager.mouse.right.release();
                                }
                                else if (e.button.button == SDL_BUTTON_MIDDLE) {
                                    w->inputManager.mouse.middle.release();
                                }
                                w->inputManager.handleMouseUp(Vector2(e.button.x, e.button.y));
                            }
                        }
                        break;
                    }
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