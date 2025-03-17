namespace Amara {
    class Scene;

    class Entity {
    public:
        std::string id;
        std::string entityID;
        std::string baseEntityID;

        Amara::Entity* parent = nullptr;
        Amara::Scene* scene = nullptr;

        std::vector<Amara::Entity*> children;
        std::vector<Amara::Entity*> children_copy_list;
        
        sol::table props;
        sol::object luaobject;

        sol::function luaPreload;
        sol::function luaCreate;
        sol::function luaUpdate;
        
        MessageBox messages;

        Vector3 pos = { 0, 0, 0 };
        
        float depth = 0;
        bool lockDepthToY = false;

        bool isDestroyed = false;
        bool isPaused = false;

        Entity() {
            baseEntityID = "Entity";
            get_lua_object();
        }

        virtual void create() {}
        virtual void init() {
            create();
            if (luaCreate.valid()) {
                try {
                    luaCreate(get_lua_object());
                }
                catch (const sol::error& e) {
                    log("Error: On ", baseEntityID, ": \"", id, "\" while executing onCreate().");
                }
            }
        }

        virtual Amara::Entity* configure(nlohmann::json config) {
            if (config.is_string()) {
                std::string path = config.get<std::string>();
                if (string_endsWith(path, ".json")) {
                    configure(Properties::files->readJSON(path));
                    return this;
                }
                if (string_endsWith(path, ".lua") || string_endsWith(path, ".luac")) {
                    configure(lua_to_json(Properties::scripts->run(path)));
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
            return get_lua_object();
        }

        sol::function configure_override;
        sol::object luaConfigure(sol::object config) {
            if (config.is<std::string>()) {
                std::string path = config.as<std::string>();
                if (string_endsWith(path, ".json")) {
                    luaConfigure(json_to_lua(Properties::files->readJSON(path)));
                    return get_lua_object();
                }
                if (string_endsWith(path, ".lua") || string_endsWith(path, ".luac")) {
                    luaConfigure(Properties::scripts->run(path));
                    return get_lua_object();
                }
            }
            if (configure_override.valid()) {
                try {
                    configure_override(this, config);
                }
                catch (const sol::error& e) {
                    log("Error: On ", baseEntityID, ": \"", id, "\" while executing configure().");
                }
            }
            else configure(lua_to_json(config));
            return get_lua_object();
        }
        sol::object luaConfigure(std::string key, sol::object val) {
            sol::table config = Properties::lua().create_table();
            config[key] = val;
            return luaConfigure(config);
        }

        virtual void preload() {
            if (luaPreload.valid()) {
                try {
                    luaPreload(*this);
                }
                catch (const sol::error& e) {
                    log("Error: On ", baseEntityID, ": \"", id, "\" while executing onPreload().");
                }
            }
        }
        
        virtual void update(double deltaTime) {}
        virtual void update_properties(double deltaTime) {}

        virtual void run(double deltaTime) {
            messages.run();

            update(deltaTime);
            if (luaUpdate.valid()) {
                try {
                    luaUpdate(get_lua_object(), deltaTime);
                }
                catch (const sol::error& e) {
                    log("Error: On ", baseEntityID, ": \"", id, "\" while executing onUpdate().");
                }
            }

            if (lockDepthToY) depth = pos.y;

            if (!isDestroyed) runChildren(deltaTime);
            cleanEntityList(children);
        }
        void runChildren(double deltaTime) {
            children_copy_list = children;

            Amara::Entity* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
				update_properties(deltaTime);

                child = *it;
				if (child == nullptr || child->isDestroyed || child->parent != this || child->isPaused) {
					++it;
					continue;
				}
				child->run(deltaTime);
				++it;
				if (isDestroyed) break;
			}
        }

        virtual void draw() {}

        void sortChildren();

        Amara::Entity* add(Amara::Entity* entity) {
            entity->scene = scene;
            entity->parent = this;
            children.push_back(entity);
            entity->init();
            return entity;
        }
        sol::object luaAdd(std::string);

        template <typename T>
        T as();

        sol::object get_lua_object();

        sol::function to_string_override;
        explicit operator std::string() const {
            return string_concat(
                "(", baseEntityID, ", ", 
                entityID, ": \"",
                id, "\") "
            );
        }
        friend std::ostream& operator<<(std::ostream& os, const Entity& e) {
            return os << static_cast<std::string>(e);
        }

        static void cleanEntityList(std::vector<Amara::Entity*>& list) {
            Amara::Entity* entity;
			for (auto it = list.begin(); it != list.end();) {
				entity = *it;
				if (entity == nullptr || entity->isDestroyed) {
					it = list.erase(it);
					continue;
				}
				++it;
			}
        }

        static void bindLua(sol::state& lua) {
            sol::usertype<Entity> entity_type = lua.new_usertype<Entity>("Entity",
                sol::constructors<Entity()>(),
                "pos", &Entity::pos,
                "id", &Entity::id,
                "baseEntityID", sol::readonly(&Entity::baseEntityID),
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
                "isDestroyed", sol::readonly(&Entity::isDestroyed),
                "string", [](Amara::Entity* e){
                    return std::string(*e);
                }
            );

            lua.new_usertype<std::vector<Amara::Entity*>>("EntityVector",
                "size", &std::vector<Amara::Entity*>::size,
                "push", [](std::vector<Amara::Entity*>& vec, Amara::Entity* entity) {
                    vec.push_back(entity);
                },
                "get", [](std::vector<Amara::Entity*>& vec, size_t index) -> sol::object {
                    std::vector<Amara::Entity*> copylist = vec;
                    cleanEntityList(copylist);
                    if (index > 0 && index <= vec.size()) {
                        return copylist[index-1]->get_lua_object();
                    }
                    return nullptr;
                },
                "find", [](std::vector<Amara::Entity*>& vec, std::string gid) -> sol::object {
                    for (Amara::Entity* entity: vec) {
                        if (entity->isDestroyed) continue;
                        if (string_equal(entity->id, gid)) {
                            return entity->get_lua_object();
                        }
                    }
                    return nullptr;
                },
                "remove", [](std::vector<Amara::Entity*>& vec, size_t index) {
                    if (index > 0 && index <= vec.size()) {
                        vec.erase(vec.begin() + index-1);
                    }
                },
                "string", [](std::vector<Amara::Entity*>& vec) -> std::string {
                    std::string output;
                    for (int i = 0; i < vec.size(); i++) {
                        output += std::string(*vec[i]);
                        if (i < vec.size()-1) {
                            output += "\n";
                        }
                    }
                    return output;
                }
            );

            entity_type["children"] = sol::readonly(&Entity::children);
        }
    };

    bool is_entity(sol::object obj) {
        return obj.is<Amara::Entity>();
    }
    std::string entity_to_string(sol::object obj) {
        return std::string(obj.as<Amara::Entity>());
    } 
}