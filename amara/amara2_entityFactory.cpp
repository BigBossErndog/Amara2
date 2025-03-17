namespace Amara {
    class EntityFactory {
    public:
        std::unordered_map<std::string, std::function<Entity*()>> factory;
        std::unordered_map<std::string, std::string> readScripts;
        std::unordered_map<std::string, sol::function> compiledScripts;
        static inline std::unordered_map<std::string, std::function<sol::object(Entity*)>> entityRegistry;

        sol::table props;

        bool exists(std::string key) {
            if (factory.find(key) != factory.end()) return true;
            if (compiledScripts.find(key) != compiledScripts.end())  return true;
            if (readScripts.find(key) != readScripts.end()) return true;

            return false;
        } 

        void add(std::string key, std::string path) {
            if (factory.find(key) != factory.end()) {
                log("Error: \"", key, "\" is a reserved entity name.");
                return;
            }
            std::string script = Properties::files->getScriptPath(path);
            if (string_endsWith(script, ".lua")) {
                readScripts[key] = path;
            }
            else {
                compiledScripts[key] = Properties::files->load_script(path);
            }
        }

        Amara::Entity* create(std::string key) {
            auto it = factory.find(key);
            if (it != factory.end() && it->second) return (it->second());
            
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
                    sol::object result = Properties::files->run(readScripts[key]);
                    return result.as<Amara::Entity*>();
                }
                catch (const sol::error& e) {
                    log("Failed to create Entity \"", key, "\" from script \"", Properties::files->getScriptPath(readScripts[key]), "\".");
                }
            }
            else log("Entity \"", key, "\" was not found.");
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
                log("Error: Entity type with key \"", entity->entityID, "\" was not registered.");
            }
            return sol::lua_nil;
        }

        template <typename T>
        void registerEntity(std::string key) {
            factory[key] = []() -> T* { return new T(); };
            entityRegistry[key] = [](Entity* e) -> sol::object {
                if (T* derived = dynamic_cast<T*>(e)) {
                    return sol::make_object(Properties::lua(), derived);
                }
                return sol::lua_nil;
            };
        }

        void prepareEntities() {
            registerEntity<Entity>("Entity");
            registerEntity<Scene>("Scene");
        }

        static void bindLua(sol::state& lua) {
            Amara::Entity::bindLua(lua);
            Amara::Scene::bindLua(lua);

            lua.new_usertype<EntityFactory>("EntityFactory",
                "add", &EntityFactory::add,
                "props", &EntityFactory::props,
                "create", &EntityFactory::luaCreate
            );
        }
    };

    sol::object Entity::luaAdd(std::string key) {
        Amara::Entity* entity = Properties::factory->create(key);
        add(entity);
        return entity->get_lua_object();
    }

    template <typename T>
    T Entity::as() {
        return dynamic_cast<T>(this);
    }
    sol::object Entity::get_lua_object() {
        if (luaobject.valid()) return luaobject;

        luaobject = Properties::factory->castLuaEntity(this, entityID);

        props = Properties::lua().create_table();

        sol::table props_meta = Properties::lua().create_table();
        props_meta["__newindex"] = [this](sol::table tbl, sol::object key, sol::object value) {
            if (value.is<sol::function>()) {
                sol::function callback = value.as<sol::function>();
                sol::function func = sol::make_object(Properties::lua(), [this, callback](sol::variadic_args va)->sol::object {
                    return callback(this->get_lua_object(), sol::as_args(va));
                });
                tbl.raw_set(key, func);
            }
            else tbl.raw_set(key, value);
        };

        props[sol::metatable_key] = props_meta;

        return luaobject;
    }
}