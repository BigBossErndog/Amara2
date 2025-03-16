namespace Amara {
    class EntityFactory {
    public:
        std::unordered_map<std::string, std::function<Entity*()>> factory;
        std::unordered_map<std::string, std::string> readScripts;
        std::unordered_map<std::string, sol::function> compiledScripts;
        static inline std::unordered_map<std::string, std::function<sol::object(Entity*)>> entityRegistry;

        sol::table props;

        void add(std::string key, std::string path) {
            if (factory.find(key) != factory.end()) {
                c_style_log("Error: \"%s\" is a reserved entity name.", key.c_str());
                return;
            }
            std::string script = GameProperties::files->getScriptPath(path);
            if (string_endsWith(script, ".lua")) {
                readScripts[key] = path;
            }
            else {
                compiledScripts[key] = GameProperties::files->load_script(path);
            }
        }

        Amara::Entity* create(std::string key) {
            auto it = factory.find(key);
            if (it != factory.end() && it->second) return (it->second())->init_build();
            
            if (compiledScripts.find(key) != compiledScripts.end()) {
                try {
                    sol::object result = compiledScripts[key]();
                    return result.as<Amara::Entity*>();
                }
                catch (const sol::error& e) {
                    log("Failed to create Entity \"", key, "\".");
                }
            }
            else if (readScripts.find(key) != readScripts.end()) {
                try {
                    sol::object result = GameProperties::files->run(readScripts[key]);
                    return result.as<Amara::Entity*>();
                }
                catch (const sol::error& e) {
                    log("Failed to create Entity \"", key, "\" from script \"", GameProperties::files->getScriptPath(readScripts[key]), "\".");
                }
            }
            else log("Entity \"", key, "\" was not found.");
            return nullptr;
        }
        
        sol::object luaCreate(std::string key) {
            Amara::Entity* entity = create(key);
            return entity->make_lua_object();
        }

        sol::object castLuaEntity(Amara::Entity* entity, std::string key) {
            auto it = entityRegistry.find(key);
            if (it != entityRegistry.end()) {
                return it->second(entity);
            }
            else c_style_log("Entity with key \"%s\" was not registered.", entity->entityID.c_str());
            return sol::lua_nil;
        }

        template <typename T>
        void registerEntity(std::string key) {
            factory[key] = []() -> T* { return new T(); };
            entityRegistry[key] = [](Entity* e) -> sol::object {
                if (T* derived = dynamic_cast<T*>(e)) {
                    return sol::make_object(*GameProperties::lua, derived);
                }
                return sol::lua_nil;
            };
        }

        void prepareEntities() {
            registerEntity<Entity>("Entity");
            registerEntity<Scene>("Scene");
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<EntityFactory>("EntityFactory",
                "add", &EntityFactory::add,
                "props", &EntityFactory::props,
                "create", &EntityFactory::luaCreate
            );
        }
    };

    sol::object Entity::luaAdd(std::string key) {
        Amara::Entity* entity = GameProperties::factory->create(key);
        add(entity);
        return entity->make_lua_object();
    }

    template <typename T>
    T Entity::as() {
        return dynamic_cast<T>(this);
    }
    sol::object Entity::make_lua_object() {
        return GameProperties::factory->castLuaEntity(this, entityID);
    }

    Amara::Scene* SceneManager::addSceneViaScript(std::string key, std::string path) {
        Scene* scene = nullptr;
        try {
            GameProperties::factory->add(key, path);
            scene = GameProperties::files->run(path).as<Amara::Scene*>()->init_build()->as<Amara::Scene*>();
            sceneMap[key] = scene;
            scenes.push_back(scene);
        }
        catch (const sol::error& e) {
            log("Failed to create Scene \"", key, "\" from script \"", GameProperties::files->getScriptPath(path), "\".");
        }
        return scene;
    }
}