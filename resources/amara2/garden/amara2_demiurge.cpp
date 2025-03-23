namespace Amara {
    class Creator;
    
    class Demiurge {
    public:
        Creator* true_creator = nullptr;

        GameManager game;
        FileManager files;
        ScriptFactory scripts;
        EntityFactory factory;

        bool demiurgic = true;

        Demiurge() {}

        void override_existence() {
            Props::lua()["Creator"] = this;
            Props::lua()["Game"] = &game;
            Props::lua()["Files"] = &files;
            Props::lua()["Factory"] = &factory;
            Props::lua()["Scripts"] = &scripts;
            
            Props::files = &files;
            Props::factory = &factory;
            Props::scripts = &scripts;
        }

        void setup() {
            factory.prepareEntities();
            factory.registerEntity<World>("World");

            game.demiurgic = demiurgic;
        }

        void makePresenceKnown() {
            override_existence();
        }

        virtual World* createWorld(std::string key);
        virtual World* createWorld();

        static void bindLua(sol::state& lua) {
            "files", &Demiurge::files,
            "factory", &Demiurge::factory,
            "scripts", &Demiurge::scripts,
            lua.new_usertype<Demiurge>("Demiurge",
                "createWorld", [](Amara::Demiurge& d, sol::object key) -> sol::object {
                    World* world = nullptr;
                    if (key.is<std::string>()) {
                        world = d.createWorld(key.as<std::string>());
                    }
                    else world = d.createWorld();
                    if (world) return world->get_lua_object();
                    return sol::nil;
                }
            );
        }
    };
}