namespace Amara {
    class World: public Entity {
    public:
        World(): Entity() {
            baseEntityID = "World";
        }

        void update_properties() {
            Properties::world = this;
            Properties::lua()["world"] = get_lua_object();
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
            Entity::bindLua(lua);
            
            ScriptFactory::bindLua(lua);
            EntityFactory::bindLua(lua);
            
            lua.new_usertype<World>("World",
                sol::constructors<World()>(),
                sol::base_classes, sol::bases<Amara::Entity>(),
                "start", &World::start
            );
        }
    };
}