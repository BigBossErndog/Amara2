namespace Amara {
    class Creator;
    
    class Demiurge {
    public:
        Creator* true_creator = nullptr;

        GameManager game;
        SystemManager system;
        
        ScriptFactory scripts;
        NodeFactory factory;
        ControlManager controls;

        std::vector<World*> worlds;

        bool demiurgic = true;
        bool paused = false;

        std::string base_dir_path;

        GameProps* gameProps = nullptr;

        Demiurge() {}

        void override_existence() {
            gameProps->lua["Creator"] = this;
            gameProps->lua["Game"] = &game;
            gameProps->lua["System"] = &system;
            gameProps->lua["NodeFactory"] = &factory;
            gameProps->lua["Scripts"] = &scripts;
            gameProps->lua["Controls"] = &controls;
            
            gameProps->system = &system;
            gameProps->factory = &factory;
            gameProps->scripts = &scripts;
            gameProps->controls = &controls;

            if (!base_dir_path.empty()) {
                gameProps->system->setBasePath(base_dir_path);
            }
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

        void setup(GameProps* _gameProps) {
            gameProps = _gameProps;

            game.gameProps = gameProps;
            factory.gameProps = gameProps;
            scripts.gameProps = gameProps;
            system.gameProps = gameProps;
            factory.gameProps = gameProps;
            controls.gameProps = gameProps;

            system.getBasePath();

            factory.prepareNodes();
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
            worlds.clear();
        }

        virtual void newDemiurgicUniverse() {
            debug_log("Note: Demiurgic presence. Universe creation disabled.");
            debug_log("Control will be handed over in target builds.");
        }

        void destroy() {
            destroyAllWorlds();
            controls.clearAllSchemes();
            factory.clear();
            scripts.clear();
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Demiurge>("Demiurge",
                "game", &Demiurge::game,
                "system", &Demiurge::system,
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
    };

    void Amara::World::removeFromDemiurge() {
        if (demiurge) demiurge->removeWorld(this);
        demiurge = nullptr;
    }
}