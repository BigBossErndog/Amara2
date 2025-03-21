namespace Amara {
    class Creator;

    class Demiurge {
    public:
        Creator* creator = nullptr;

        FileManager files;
        ScriptFactory scripts;
        EntityFactory factory;
        MessageQueue messages;

        Demiurge() {}

        void override_existence() {
            Properties::lua()["Creator"] = this;
            Properties::lua()["Files"] = &files;
            Properties::lua()["Factory"] = &factory;
            Properties::lua()["Scripts"] = &scripts;
        }

        void setup() {
            factory.prepareEntities();
            factory.registerEntity<World>("World");
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