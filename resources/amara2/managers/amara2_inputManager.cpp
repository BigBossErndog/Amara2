namespace Amara {
    class InputManager {
    public:
        bool logicBlocking = false;
        SDL_Event e;

        KeyboardManager keyboard;

        InputManager() = default;

        void handleEvents(
            std::vector<Amara::World*>& worlds,
            GameManager& game
        ) {
            logicBlocking = false;

            keyboard.manage(game.deltaTime);
            
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
                        break;
                    case SDL_EVENT_KEY_UP:
                        keyboard.release(e.key.key);
                        break;
                }
            }
        }
    };
}