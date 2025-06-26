namespace Amara {
    class Mouse: public Amara::Pointer {
    public:
        Amara::Button left;
        Amara::Button right;
        Amara::Button middle;

        Amara::Vector2 wheel;

        
        Mouse() = default;

        void update(double deltaTime) {
            left.update(deltaTime);
            right.update(deltaTime);
            middle.update(deltaTime);

            wheel = Vector2(0, 0);

            Amara::Pointer::update(deltaTime);
        }
        
        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Mouse>("MouseHandler",
                sol::base_classes, sol::bases<Amara::Pointer, Amara::Vector2>(),
                "left", sol::readonly(&Mouse::left),
                "right", sol::readonly(&Mouse::right),
                "middle", sol::readonly(&Mouse::middle),
                "wheel", sol::readonly(&Mouse::wheel)
            );
        }
    };
}