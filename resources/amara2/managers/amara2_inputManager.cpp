namespace Amara {
    class InputManager {
    public:
        bool logicBlocking = false;
        SDL_Event e;

        void handleEvents(
            std::vector<Amara::World*>& worlds,
            bool& quit
        ) {
            logicBlocking = false;

            while (SDL_PollEvent(&e) != 0) {
                switch (e.type) {
                    case SDL_EVENT_QUIT:
                        quit = true;
                        break;
                    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                        for (auto w: worlds) {
                            if (w->window != nullptr && w->windowID == e.window.windowID) {
                                w->destroy();
                            }
                        }
                        break;
                }
            }
        }
    };
}