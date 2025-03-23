namespace Amara {
    class EntityFactory {
    public:
        std::unordered_map<std::string, std::function<Entity*()>> factory;
        std::unordered_map<std::string, std::string> readScripts;
        std::unordered_map<std::string, sol::function> compiledScripts;
        static inline std::unordered_map<std::string, std::function<sol::object(Entity*)>> entityRegistry;

        sol::table prop;

        bool exists(std::string key) {
            if (factory.find(key) != factory.end()) return true;
            if (compiledScripts.find(key) != compiledScripts.end())  return true;
            if (readScripts.find(key) != readScripts.end()) return true;

            return false;
        } 

        bool load(std::string key, std::string path) {
            if (factory.find(key) != factory.end()) {
                debug_log("Error: \"", key, "\" is a reserved entity name.");
                return false;
            }

            std::string script_path = Props::files->getScriptPath(path);

            if (!Props::files->fileExists(script_path)) {
                debug_log("Error: Failed to load Entity \"", key, "\" from \"", path, "\". File not found.");
                return false;
            }
            
            if (string_endsWith(script_path, ".lua")) {
                readScripts[key] = script_path;
            }
            else {
                compiledScripts[key] = Props::files->load_script(script_path);
            }

            return true;
        }

        Amara::Entity* prepEntity(Amara::Entity* entity, std::string key) {
            entity->entityID = key;
            entity->init();
            return entity;
        }

        Amara::Entity* create(std::string key) {
            auto it = factory.find(key);
            if (it != factory.end() && it->second) {
                return prepEntity(it->second(), key);
            }
            
            if (compiledScripts.find(key) != compiledScripts.end()) {
                try {
                    sol::object result = compiledScripts[key]();
                    return prepEntity(result.as<Amara::Entity*>(), key);
                }
                catch (const sol::error& e) {
                    debug_log("Failed to create Entity \"", key, "\".");
                }
            }
            else if (readScripts.find(key) != readScripts.end()) {
                try {
                    sol::object result = Props::files->run(readScripts[key]);
                    return prepEntity(result.as<Amara::Entity*>(), key);
                }
                catch (const sol::error& e) {
                    debug_log("Failed to create Entity \"", key, "\" from script \"", Props::files->getScriptPath(readScripts[key]), "\".");
                }
            }
            else debug_log("Entity \"", key, "\" was not found.");
            return nullptr;
        }
        
        sol::object luaCreate(std::string key) {
            Amara::Entity* entity = create(key);
            return entity->get_lua_object();
        }

        sol::object castLuaEntity(Amara::Entity* entity, std::string key) {
            auto it = entityRegistry.find(key);
            if (it != entityRegistry.end()) {
                return it->second(entity);
            }
            else {
                debug_log("Error: Entity type with key \"", entity->baseEntityID, "\" was not registered.");
            }
            return sol::lua_nil;
        }

        template <typename T>
        void registerEntity(std::string key) {
            factory[key] = []() -> T* { return new T(); };
            
            entityRegistry[key] = [](Entity* e) -> sol::object {
                if (T* derived = dynamic_cast<T*>(e)) {
                    return sol::make_object(Props::lua(), derived);
                }
                return sol::lua_nil;
            };
        }

        void prepareEntities() {
            registerEntity<Amara::Entity>("Entity");
            registerEntity<Amara::Camera>("Camera");
            registerEntity<Amara::Scene>("Scene");
            registerEntity<Amara::Action>("Action");
            registerEntity<Amara::Tween>("Tween");
            registerEntity<Amara::StateMachine>("StateMachine");
            registerEntity<Amara::World>("World");
        }

        static void bindLua(sol::state& lua) {
            Amara::Entity::bindLua(lua);
            Amara::Camera::bindLua(lua);
            Amara::Scene::bindLua(lua);
            Amara::Action::bindLua(lua);
            Amara::Tween::bindLua(lua);
            Amara::StateMachine::bindLua(lua);
            Amara::World::bindLua(lua);

            lua.new_usertype<EntityFactory>("EntityFactory",
                "load", &EntityFactory::load,
                "prop", &EntityFactory::prop,
                "create", &EntityFactory::luaCreate
            );
        }
    };

    Amara::Entity* Entity::createChild(std::string key) {
        Amara::Entity* entity = Props::factory->create(key);
        if (entity) addChild(entity);
        return entity;
    }

    sol::object Entity::luaCreateChild(std::string key) {
        Amara::Entity* entity = createChild(key);
        if (entity) return entity->get_lua_object();
        return sol::nil;
    }

    template <typename T>
    T Entity::as() {
        return dynamic_cast<T>(this);
    }
    sol::object Entity::get_lua_object() {
        if (luaobject.valid()) return luaobject;

        luaobject = Props::factory->castLuaEntity(this, baseEntityID);
        
        prop = Props::lua().create_table();

        sol::table prop_meta = Props::lua().create_table();
        
        prop_meta["__newindex"] = [this](sol::table tbl, sol::object key, sol::object value) {
            if (value.is<sol::function>()) {
                sol::function callback = value.as<sol::function>();
                sol::function func = sol::make_object(Props::lua(), [this, callback](sol::variadic_args va)->sol::object {
                    return callback(this->get_lua_object(), sol::as_args(va));
                });
                tbl.raw_set(key, func);
            }
            else tbl.raw_set(key, value);
        };
        prop[sol::metatable_key] = prop_meta;

        sol::userdata entityData = luaobject.as<sol::userdata>();
        sol::table metatable = entityData[sol::metatable_key];

        return luaobject;
    }
}