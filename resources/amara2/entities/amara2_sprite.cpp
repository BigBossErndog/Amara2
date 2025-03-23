namespace Amara {
    /**
     * This is used for 2D sprites.
     */
    class Sprite: public Amara::Entity {
    public:
        int frame = 0;

        Sprite(): Amara::Entity() {
            set_base_entity_id("Sprite");
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Sprite>("Sprite"
            
            );
        }
    };
}