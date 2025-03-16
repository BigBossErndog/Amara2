namespace Amara {
    class Scene: public Amara::Entity {
    public:
        Scene(): Entity() {
            entityID = "Scene";
        }

        void update_properties() {
            if (props.valid()) Properties::lua()["scene"] = make_lua_object();
        }

        virtual void preload() override {
            update_properties();
            Amara::Entity::preload();
        }

        virtual void create() override {
            update_properties();
        }

        virtual void update() override {
            update_properties();
        }
        
        static void bindLua(sol::state& lua) {
            lua.new_usertype<Scene>("Scene",
                sol::constructors<Scene()>(),
                sol::base_classes, sol::bases<Amara::Entity>()
            );

            sol::usertype<Entity> entity_type = lua["Entity"];
            entity_type["scene"] = sol::readonly(&Entity::scene);
        }
    };
}