namespace Amara {
    class World: public Entity {
    public:
        World(): Entity() {
            entityID = "World";
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

        void update_properties() {
            if (props.valid()) {
                Properties::lua()["world"] = make_lua_object();
            }
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