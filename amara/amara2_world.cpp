namespace Amara {
    class World: public Entity {
    public:
        World(): Entity() {
            entityID = "World";
            prepare();
        }

        void update_properties() {
            WorldProperties::world = this;

            if (props.valid()) {
                WorldProperties::lua()["world"] = make_lua_object();
            }
        }

        void prepare() {
            update_properties();
            init_build();
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
                sol::base_classes, sol::bases<Amara::Entity>()
            );
        }
    };
}