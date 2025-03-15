namespace Amara {
    class EntityFactory {
    public:
        std::unordered_map<std::string, std::function<Entity*()>> factory;
        std::unordered_map<std::string, std::string> entityScripts;
        static inline std::unordered_map<std::string, std::function<sol::object(Entity*)>> entityRegistry;

        sol::table props;

        void add(std::string key, std::string script) {
            if (factory.find(key) != factory.end()) {
                c_style_log("Error: \"%s\" is a reserved entity name.", key.c_str());
                return;
            }
            entityScripts[key] = script;
        }

        Amara::Entity* create(std::string key) {
            auto it = factory.find(key);
            if (it != factory.end() && it->second) return (it->second())->init_build();
            
            if (entityScripts.find(key) == entityScripts.end()) {
                c_style_log("Error: Entity \"%s\" has not been defined.", key.c_str());
                return nullptr;
            }
            try {
                return (GameProperties::files->run(entityScripts[key]).as<Amara::Entity*>())->init_build();
            }
            catch (const sol::error& e) {
                c_style_log("EntityFactory error: Failed to create entity \'%s\'", key.c_str());
            }
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
    T* Entity::as() {
        return dynamic_cast<T*>(this);
    }
    sol::object Entity::make_lua_object() {
        return GameProperties::factory->castLuaEntity(this, entityID);
    }

    Amara::Scene* SceneManager::addSceneViaScript(std::string key, std::string path) {
        Scene* scene = nullptr;
        try {
            GameProperties::factory->add(key, path);
            scene = GameProperties::files->run(path).as<Amara::Scene*>();
            sceneMap[key] = scene;
            scenes.push_back(scene);
        }
        catch (const sol::error& e) {
            c_style_log("EntityFactory error: Failed to create scene \'%s\'", key.c_str());
        }
        return scene;
    }
}