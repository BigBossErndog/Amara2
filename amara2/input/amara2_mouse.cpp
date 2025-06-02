namespace Amara {
    class Mouse: public Amara::Pointer {
    public:
        Amara::Button left;
        Amara::Button right;
        Amara::Button middle;
        
        Mouse() = default;

        void update(double deltaTime) {
            left.update(deltaTime);
            right.update(deltaTime);
            middle.update(deltaTime);

            state.update(deltaTime);
        }
        
        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Mouse>("MouseHandler",
                sol::base_classes, sol::bases<Amara::Pointer, Amara::Vector2>(),
                "left", &Mouse::left,
                "right", &Mouse::right,
                "middle", &Mouse::middle
            );
        }
    };
}