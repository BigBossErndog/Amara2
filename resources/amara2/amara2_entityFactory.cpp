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

        bool load(std::string key, std::string path) {
            if (factory.find(key) != factory.end()) {
                log("Error: \"", key, "\" is a reserved entity name.");
                return false;
            }

            std::string script_path = Properties::files->getScriptPath(path);

            if (!Properties::files->fileExists(script_path)) {
                log("Error: Failed to load Entity \"", key, "\" from \"", path, "\". File not found.");
                return false;
            }
            
            if (string_endsWith(script_path, ".lua")) {
                readScripts[key] = path;
            }
            else {
                compiledScripts[key] = Properties::files->load_script(script_path);
            }

            return true;
        }

        Amara::Entity* prepEntity(Amara::Entity* entity, std::string key) {
            entity->entityID = key;
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
                    log("Failed to create Entity \"", key, "\".");
                }
            }
            else if (readScripts.find(key) != readScripts.end()) {
                try {
                    sol::object result = Properties::files->run(readScripts[key]);
                    return prepEntity(result.as<Amara::Entity*>(), key);
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
                log("Error: Entity type with key \"", entity->baseEntityID, "\" was not registered.");
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
            registerEntity<Camera>("Camera");
            registerEntity<Scene>("Scene");
            registerEntity<Action>("Action");
            registerEntity<Tween>("Tween");
        }

        static void bindLua(sol::state& lua) {
            Amara::StateManager::bindLua(lua);

            Amara::Entity::bindLua(lua);
            Amara::Camera::bindLua(lua);
            Amara::Scene::bindLua(lua);
            Amara::Action::bindLua(lua);
            Amara::Tween::bindLua(lua);

            lua.new_usertype<EntityFactory>("EntityFactory",
                "load", &EntityFactory::load,
                "props", &EntityFactory::props,
                "create", &EntityFactory::luaCreate
            );
        }
    };

    Amara::Entity* Entity::createChild(std::string key) {
        Amara::Entity* entity = Properties::factory->create(key);
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

        luaobject = Properties::factory->castLuaEntity(this, baseEntityID);

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

        sol::userdata entityData = luaobject.as<sol::userdata>();
        sol::table metatable = entityData[sol::metatable_key];

        if (!metatable["__indexing_overridden"].valid()) {
            metatable["__indexing_overridden"] = true;

            sol::function old_indexer = metatable["__index"];
            metatable["__index_rec"] = old_indexer;
            metatable["__index"] = [old_indexer](Amara::Entity& e, sol::object key) -> sol::object {
                if (e.props[key].valid()) {
                    return e.props[key];
                }
                return old_indexer(e, key);
            };

            // sol::function old_newindexer = metatable["__newindex"];
            // metatable["__newindex"] = [old_indexer](sol::table self, sol::object key, sol::object value) {
            //     sol::table metatable = obj.as<sol::userdata>()[sol::metatable_key];

            //     if (key.is<std::string>()) {
            //         std::string key_str = key.as<std::string>();

            //         if (metatable[key_str].valid()) {
            //             old_newindexer(obj, key, value);
            //             return;
            //         }

            //         // Walk up the metatable chain
            //         sol::table parent_meta = metatable;
            //         while (parent_meta.valid()) {
            //             sol::object indexer = parent_meta["__index"];

            //             if (indexer.is<sol::table>()) {
            //                 parent_meta = indexer.as<sol::table>();

            //                 if (parent_meta[key_str].valid()) {
            //                     // If found in parent, update it there
            //                     parent_meta[key_str] = value;
            //                     return;
            //                 }
            //             } else {
            //                 break;  // Stop if __index is not a table
            //             }
            //         }
            //     }


            //     Amara::Entity& e = self.as<Amara::Entity&>();
            //     e.props
            // };
        }

        return luaobject;
    }
}