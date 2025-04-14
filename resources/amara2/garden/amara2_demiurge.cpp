namespace Amara {
    class Creator;
    
    class Demiurge {
    public:
        Creator* true_creator = nullptr;

        GameManager game;
        FileManager files;
        
        ScriptFactory scripts;
        NodeFactory factory;
        AnimationFactory animations;
        ControlManager controls;
        AudioMaster audio;

        std::vector<World*> worlds;

        bool demiurgic = true;

        Demiurge() {}

        void override_existence() {
            Props::lua()["Creator"] = this;
            Props::lua()["Game"] = &game;
            Props::lua()["Files"] = &files;
            Props::lua()["NodeFactory"] = &factory;
            Props::lua()["Scripts"] = &scripts;
            Props::lua()["Animations"] = &animations;
            Props::lua()["Controls"] = &controls;
            Props::lua()["AudioMaster"] = &audio;
            
            Props::files = &files;
            Props::factory = &factory;
            Props::scripts = &scripts;
            Props::animations = &animations;
            Props::controls = &controls;
            Props::audio = &audio;
        }

        void removeWorld(Amara::World* world) {
            for (auto it = worlds.begin(); it != worlds.end(); it++) {
                if (*it == world) {
                    worlds.erase(it);
                    break;
                }
            }
        }

        void addWorld(Amara::World* world) {
            worlds.push_back(world);
        }

        void setup() {
            factory.prepareEntities();
            factory.registerNode<World>("World");

            game.demiurgic = demiurgic;
        }

        void makePresenceKnown() {
            override_existence();
        }

        virtual World* createWorld(sol::object config);
        virtual World* createWorld();

        void destroyAllWorlds() {
            for (Amara::World* world: worlds) world->destroy();
        }

        virtual void newDemiurgicUniverse() {
            debug_log("Note: Demiurgic presence. Universe creation disabled.");
            debug_log("Control will be handed over in target builds.");
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Demiurge>("Demiurge",
                "game", &Demiurge::game,
                "files", &Demiurge::files,
                "factory", &Demiurge::factory,
                "scripts", &Demiurge::scripts,
                "createWorld", [](Amara::Demiurge& d, sol::object key) -> sol::object {
                    World* world = nullptr;
                    if (!key.is<sol::nil_t>()) {
                        world = d.createWorld(key);
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
        }
    };

    void Amara::World::removeFromDemiurge() {
        if (demiurge) demiurge->removeWorld(this);
        demiurge = nullptr;
    }
}