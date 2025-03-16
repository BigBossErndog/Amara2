namespace Amara {
    class World: public Scene {
    public:
        SceneManager scenes;
        EntityFactory factory;
        ScriptFactory scripts;

        FileManager files;
        MessageQueue messages;

        World(): Scene() {
            entityID = "World";
            prepare();
        }

        void update_properties() {
            WorldProperties::world = this;
            WorldProperties::factory = &factory;
            WorldProperties::scenes = &scenes;
            WorldProperties::scripts = &scripts;
            WorldProperties::files = &files;
            WorldProperties::messages = &messages;

            if (make_lua_object().valid()) {
                WorldProperties::lua()["world"] = make_lua_object();
            }
        }

        void prepare() {
            update_properties();
            
            factory.prepareEntities();
            factory.registerEntity<World>("World");

            init_build();
        }

        void execute(std::string command) {
            std::system(command.c_str());
        }

        virtual void init() override {
            update_properties();
            Scene::init();
        }
        void init(std::string key) {
            init();
            files.run(key);
        }
        
        static void bindLua(sol::state& lua) {
            bindLuaUtilityFunctions(lua);
            bindLuaGeometry(lua);

            FileManager::bindLua(lua);

            Entity::bindLua(lua);
            
            ScriptFactory::bindLua(lua);
            SceneManager::bindLua(lua);
            EntityFactory::bindLua(lua);
            
            lua.new_usertype<World>("World",
                sol::constructors<World()>(),
                sol::base_classes, sol::bases<Amara::Scene>(),
                "files", &World::files,
                "scenes", &World::scenes,
                "scripts", &World::scripts,
                "factory", &World::factory,
                "execute", &World::execute
            );
        }
    };
}