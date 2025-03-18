namespace Amara {
    class Scene;
    class World;

    class Entity {
    public:
        std::string id;
        std::string entityID;
        std::string baseEntityID;

        Amara::World* world = nullptr;
        Amara::Entity* parent = nullptr;
        Amara::Scene* scene = nullptr;

        std::vector<Amara::Entity*> children;
        std::vector<Amara::Entity*> children_copy_list;

        std::unordered_map<std::string, std::function<void(nlohmann::json)>> configurables;
        
        sol::table props;
        sol::object luaobject;

        sol::function luaPreload;
        sol::function luaCreate;
        sol::function luaUpdate;
        
        MessageBox messages;

        Vector3 pos = { 0, 0, 0 };
        
        float depth = 0;
        bool lockDepthToY = false;
        bool do_not_depth_sort = false;

        bool isDestroyed = false;
        bool isPaused = false;
        bool isVisible = true;

        bool is_camera = false;
        bool is_scene = false;

        Entity() {
            baseEntityID = "Entity";
        }

        virtual void create() {}
        virtual void init() {
            update_properties();
            get_lua_object();
            create();
            if (luaCreate.valid()) {
                try {
                    luaCreate(get_lua_object());
                }
                catch (const sol::error& e) {
                    log("Error: On ", *this, "\" while executing onCreate().");
                }
            }
            make_configurables();
        }

        virtual void make_configurables() {
            configurables["id"] = [this](nlohmann::json val) { this->id = val; };
            configurables["x"] = [this](nlohmann::json val) { this->pos.x = val; };
            configurables["y"] = [this](nlohmann::json val) { this->pos.y = val; };
            configurables["z"] = [this](nlohmann::json val) { this->pos.z = val; };
        }

        virtual nlohmann::json toJSON() {
            return nlohmann::json::object({
                { "id", id },
                { "x", pos.x },
                { "y", pos.y },
                { "z", pos.z }
            });
        }
        
        Amara::Entity* configure(std::string key, nlohmann::json value) {
            configurables[key](value);
            return this;
        }

        Amara::Entity* configure(nlohmann::json config) {
            update_properties();
            if (config.is_string()) {
                std::string path = config.get<std::string>();
                if (string_endsWith(path, ".json")) {
                    configure(Properties::files->readJSON(path));
                }
                else if (string_endsWith(path, ".lua") || string_endsWith(path, ".luac")) {
                    configure(lua_to_json(Properties::scripts->run(path)));
                }
                return this;
            }
            for (auto it = config.begin(); it != config.end(); ++it) {
                configure(it.key(), it.value());
            }
            
            return this;
        }
        
        sol::object super_configure(sol::object config) {
            configure(lua_to_json(config));
            return get_lua_object();
        }

        sol::function configure_override;
        sol::object luaConfigure(sol::object config) {
            update_properties();
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
                    log("Error: On ", *this, "\" while executing configure().");
                }
            }
            else configure(lua_to_json(config));
            return get_lua_object();
        }
        sol::object luaConfigure(std::string key, sol::object val) {
            configure(key, lua_to_json(val));
            return get_lua_object();
        }

        virtual void preload() {
            update_properties();
            if (luaPreload.valid()) {
                try {
                    luaPreload(*this);
                }
                catch (const sol::error& e) {
                    log("Error: On ", *this, "\" while executing onPreload().");
                }
            }
        }
        
        virtual void update(double deltaTime) {}
        virtual void update_properties() {}

        virtual void run(double deltaTime) {
            update_properties();
            messages.run();

            update(deltaTime);
            if (luaUpdate.valid()) {
                try {
                    luaUpdate(get_lua_object(), deltaTime);
                }
                catch (const sol::error& e) {
                    log("Error: On ", *this, "\" while executing onUpdate().");
                }
            }

            if (lockDepthToY) depth = pos.y;

            if (!isDestroyed) runChildren(deltaTime);
            clean_entity_list(children);
        }
        void runChildren(double deltaTime) {
            children_copy_list = children;

            Amara::Entity* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
				update_properties();

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

        virtual void draw() {
            if (isDestroyed) return;

            sortChildren();
            drawChildren();
        }
        virtual void drawChildren() {
            children_copy_list = children;

            Amara::Entity* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = *it;
				if (child == nullptr || child->isDestroyed || child->parent != this) {
					++it;
					continue;
				}
                update_properties();
				child->draw();
				++it;
			}
        }

        void sortChildren();

        virtual Amara::Entity* addChild(Amara::Entity* entity) {
            if (isDestroyed) return entity;
            
            update_properties();
            entity->world = world;
            entity->scene = scene;
            entity->parent = this;
            children.push_back(entity);
            entity->init();

            return entity;
        }
        Amara::Entity* createChild(std::string);
        sol::object luaCreateChild(std::string);

        void removeChild(Amara::Entity* find) {
            if (isDestroyed) return;
            Amara::Entity* child;
			for (auto it = children.begin(); it != children.end();) {
                child = *it;
				if (child == find) {
					it = children.erase(it);
					continue;
				}
				++it;
			}
        }

        void destroy() {
            if (isDestroyed) return;
            isDestroyed = true;

            if (parent) parent->removeChild(this);

            Amara::Entity* child;
            for (auto it = children.begin(); it != children.end();) {
                child = *it;
                child->destroy();
				++it;
			}

            // NOTE: ADD TO GARBAGE QUEUE
        }

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

        static void clean_entity_list(std::vector<Amara::Entity*>& list) {
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

        virtual ~Entity() {}

        static void bindLua(sol::state& lua) {
            sol::usertype<Entity> entity_type = lua.new_usertype<Entity>("Entity",
                "pos", &Entity::pos,
                "id", &Entity::id,
                "baseEntityID", sol::readonly(&Entity::baseEntityID),
                "entityID", sol::readonly(&Entity::entityID),
                "parent", sol::readonly(&Entity::parent),
                "props", &Entity::props,
                "x", sol::property([](Entity& e, float val) { e.pos.x = val; }, [](Entity& e) { return e.pos.x; }),
                "y", sol::property([](Entity& e, float val) { e.pos.y = val; }, [](Entity& e) { return e.pos.y; }),
                "z", sol::property([](Entity& e, float val) { e.pos.z = val; }, [](Entity& e) { return e.pos.z; }),
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
                "createChild", &Entity::luaCreateChild,
                "addChild", &Entity::addChild,
                "isDestroyed", sol::readonly(&Entity::isDestroyed),
                "destroy", &Entity::destroy,
                "do_not_depth_sort", &Entity::do_not_depth_sort,
                "string", [](Amara::Entity* e){
                    return std::string(*e);
                }
            );

            lua.new_usertype<std::vector<Amara::Entity*>>("EntityVector",
                "size", &std::vector<Amara::Entity*>::size,
                sol::meta_function::length, &std::vector<Amara::Entity*>::size,
                sol::meta_function::index, [](std::vector<Amara::Entity*>& vec, sol::object getter) -> sol::object {
                    if (getter.is<size_t>()) {
                        size_t index = getter.as<size_t>();
                        std::vector<Amara::Entity*> copylist = vec;
                        clean_entity_list(copylist);
                        if (index > 0 && index <= vec.size()) {
                            return copylist[index-1]->get_lua_object();
                        }
                    }
                    else if (getter.is<std::string>()) {
                        std::string gid = getter.as<std::string>();
                        for (Amara::Entity* entity: vec) {
                            if (entity->isDestroyed) continue;
                            if (string_equal(entity->id, gid)) {
                                return entity->get_lua_object();
                            }
                        }
                    }
                    return sol::nil;
                }, 
                "push", [](std::vector<Amara::Entity*>& vec, Amara::Entity* entity) {
                    vec.push_back(entity);
                },
                "get", [](std::vector<Amara::Entity*>& vec, size_t index) -> sol::object {
                    std::vector<Amara::Entity*> copylist = vec;
                    clean_entity_list(copylist);
                    if (index > 0 && index <= vec.size()) {
                        return copylist[index-1]->get_lua_object();
                    }
                    return sol::nil;
                },
                "find", [](std::vector<Amara::Entity*>& vec, std::string gid) -> sol::object {
                    for (Amara::Entity* entity: vec) {
                        if (entity->isDestroyed) continue;
                        if (string_equal(entity->id, gid)) {
                            return entity->get_lua_object();
                        }
                    }
                    return sol::nil;
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

    struct sort_entities_by_depth {
		inline bool operator() (Amara::Entity* entity1, Amara::Entity* entity2) {
			if (entity1 == nullptr) return true;
			if (entity2 == nullptr) return true;
            if (entity1->isDestroyed || entity1->do_not_depth_sort) return true;
			if (entity2->isDestroyed || entity2->do_not_depth_sort) return true;
            return (entity1->depth < entity2->depth);
		}
	};

    void Entity::sortChildren() {
        std::stable_sort(children.begin(), children.end(), sort_entities_by_depth());
    }
}