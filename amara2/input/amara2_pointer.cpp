namespace Amara {
    class Pointer: public Amara::Vector2 {
    public:
        Amara::Vector2 realPos;
        Vector2 movement;

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

        void handleMovement(const Vector2& _realPos, const Vector2& _pos, const Vector2& _movement) {
            movement = _movement;
            realPos = _realPos;
            x = _pos.x;
            y = _pos.y;
        }

        void update(double deltaTime) {
            state.update(deltaTime);
        }
        
        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::Pointer>("Pointer",
                sol::base_classes, sol::bases<Amara::Vector2>(),
                "id", sol::readonly(&Amara::Pointer::id),
                "movement", sol::readonly(&Amara::Pointer::movement),
                "active", sol::readonly(&Amara::Pointer::active),
                "state", sol::readonly(&Amara::Pointer::state)
            );
        }
    };
}