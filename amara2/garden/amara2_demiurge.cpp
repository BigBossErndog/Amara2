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
        sol::object luaobject;

        sol::function onError;

        int worldDepth = 0;

        Demiurge() {}

        virtual void override_existence() {
            gameProps->lua["Creator"] = luaobject;
            gameProps->lua["Game"] = game.luaobject;
            gameProps->lua["System"] = system.luaobject;
            gameProps->lua["Nodes"] = factory.luaobject;
            gameProps->lua["Scripts"] = scripts.luaobject;
            gameProps->lua["Controls"] = controls.luaobject;
            
            gameProps->game = &game;
            gameProps->system = &system;
            gameProps->factory = &factory;
            gameProps->scripts = &scripts;
            gameProps->controls = &controls;

            if (!base_dir_path.empty()) {
                gameProps->system->setBasePath(base_dir_path);
            }
        }

        virtual void unbind() {
            gameProps->lua["Game"] = sol::nil;
            game.luaobject = sol::object(sol::nil);

            gameProps->lua["System"] = sol::nil;
            system.luaobject = sol::object(sol::nil);

            gameProps->lua["Nodes"] = sol::nil;
            factory.luaobject = sol::object(sol::nil);
            
            gameProps->lua["Scripts"] = sol::nil;
            scripts.luaobject = sol::object(sol::nil);
            
            gameProps->lua["Controls"] = sol::nil;
            controls.clean_up();
            controls.luaobject = sol::object(sol::nil);
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
            luaobject = sol::make_object(gameProps->lua, this);

            game.gameProps = gameProps;
            game.luaobject = sol::make_object(gameProps->lua, &game);
            gameProps->game = &game;

            factory.gameProps = gameProps;
            factory.luaobject = sol::make_object(gameProps->lua, &factory);

            scripts.gameProps = gameProps;
            scripts.luaobject = sol::make_object(gameProps->lua, &scripts);

            system.gameProps = gameProps;
            system.luaobject = sol::make_object(gameProps->lua, &system);

            controls.init(gameProps);
            controls.luaobject = sol::make_object(gameProps->lua, &controls);

            system.getBasePath();

            factory.prepareNodes();
            factory.registerNode<World>("World");

            game.demiurgic = demiurgic;
        }

        virtual World* createWorld(sol::object config);
        virtual World* createWorld();

        void destroyAllWorlds() {
            for (Amara::World* world: worlds) world->destroy();
            worlds.clear();
        }

        void destroy() {
            destroyAllWorlds();
            controls.clearAllSchemes();
            factory.clear();
            scripts.clear();
            unbind();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Demiurge>("DemiurgeClass",
                "game", &Demiurge::game,
                "system", &Demiurge::system,
                "factory", &Demiurge::factory,
                "scripts", &Demiurge::scripts,
                "createWorld", [](Amara::Demiurge& d, sol::object key) -> sol::object {
                    World* world = nullptr;
                    if (key.valid()) {
                        world = d.createWorld(key);
                    }
                    else world = d.createWorld();
                    if (world) return world->get_lua_object();
                    return sol::nil;
                },
                "worldDepth", &Demiurge::worldDepth,
                "onError", &Demiurge::onError,
                "pause", [](Amara::Demiurge& d) {
                    d.paused = true;
                },
                "resume", [](Amara::Demiurge& d) {
                    d.paused = false;
                },
                "paused", &Demiurge::paused
            );
        }
    };

    void Amara::World::removeFromDemiurge() {
        if (demiurge) demiurge->removeWorld(this);
        demiurge = nullptr;
    }
}