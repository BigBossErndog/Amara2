namespace Amara {
    class Mouse: public Vector2 {
    public:
        Vector2 movement;

        Amara::Button left;
        Amara::Button right;
        Amara::Button middle;
        
        Mouse() = default;

        void handleMovement(const Vector2& _pos, const Vector2& _movement) {
            movement = _movement;
            x = _pos.x;
            y = _pos.y;
        }

        void update(double deltaTime) {
            left.update(deltaTime);
            right.update(deltaTime);
            middle.update(deltaTime);
        }
        
        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Mouse>("MouseHandler",
                sol::base_classes, sol::bases<Vector2>(),
                "movement", sol::readonly(&Mouse::movement),
                "left", &Mouse::left,
                "right", &Mouse::right,
                "middle", &Mouse::middle
            );
        }
    };
}