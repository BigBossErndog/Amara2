namespace Amara {
    class Scene;

    class Entity {
    public:
        std::string id;
        std::string entityID;

        Amara::Entity* parent = nullptr;
        Amara::Scene* scene = nullptr;

        std::vector<Amara::Entity*> children;
        
        sol::table props;
        sol::object luaobject;

        sol::function luaPreload;
        sol::function luaCreate;
        sol::function luaUpdate;
        
        MessageBox messages;

        Vector3 pos = { 0, 0, 0 };
        
        float depth = 0;
        bool lockDepthToY = false;

        Entity() {
            entityID = "Entity";
        }

        Amara::Entity* init_build() {
            if (!props.valid()) {
                props = WorldProperties::lua().create_table();

                sol::table props_meta = WorldProperties::lua().create_table();
                props_meta["__newindex"] = [this](sol::table tbl, sol::object key, sol::object value) {
                    if (value.is<sol::function>()) {
                        sol::function callback = value.as<sol::function>();
                        sol::function func = sol::make_object(WorldProperties::lua(), [this, callback](sol::variadic_args va)->sol::object {
                            return callback(this->make_lua_object(), sol::as_args(va));
                        });
                        tbl.raw_set(key, func);
                    }
                    else tbl.raw_set(key, value);
                };

                props[sol::metatable_key] = props_meta;
            }
            if (!luaobject.valid()) {
                luaobject = make_lua_object();
            }

            return this;
        }

        virtual void init() {
            if (luaCreate.valid()) {
                try {
                    luaCreate(make_lua_object());
                }
                catch (const sol::error& e) {
                    log(entityID, ": \"", id, "\" error on create.");
                }
            }
        }

        virtual Amara::Entity* configure(nlohmann::json config) {
            if (config.is_string()) {
                std::string path = config.get<std::string>();
                if (string_endsWith(path, ".json")) {
                    configure(WorldProperties::files->readJSON(path));
                    return this;
                }
                if (string_endsWith(path, ".lua") || string_endsWith(path, ".luac")) {
                    configure(lua_to_json(WorldProperties::scripts->run(path)));
                    return this;
                }
            }
            if (json_has(config, "x")) pos.x = config["x"];
            if (json_has(config, "y")) pos.y = config["y"];
            if (json_has(config, "z")) pos.z = config["z"];
            return this;
        }
        Amara::Entity* configure(std::string key, nlohmann::json value) {
            nlohmann::json obj;
            obj[key] = value;
            return configure(obj);
        }
        sol::object super_configure(sol::object config) {
            configure(lua_to_json(config));
            return make_lua_object();
        }

        sol::function configure_override;
        sol::object luaConfigure(sol::object config) {
            if (config.is<std::string>()) {
                std::string path = config.as<std::string>();
                if (string_endsWith(path, ".json")) {
                    luaConfigure(json_to_lua(WorldProperties::files->readJSON(path)));
                    return make_lua_object();
                }
                if (string_endsWith(path, ".lua") || string_endsWith(path, ".luac")) {
                    luaConfigure(WorldProperties::scripts->run(path));
                    return make_lua_object();
                }
            }
            if (configure_override.valid()) {
                try {
                    configure_override(this, config);
                }
                catch (const sol::error& e) {
                    c_style_log("%s: \"%s\" error on configure().", entityID.c_str(), id.c_str());
                }
            }
            else configure(lua_to_json(config));
            return make_lua_object();
        }
        sol::object luaConfigure(std::string key, sol::object val) {
            sol::table config = WorldProperties::lua().create_table();
            config[key] = val;
            return luaConfigure(config);
        }

        virtual void preload() {
            if (luaPreload.valid()) {
                try {
                    luaPreload(*this);
                }
                catch (const sol::error& e) {
                    c_style_log("%s: \"%s\" error on preload().", entityID.c_str(), id.c_str());
                }
            }
        }

        virtual void update() {}

        virtual void run(float delta) {
            messages.run();

            update();
            if (luaUpdate.valid()) {
                try {
                    luaUpdate(make_lua_object(), delta);
                }
                catch (const sol::error& e) {
                    c_style_log("%s: \"%s\" error on update().", entityID.c_str(), id.c_str());
                }
            }

            if (lockDepthToY) depth = pos.y;
        }

        virtual void draw() {

        } 

        Amara::Entity* add(Amara::Entity* entity) {
            entity->scene = scene;
            entity->parent = this;
            children.push_back(entity);
            entity->init();
            return entity;
        }
        sol::object luaAdd(std::string);

        sol::object get(std::string key) {
            if (props[key].valid() && props[key].is<sol::function>()) {
                sol::function func = props[key];
                return sol::make_object(WorldProperties::lua(), [this, &func](sol::table table, sol::variadic_args va, sol::this_state s) -> sol::object {
                    return func(this->make_lua_object(), sol::as_args(va));
                });
            }
            return props[key];
        }
        void set(std::string key, sol::object obj) {
            props[key] = obj;
        }

        template <typename T>
        T as();

        sol::object make_lua_object();

        static void bindLua(sol::state& lua) {
            sol::usertype<Entity> entity_type = lua.new_usertype<Entity>("Entity",
                sol::constructors<Entity()>(),
                "pos", &Entity::pos,
                "id", &Entity::id,
                "entityID", sol::readonly(&Entity::entityID),
                "parent", sol::readonly(&Entity::parent),
                "props", &Entity::props,
                "configure", sol::overload(
                    sol::resolve<sol::object(sol::object)>(&Entity::luaConfigure),
                    sol::resolve<sol::object(std::string, sol::object)>(&Entity::luaConfigure)
                ),
                "configure_override", &Entity::configure_override,
                "super_configure", &Entity::super_configure,
                "depth", &Entity::depth,
                "lockDepthToY", &Entity::lockDepthToY,
                "onPreload", &Entity::luaPreload,
                "onCreate", &Entity::luaCreate,
                "onUpdate", &Entity::luaUpdate,
                "createChild", &Entity::luaAdd,
                "addChild", &Entity::add,
                "get", &Entity::get,
                "set", &Entity::set
            );

            lua.new_usertype<std::vector<Amara::Entity*>>("EntityVector",
                "size", &std::vector<Amara::Entity*>::size,
                "push", [](std::vector<Amara::Entity*>& vec, Amara::Entity* entity) {
                    vec.push_back(entity);
                },
                "get", [](std::vector<Amara::Entity*>& vec, size_t index) -> sol::object {
                    if (index > 0 && index <= vec.size()) {
                        return vec[index-1]->make_lua_object();
                    }
                    return nullptr;
                },
                "find", [](std::vector<Amara::Entity*>& vec, std::string gid) -> sol::object {
                    for (Amara::Entity* entity: vec) {
                        if (string_equal(entity->id, gid)) {
                            return entity->make_lua_object();
                        }
                    }
                    return nullptr;
                },
                "remove", [](std::vector<Amara::Entity*>& vec, size_t index) {
                    if (index > 0 && index <= vec.size()) {
                        vec.erase(vec.begin() + index-1);
                    }
                }
            );

            entity_type["children"] = &Entity::children;
        }
    };
}