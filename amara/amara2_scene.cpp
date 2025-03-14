namespace Amara {
    class Scene: public Amara::Entity {
    public:
        Scene(): Entity() {
            entityID = "Scene";
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Scene>("Scene",
                sol::constructors<Scene()>(),
                sol::base_classes, sol::bases<Amara::Entity>()
            );

            sol::usertype<Entity> entity_type = lua["Entity"];
            entity_type["scene"] = &Entity::scene;
        }
    };
}