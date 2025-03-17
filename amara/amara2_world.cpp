namespace Amara {
    class World: public Entity {
    public:
        World(): Entity() {
            baseEntityID = "World";
        }

        virtual void update_properties(double deltaTime) override {
            Properties::world = this;
            Properties::lua()["world"] = get_lua_object();
            Entity::update_properties(deltaTime);
        }

        virtual void preload() override {
            update_properties(0);
            Amara::Entity::preload();
        }

        virtual void create() override {
            update_properties(0);
        }
        
        virtual void update(double deltaTime) override {
            update_properties(deltaTime);
        }

        void start() {
            update_properties(0);
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