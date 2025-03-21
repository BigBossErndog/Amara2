namespace Amara {
    class Demiurge;

    class World: public Entity {
    public:
        Amara::Demiurge* demiurge = nullptr;

        std::string base_dir_path;

        World(): Entity() {
            set_base_entity_id("World");
            world = this;
        }

        virtual void update_properties() override {
            Props::world = this;
            Entity::update_properties();
        }

        virtual void run(double deltaTime) override {
            if (!base_dir_path.empty()) {
                Props::files->setBasePath(base_dir_path);
            }
            Amara::Entity::run(deltaTime);
        }
        
        static void bindLua(sol::state& lua) {    
            lua.new_usertype<World>("World",
                sol::constructors<World()>(),
                sol::base_classes, sol::bases<Amara::Entity>(),
                "base_dir_path", &World::base_dir_path
            );

            sol::usertype<Entity> entity_type = lua["Entity"];
            entity_type["world"] = sol::readonly(&Entity::world);
        }
    };
}