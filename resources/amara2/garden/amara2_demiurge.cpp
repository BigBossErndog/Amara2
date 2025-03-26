namespace Amara {
    class Creator;
    
    class Demiurge {
    public:
        Creator* true_creator = nullptr;

        GameManager game;
        FileManager files;
        AssetManager assets;
        ScriptFactory scripts;
        NodeFactory factory;
        AnimationFactory animations;

        bool demiurgic = true;

        Demiurge() {}

        void override_existence() {
            Props::lua()["Creator"] = this;
            Props::lua()["Game"] = &game;
            Props::lua()["Files"] = &files;
            Props::lua()["Assets"] = &assets;
            Props::lua()["NodeFactory"] = &factory;
            Props::lua()["Scripts"] = &scripts;
            Props::lua()["Animations"] = &animations;
            
            Props::files = &files;
            Props::assets = &assets;
            Props::factory = &factory;
            Props::scripts = &scripts;
            Props::animations = &animations;
        }

        void setup() {
            factory.prepareEntities();
            factory.registerNode<World>("World");

            game.demiurgic = demiurgic;
        }

        void makePresenceKnown() {
            override_existence();
        }

        virtual World* createWorld(std::string key);
        virtual World* createWorld();

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Demiurge>("Demiurge",
                "game", &Demiurge::game,
                "files", &Demiurge::files,
                "assets", &Demiurge::assets,
                "factory", &Demiurge::factory,
                "scripts", &Demiurge::scripts,
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

        virtual ~Demiurge() {
            factory.clear();
            scripts.clear();
            assets.clear();
        }
    };
}