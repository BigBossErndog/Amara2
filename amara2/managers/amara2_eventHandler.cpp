namespace Amara {
    class EventHandler {
    public:
        bool logicBlocking = false;
        SDL_Event e;

        KeyboardManager keyboard;
        GamepadManager gamepads;

        Amara::GameProps* gameProps = nullptr;

        Amara::Pointer globalPointer;

        EventHandler() = default;

        void init(Amara::GameProps* _gameProps) {
            gameProps = _gameProps;
            gameProps->globalPointer = &globalPointer;
            
            SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
            SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
        }

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
                    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
                        for (auto w: worlds) {
                            if (w->window != nullptr && w->windowID == e.window.windowID) {
                                w->destroy();
                            }
                        }
                        break;
                    }
                    case SDL_EVENT_WINDOW_FOCUS_GAINED: {
                        for (auto w: worlds) {
                            if (w->window != nullptr && w->windowID == e.window.windowID) {
                                w->windowFocused = true;
                                
                                if (w->update_mouse) {
                                    Vector2 mousePos;
                                    float mx, my;
                                    int wx, wy;
                                    SDL_MouseButtonFlags mouseFlags = SDL_GetGlobalMouseState(&mx, &my);
                                    SDL_GetWindowPosition(w->window, &wx, &wy);
                                    mousePos.x = static_cast<float>(mx) - static_cast<float>(wx);
                                    mousePos.y = static_cast<float>(my) - static_cast<float>(wy);

                                    bool any_pressed = false;
                                    if (mouseFlags & SDL_BUTTON_LMASK) {
                                        w->inputManager.mouse.left.press();
                                        any_pressed = true;
                                    }
                                    if (mouseFlags & SDL_BUTTON_RMASK) {
                                        w->inputManager.mouse.right.press();
                                        any_pressed = true;
                                    }
                                    if (mouseFlags & SDL_BUTTON_MMASK) {
                                        w->inputManager.mouse.middle.press();
                                        any_pressed = true;
                                    }
                                    if (w->inputManager.pointerMode == Amara::InputMode::Touch) {
                                        w->inputManager.mouse.left.press();
                                        any_pressed = true;
                                    }
                                    if (any_pressed) {
                                        w->inputManager.handleMouseDown(mousePos);
                                        w->handleMouseMovement(mousePos, Vector2(0, 0));
                                        if (w->inputManager.mouse.left.justPressed) {
                                            w->inputManager.mouse.rec_position();
                                        }
                                        w->inputManager.force_release_pointer = true;
                                    }
                                }
                                w->update_mouse = false;
                            }
                        }
                        break;
                    }
                    case SDL_EVENT_WINDOW_FOCUS_LOST: {
                        for (auto w: worlds) {
                            if (w->window != nullptr && w->windowID == e.window.windowID) {
                                w->windowFocused = false;
                            }
                        }
                        break;
                    }
                    case SDL_EVENT_MOUSE_MOTION: {
                        for (auto w: worlds) {
                            if (e.motion.which == SDL_TOUCH_MOUSEID) {
                                continue;
                            }
                            if (w->window != nullptr && w->windowID == e.motion.windowID) {
                                w->handleMouseMovement(Vector2(e.motion.x, e.motion.y), Vector2(e.motion.xrel, e.motion.yrel));
                            }
                        }
                        break;
                    }
                    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                        for (auto w: worlds) {
                            if (e.button.which == SDL_TOUCH_MOUSEID) {
                                continue;
                            }
                            if (w->window != nullptr && w->windowID == e.button.windowID) {
                                switch (e.button.button) {
                                    case SDL_BUTTON_LEFT:
                                        w->inputManager.mouse.left.press();
                                        break;
                                    case SDL_BUTTON_RIGHT:
                                        w->inputManager.mouse.right.press();
                                        break;
                                    case SDL_BUTTON_MIDDLE:
                                        w->inputManager.mouse.middle.press();
                                        break;
                                }
                                w->inputManager.handleMouseDown(Vector2(e.button.x, e.button.y));
                                if (w->inputManager.mouse.left.justPressed) {
                                    w->inputManager.mouse.rec_position();
                                }
                            }
                        }
                        break;
                    }
                    case SDL_EVENT_MOUSE_BUTTON_UP: {
                        for (auto w: worlds) {
                            if (e.button.which == SDL_TOUCH_MOUSEID) {
                                continue;
                            }
                            if (w->window != nullptr && w->windowID == e.button.windowID) {
                                switch (e.button.button) {
                                    case SDL_BUTTON_LEFT:
                                        w->inputManager.mouse.left.release();
                                        break;
                                    case SDL_BUTTON_RIGHT:
                                        w->inputManager.mouse.right.release();
                                        break;
                                    case SDL_BUTTON_MIDDLE:
                                        w->inputManager.mouse.middle.release();
                                        break;
                                }
                                w->inputManager.handleMouseUp(Vector2(e.button.x, e.button.y));
                            }
                        }
                        break;
                    }
                    case SDL_EVENT_FINGER_DOWN: {
                        for (auto w: worlds) {
                            if (w->window != nullptr && w->windowID == e.tfinger.windowID) {
                                w->handleFingerEvent(Vector2(e.tfinger.x, e.tfinger.y), e.tfinger.fingerID, (SDL_EventType)e.type);
                            }
                        }
                        break;
                    }
                    case SDL_EVENT_FINGER_UP: {
                        for (auto w: worlds) {
                            if (w->window != nullptr && w->windowID == e.tfinger.windowID) {
                                w->handleFingerEvent(Vector2(e.tfinger.x, e.tfinger.y), e.tfinger.fingerID, (SDL_EventType)e.type);
                            }
                        }
                        break;
                    }
                    case SDL_EVENT_FINGER_MOTION: {
                        for (auto w: worlds) {
                            if (w->window != nullptr && w->windowID == e.tfinger.windowID) {
                                w->handleFingerEvent(Vector2(e.tfinger.x, e.tfinger.y), e.tfinger.fingerID, (SDL_EventType)e.type);
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

    bool InputManager::checkPointerHover(const Vector2& pos) {
        Amara::NodeInput* input;
        for (auto it = queue.rbegin(); it != queue.rend(); ++it) {
            input = *it;
            if (input->shape.collidesWith(pos)) {
                any_hovered = true;
                return true;
            }
        }
        any_hovered = false;
        return false;
    }

    void InputManager::forceReleasePointer() {
        if (world->window != nullptr) {
            Vector2 mousePos;
            float mx, my;
            int wx, wy;
            SDL_MouseButtonFlags mouseFlags = SDL_GetGlobalMouseState(&mx, &my);
            SDL_GetWindowPosition(world->window, &wx, &wy);
            mousePos.x = static_cast<float>(mx) - static_cast<float>(wx);
            mousePos.y = static_cast<float>(my) - static_cast<float>(wy);

            bool all_released = true;
            if ((mouseFlags & SDL_BUTTON_LMASK) == 0) {
                mouse.left.release();
            }
            else all_released = false;
            if ((mouseFlags & SDL_BUTTON_RMASK) == 0) {
                mouse.right.release();
            }
            else all_released = false;
            if ((mouseFlags & SDL_BUTTON_MMASK) == 0) {
                mouse.middle.release();
            }
            else all_released = false;
            
            if (all_released) {
                handleMouseUp(mousePos);
                force_release_pointer = false;
            }
        }
    }

    void InputManager::handleMouseMovement(const Vector2& pos) {
        pointerMode = Amara::InputMode::Mouse;

        *(gameProps->globalPointer) = mouse;

        Amara::NodeInput* input;
        any_hovered = false;
        for (auto it = queue.rbegin(); it != queue.rend(); ++it) {
            input = *it;
            if (input->shape.collidesWith(pos)) {
                input->lastPointer = &mouse;
                input->hover.press();

                if (input->hover.justPressed) {
                    input->hover_by_mouse = true;
                    input->handleMessage({ nullptr, "onMouseHover", mouse.get_lua_object(gameProps) });
                    input->handleMessage({ nullptr, "onPointerHover", mouse.get_lua_object(gameProps) });
                }
                any_hovered = true;
                break;
            }
        }
        gameProps->messages->send("onMouseMove", sol::make_object(gameProps->lua, pos));
    }
    void InputManager::handleMouseDown(const Amara::Vector2& point) {
        pointerMode = Amara::InputMode::Mouse;

        *(gameProps->globalPointer) = mouse;

        mouse.state.press();

        Amara::NodeInput* input;
        for (auto it = queue.rbegin(); it != queue.rend(); ++it) {
            input = *it;
            if (input->shape.collidesWith(point)) {
                input->lastPointer = &mouse;
                input->handleMessage({ nullptr, "onMouseDown", mouse.get_lua_object(gameProps) });
                if (mouse.left.justPressed) {
                    input->held = true;
                    input->handleMessage({ nullptr, "onLeftMouseDown", mouse.get_lua_object(gameProps) });
                    input->handleMessage({ nullptr, "onPointerDown", mouse.get_lua_object(gameProps) });

                    input->rec_interact_pos = input->node->pos;
                }
                else if (mouse.right.justPressed) {
                    input->handleMessage({ nullptr, "onRightMouseDown", mouse.get_lua_object(gameProps) });
                }
                else if (mouse.middle.justPressed) {
                    input->handleMessage({ nullptr, "onMiddleMouseDown", mouse.get_lua_object(gameProps) });
                }
                break;
            }
        }
    }
    void InputManager::handleMouseUp(const Amara::Vector2& point) {
        pointerMode = Amara::InputMode::Mouse;

        *(gameProps->globalPointer) = mouse;

        mouse.state.release();

        Amara::NodeInput* input;
        for (auto it = queue.rbegin(); it != queue.rend(); ++it) {
            input = *it;
            if (input->shape.collidesWith(point)) {
                input->handleMessage({ nullptr, "onMouseUp", mouse.get_lua_object(gameProps) });
                if (mouse.left.justReleased) {
                    input->handleMessage({ nullptr, "onLeftMouseUp", mouse.get_lua_object(gameProps) });
                    input->handleMessage({ nullptr, "onPointerUp", mouse.get_lua_object(gameProps) });
                }
                else if (mouse.right.justReleased) {
                    input->handleMessage({ nullptr, "onRightMouseUp", mouse.get_lua_object(gameProps) });
                }
                else if (mouse.middle.justReleased) {
                    input->handleMessage({ nullptr, "onMiddleMouseUp", mouse.get_lua_object(gameProps) });
                }
                break;
            }
        }
    }

    void InputManager::handleFingerEvent(const Amara::Vector2& pos, Pointer* finger, SDL_EventType eventType) {
        pointerMode = Amara::InputMode::Touch;

        *(gameProps->globalPointer) = *finger;
        
        Amara::NodeInput* input;
        for (auto it = queue.rbegin(); it != queue.rend(); ++it) {
            input = *it;
            if (input->shape.collidesWith(pos)) {
                input->lastPointer = finger;
                switch (eventType) {
                    case SDL_EVENT_FINGER_DOWN: {
                        input->hover.press();
                        input->handleMessage({ nullptr, "onPointerDown", finger->get_lua_object(gameProps) });
                        input->handleMessage({ nullptr, "onTouchDown", finger->get_lua_object(gameProps) });
                        if (input->hover.justPressed) {
                            input->held = true;
                            input->handleMessage({ nullptr, "onPointerHover", finger->get_lua_object(gameProps) });
                            input->handleMessage({ nullptr, "onTouchHover", finger->get_lua_object(gameProps) });

                            input->rec_interact_pos = input->node->pos;
                        }
                        break;
                    }
                    case SDL_EVENT_FINGER_UP: {
                        input->hover.release();
                        input->handleMessage({ nullptr, "onPointerUp", finger->get_lua_object(gameProps) });
                        input->handleMessage({ nullptr, "onTouchUp", finger->get_lua_object(gameProps) });
                        break;
                    }
                    case SDL_EVENT_FINGER_MOTION: {
                        input->hover.press();
                        if (input->hover.justPressed) {
                            input->handleMessage({ nullptr, "onPointerHover", finger->get_lua_object(gameProps) });
                            input->handleMessage({ nullptr, "onTouchHover", finger->get_lua_object(gameProps) });
                        }
                        break;
                    }
                }
                return;
            }
        }
    }
}