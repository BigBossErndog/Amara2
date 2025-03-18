namespace Amara {
    class World: public Entity {
    public:
        World(): Entity() {
            baseEntityID = "World";
            world = this;
        }

        virtual void update_properties() override {
            Properties::world = this;
            Properties::lua()["world"] = get_lua_object();
            Entity::update_properties();
        }

        virtual void preload() override {
            update_properties();
            Amara::Entity::preload();
        }

        virtual void create() override {
            update_properties();
        }
        
        virtual void update(double deltaTime) override {
            update_properties();
        }

        void start() {
            update_properties();
            Entity::init();
        }
        
        static void bindLua(sol::state& lua) {
            EntityFactory::bindLua(lua);
            ScriptFactory::bindLua(lua);
            
            lua.new_usertype<World>("World",
                sol::constructors<World()>(),
                sol::base_classes, sol::bases<Amara::Entity>(),
                "start", &World::start
            );
        }
    };
}