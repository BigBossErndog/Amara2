namespace Amara {
    class Mouse: public Amara::Pointer {
    public:
        Amara::Button left;
        Amara::Button right;
        Amara::Button middle;

        Amara::Vector2 wheel;
        
        bool moved = false;

        GameProps* gameProps = nullptr;

        sol::object luaobject;
        
        Mouse() = default;
        Mouse(GameProps* _gameProps): gameProps(_gameProps) {}
        
        void update(double deltaTime) {
            left.update(deltaTime);
            right.update(deltaTime);
            middle.update(deltaTime);
            
            wheel = Vector2(0, 0);
            moved = false;
            
            Amara::Pointer::update(deltaTime);
        }

        void show() {
            SDL_ShowCursor();
        }
        void hide() {
            SDL_HideCursor();
        }

        void lock() {
            SDL_SetWindowRelativeMouseMode(gameProps->current_window, true);
        }
        void unlock() {
            SDL_SetWindowRelativeMouseMode(gameProps->current_window, false);
        }
        
        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Mouse>("MouseHandler",
                sol::base_classes, sol::bases<Amara::Pointer, Amara::Vector2>(),
                "left", sol::readonly(&Mouse::left),
                "right", sol::readonly(&Mouse::right),
                "middle", sol::readonly(&Mouse::middle),
                "wheel", sol::readonly(&Mouse::wheel),
                "moved", sol::readonly(&Mouse::moved),
                "show", &Mouse::show,
                "hide", &Mouse::hide,
                "visible", sol::property(
                    [](Amara::Mouse& m) {
                        return SDL_CursorVisible();
                    },
                    [](Amara::Mouse& m, bool visible) {
                        if (visible) m.show();
                        else m.hide();
                    }
                ),
                "lock", &Mouse::lock,
                "unlock", &Mouse::unlock,
                "locked", sol::property([](Amara::Mouse& m) {
                    return SDL_GetWindowRelativeMouseMode(m.gameProps->current_window);
                })
            );
        }
    };
}