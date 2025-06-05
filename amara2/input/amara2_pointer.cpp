namespace Amara {
    class Pointer: public Amara::Vector2 {
    public:
        Amara::Vector2 real_pos;
        Amara::Vector2 real_movement;

        Amara::Vector2 movement;

        Amara::Vector2 rec_pos;

        using Vector2::Vector2;

        bool active = false;

        unsigned int id = -1;
        
        Button state;

        sol::object luaobject;

        sol::object get_lua_object(Amara::GameProps* gameProps) {
            if (luaobject.valid()) return luaobject;
            luaobject = sol::make_object(gameProps->lua, this);
            return luaobject;
        }

        void rec_position(const Vector2& _pos) {
            x = _pos.x;
            y = _pos.y;
            rec_pos = _pos;
        }
        void rec_position() {
            rec_position(*this);
        }

        void handleMovement(const Vector2& _real_pos, const Vector2& _pos, const Vector2 _real_movement, const Vector2& _movement) {
            real_pos = _real_pos;
            real_movement = _real_movement;
            movement = _movement;
            
            x = _pos.x;
            y = _pos.y;
        }
        
        void update(double deltaTime) {
            state.update(deltaTime);
            real_movement = Vector2(0, 0);
            movement = Vector2(0, 0);
        }
        
        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::Pointer>("Pointer",
                sol::base_classes, sol::bases<Amara::Vector2>(),
                "id", sol::readonly(&Amara::Pointer::id),
                "movement", sol::readonly(&Amara::Pointer::movement),
                "active", sol::readonly(&Amara::Pointer::active),
                "state", sol::readonly(&Amara::Pointer::state),
                "real_pos", sol::readonly(&Amara::Pointer::real_pos),
                "real_movement", sol::readonly(&Amara::Pointer::real_movement)
            );
        }
    };
}