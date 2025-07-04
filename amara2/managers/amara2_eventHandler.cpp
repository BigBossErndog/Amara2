namespace Amara {
    class EventHandler {
    public:
        bool logicBlocking = false;
        SDL_Event e;

        KeyboardManager keyboard;
        GamepadManager gamepads;

        Amara::GameProps* gameProps = nullptr;

        Amara::Pointer globalPointer;

        double backspace_counter = 0;
        bool backspace_held = false;
        double backspace_hold = 0.5;
        double backspace_period = 0.05;
        
        EventHandler() = default;

        void init(Amara::GameProps* _gameProps) {
            gameProps = _gameProps;
            gameProps->globalPointer = &globalPointer;
            
            SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
            SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
            
            gameProps->cursor_default = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
            gameProps->cursor_pointer = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
            if (gameProps->cursor_default) {
                SDL_SetCursor(gameProps->cursor_default);
            }
        }

        void handleEvents(
            std::vector<Amara::World*>& worlds,
            GameManager& game
        ) {
            logicBlocking = false;

            keyboard.update(game.deltaTime);
            gamepads.update(game.deltaTime);

            gameProps->text_input.clear();
            gameProps->text_input_type = TextInputEnum::None;
            
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
                    case SDL_EVENT_MOUSE_WHEEL: {
                        for (auto w: worlds) {
                            if (w->window != nullptr && w->windowID == e.wheel.windowID) {
                                w->inputManager.mouse.wheel = Vector2(e.wheel.x, e.wheel.y);
                            }
                        }
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
                        
                        if(e.key.key == SDLK_V && SDL_GetModState() & SDL_KMOD_CTRL) {
                            char* tempText{ SDL_GetClipboardText() };
                            gameProps->text_input = tempText;
                            SDL_free( tempText );

                            gameProps->text_input_type = TextInputEnum::Text;
                        }
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
                    case SDL_EVENT_TEXT_INPUT: {
                        char firstChar = static_cast<char>(toupper( e.text.text[ 0 ]));
                        if (!(SDL_GetModState() & SDL_KMOD_CTRL && ( firstChar == 'C' || firstChar == 'V' ))) {
                            gameProps->text_input = e.text.text;
                            gameProps->text_input_type = TextInputEnum::Text;
                        }
                        break;
                    }
                }
            }

            if (gameProps->recording_text_input() && keyboard.isDown(SDLK_BACKSPACE)) {
                if (keyboard.justPressed(SDLK_BACKSPACE)) {
                    gameProps->text_input_type = TextInputEnum::Backspace;
                    backspace_counter = 0;
                }
                else {
                    backspace_counter += game.deltaTime;
                    if (!backspace_held) {
                        if (backspace_counter >= backspace_hold) {
                            backspace_held = true;
                            gameProps->text_input_type = TextInputEnum::Backspace;
                            backspace_counter -= backspace_hold;
                        }
                    }
                    if (backspace_held) {
                        while (backspace_counter >= backspace_period) {
                            gameProps->text_input_type = TextInputEnum::Backspace;
                            backspace_counter -= backspace_period;
                        }
                    }
                }
            }
            else {
                backspace_counter = 0;
                backspace_held = false;
            }

            gameProps->system->setCursor(gameProps->current_cursor);
        }
    };

    bool InputManager::checkPointerHover(const Vector2& pos) {
        Amara::InputDef inputDef;
        gameProps->current_cursor = CursorEnum::Default;
        
        for (auto it = queue.rbegin(); it != queue.rend(); ++it) {
            inputDef = *it;
            if (inputDef.shape.collidesWith(pos) && Shape::collision(inputDef.viewport, pos)) {
                any_hovered = true;
                gameProps->current_cursor = inputDef.input->cursor;
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

        InputDef inputDef;
        NodeInput* input;

        any_hovered = false;
        gameProps->current_cursor = CursorEnum::Default;

        for (auto it = queue.rbegin(); it != queue.rend(); ++it) {
            inputDef = *it;
            input = inputDef.input;

            if (inputDef.shape.collidesWith(pos) && Shape::collision(inputDef.viewport, pos)) {
                inputDef.lastPointer = &mouse;
                input->lastInteraction = inputDef;
                
                input->hover.press();

                if (input->hover.justPressed) {
                    input->hover_by_mouse = true;
                    input->handleMessage({ nullptr, "onMouseHover", mouse.get_lua_object(gameProps) });
                    input->handleMessage({ nullptr, "onPointerHover", mouse.get_lua_object(gameProps) });
                }
                any_hovered = true;
                
                gameProps->current_cursor = inputDef.input->cursor;
                break;
            }
        }
        gameProps->messages->send("onMouseMove", sol::make_object(gameProps->lua, pos));
    }
    void InputManager::handleMouseDown(const Amara::Vector2& point) {
        pointerMode = Amara::InputMode::Mouse;

        *(gameProps->globalPointer) = mouse;

        mouse.state.press();

        InputDef inputDef;
        NodeInput* input;

        for (auto it = queue.rbegin(); it != queue.rend(); ++it) {
            inputDef = *it;
            input = inputDef.input;

            if (inputDef.shape.collidesWith(point) && Shape::collision(inputDef.viewport, point)) {
                inputDef.lastPointer = &mouse;
                input->lastInteraction = inputDef;

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

        Amara::InputDef inputDef;
        Amara::NodeInput* input;

        for (auto it = queue.rbegin(); it != queue.rend(); ++it) {
            inputDef = *it;
            input = inputDef.input;

            if (inputDef.shape.collidesWith(point) && Shape::collision(inputDef.viewport, point)) {
                inputDef.lastPointer = &mouse;
                input->lastInteraction = inputDef;
                
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
        Amara::InputDef inputDef;

        for (auto it = queue.rbegin(); it != queue.rend(); ++it) {
            inputDef = *it;
            input = inputDef.input;

            if (inputDef.shape.collidesWith(pos) && Shape::collision(inputDef.viewport, pos)) {
                inputDef.lastPointer = finger;
                input->lastInteraction = inputDef;

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