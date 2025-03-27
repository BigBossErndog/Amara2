namespace Amara {
    class Scene;
    class World;
    class StateMachine;
    class Loader;

    class Node {
    public:
        std::string id;
        std::string nodeID;
        std::string baseNodeID;

        Amara::World* world = nullptr;
        Amara::Node* parent = nullptr;
        Amara::Scene* scene = nullptr;

        std::vector<Amara::Node*> children;
        std::vector<Amara::Node*> children_copy_list;

        std::unordered_map<std::string, std::function<void(nlohmann::json)>> configurables;
        
        sol::table props;
        sol::object luaobject;

        PassOnProps passOn;
        bool passOnPropsEnabled = true;

        sol::protected_function luaPreload;
        sol::protected_function luaCreate;
        sol::protected_function luaUpdate;
        sol::protected_function luaDestroy;
        
        MessageBox messages;

        Amara::StateMachine* stateMachine = nullptr;
        Amara::Loader* loader = nullptr;

        Vector3 pos = { 0, 0, 0 };
        Vector2 scale = { 1, 1 };
        Vector2 zoomFactor = { 1, 1 };
        Vector2 scrollFactor = { 1, 1 };
        float rotation = 0;

        Vector2 cameraFollowOffset = { 0, 0 };
        
        float depth = 0;
        bool yDepthLocked = false;
        bool zDepthLocked = false;

        bool depthSortEnabled = true;

        bool isDestroyed = false;
        bool isPaused = false;
        bool isVisible = true;
        bool isActuated = false;

        bool is_camera = false;
        bool is_scene = false;
        bool is_action = false;
        bool is_load_task = false;

        bool is_animation = false;

        bool is_audio = false;
        bool is_audio_group = false;

        std::deque<std::string> inheritanceChain;

        Node() {
            set_base_node_id("Node");
        }

        void set_base_node_id(std::string key) {
            if (!baseNodeID.empty()) {
                inheritanceChain.push_front(baseNodeID);
            }
            baseNodeID = key;
        }

        virtual void create() {
            if (luaCreate.valid()) {
                try {
                    sol::protected_function_result result = luaCreate(get_lua_object());
                    if (!result.valid()) {
                        sol::error err = result;
                        throw std::runtime_error("Lua Error: " + std::string(err.what()));  
                    }
                } catch (const std::exception& e) {
                    debug_log(e.what());
                    Props::breakWorld();
                }
            }
        }

        virtual void init() {
            update_properties();
            get_lua_object();
        }

        virtual nlohmann::json toJSON() {
            return nlohmann::json::object({
                { "id", id },
                { "nodeID", nodeID },
                { "baseNodeID", baseNodeID },
                { "x", pos.x },
                { "y", pos.y },
                { "z", pos.z }
            });
        }

        sol::object toData() {
            return json_to_lua(toJSON());
        }

        virtual Amara::Node* configure(nlohmann::json config) {
            update_properties();
            if (config.is_string()) {
                std::string path = config.get<std::string>();
                if (string_endsWith(path, ".json")) {
                    configure(Props::files->readJSON(path));
                }
                else if (string_endsWith(path, ".lua") || string_endsWith(path, ".luac")) {
                    configure(lua_to_json(Props::scripts->run(path)));
                }
                return this;
            }

            if (json_has(config, "id")) id = config["id"];
            if (json_has(config, "x")) pos.x = config["x"];
            if (json_has(config, "y")) pos.y = config["y"];
            if (json_has(config, "z")) pos.z = config["z"];
            if (json_has(config, "props")) {
                nlohmann::json data = config["props"];
                if (data.is_object()) {
                    for (auto it = data.begin(); it != data.end(); ++it) {
                        props[it.key()] = json_to_lua(it.value());
                    }
                }
            }
            
            return this;
        }

        Amara::Node* configure(std::string key, nlohmann::json value) {
            nlohmann::json config = nlohmann::json::object();   
            return configure(config);
        }
        
        sol::object super_configure(sol::object config) {
            if (config.is<sol::table>()) {
                sol::table tbl = config.as<sol::table>();
                for (const auto& it: tbl) {
                    sol::object val = it.second;
                    if (val.is<sol::function>()) {
                        std::string key = it.first.as<std::string>();
                        sol::function func = val.as<sol::function>();
                        if (string_equal("onPreload", key)) luaPreload = func;
                        else if (string_equal("onCreate", key)) luaCreate = func;
                        else if (string_equal("onUpdate", key)) luaUpdate = func;
                        else if (string_equal("onDestroy", key)) luaDestroy = func;
                    }
                }
            }

            if (config.is<std::string>()) {
                std::string path = config.as<std::string>();
                if (string_endsWith(path, ".json")) {
                    luaConfigure(json_to_lua(Props::files->readJSON(path)));
                    return get_lua_object();
                }
                if (string_endsWith(path, ".lua") || string_endsWith(path, ".luac")) {
                    luaConfigure(Props::scripts->run(path));
                    return get_lua_object();
                }
            }

            configure(lua_to_json(config));
            return get_lua_object();
        }

        sol::protected_function configure_override;
        sol::object luaConfigure(sol::object config) {
            update_properties();

            if (configure_override.valid()) {
                try {
                    sol::protected_function_result result = configure_override(get_lua_object(), config);
                    if (!result.valid()) {
                        sol::error err = result;
                        throw std::runtime_error("Lua Error: " + std::string(err.what()));  
                    }
                } catch (const std::exception& e) {
                    debug_log(e.what());
                    Props::breakWorld();
                }
            }
            else {
                super_configure(config);
            }

            return get_lua_object();
        }
        virtual sol::object luaConfigure(std::string key, sol::object val) {
            nlohmann::json config = nlohmann::json::object();
            config[key] = lua_to_json(val);
            configure(config);
            return get_lua_object();
        }

        virtual void preload() {
            update_properties();
            if (luaPreload.valid()) {
                try {
                    sol::protected_function_result result = luaPreload(get_lua_object());
                    if (!result.valid()) {
                        sol::error err = result;
                        throw std::runtime_error("Lua Error: " + std::string(err.what()));  
                    }
                } catch (const std::exception& e) {
                    debug_log(e.what());
                    Props::breakWorld();
                }
            }
        }
        
        virtual void update(double deltaTime) {}
        virtual void update_properties() {}
        virtual void pass_on_properties() {
            passOn = Props::passOn;
            
            if (passOnPropsEnabled) {
                passOn.anchor = Vector3(
                    rotateAroundAnchor(
                        Props::passOn.anchor, 
                        Vector2( 
                            (Props::passOn.anchor.x + pos.x*Props::passOn.scale.x), 
                            (Props::passOn.anchor.y + pos.y*Props::passOn.scale.y)
                        ), 
                        Props::passOn.rotation
                    ),
                    passOn.anchor.z + pos.z
                );

                passOn.rotation += rotation;

                passOn.scale = {
                    Props::passOn.scale.x * scale.x,
                    Props::passOn.scale.y * scale.y
                };
                passOn.zoom = {
                    Props::passOn.zoom.x * zoomFactor.x,
                    Props::passOn.zoom.y * zoomFactor.y
                };
                passOn.scroll = {
                    Props::passOn.scroll.x * scrollFactor.x,
                    Props::passOn.scroll.y * scrollFactor.y
                };

                Props::passOn = passOn;
            }
        }
 
        virtual void run(double deltaTime) {
            if (!isActuated) {
                preload();
                create();
                isActuated = true;
            }

            update_properties();
            messages.run();

            update(deltaTime);
            if (!isDestroyed && luaUpdate.valid()) {
                try {
                    sol::protected_function_result result = luaUpdate(get_lua_object(), deltaTime);
                    if (!result.valid()) {
                        sol::error err = result;
                        throw std::runtime_error("Lua Error: " + std::string(err.what()));  
                    }
                } catch (const std::exception& e) {
                    debug_log(e.what());
                    Props::breakWorld();
                }
            }

            if (yDepthLocked) depth = pos.y;
            else if (zDepthLocked) depth = pos.z;
            
            if (!isDestroyed) runChildren(deltaTime);
            clean_node_list(children);
        }

        bool finishedLoading();

        void runChildren(double deltaTime) {
            children_copy_list = children;

            Amara::Node* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
				if (isDestroyed) break;
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

        virtual void draw(const Rectangle& v) {
            if (isDestroyed) return;
            update_properties();
            drawObjects(v);
        }
        virtual void drawObjects(const Rectangle& v) {
            passOn = Props::passOn;
            
            drawSelf(v);

            sortChildren();
            drawChildren(v);
        }
        virtual void drawSelf(const Rectangle& v) {}
        virtual void drawChildren(const Rectangle& v) {
            children_copy_list = children;

            pass_on_properties();

            Amara::Node* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = *it;
				if (child == nullptr || child->isDestroyed || child->parent != this) {
					++it;
					continue;
				}

                update_properties();
				child->draw(v);

                Props::passOn = passOn;
				++it;
			}
            
        }

        void sortChildren();

        virtual Amara::Node* addChild(Amara::Node* node) {
            if (isDestroyed) return node;
            
            update_properties();
            node->world = world;
            node->scene = scene;
            node->parent = this;
            children.push_back(node);

            return node;
        }
        Amara::Node* createChild(std::string);
        sol::object luaCreateChild(std::string);

        void removeChild(Amara::Node* find) {
            if (isDestroyed) return;
            Amara::Node* child;
			for (auto it = children.begin(); it != children.end();) {
                child = *it;
				if (child == find) {
					it = children.erase(it);
					continue;
				}
				++it;
			}
        }

        virtual void destroy() {
            if (isDestroyed) return;
            update_properties();
            isDestroyed = true;

            if (luaDestroy.valid()) {
                try {
                    sol::protected_function_result result = luaDestroy(get_lua_object());
                    if (!result.valid()) {
                        sol::error err = result;
                        throw std::runtime_error("Lua Error: " + std::string(err.what()));  
                    }
                } catch (const std::exception& e) {
                    debug_log(e.what());
                    Props::breakWorld();
                }
            }

            if (parent) parent->removeChild(this);

            Amara::Node* child;
            for (auto it = children.begin(); it != children.end();) {
                child = *it;
                child->destroy();
				++it;
			}

            Props::queue_garbage(this);
        }

        sol::object bringToFront() {
            if (parent == nullptr || isDestroyed) return get_lua_object();
            
            std::vector<Node*> family = parent->children;
            Node* node = nullptr;

            bool foundSelf = false;

            for (int i = 0; i < family.size(); i++) {
                node = family[i];
                if (node == this) foundSelf = true;
                else if (foundSelf) {
                    family[i - 1] = node;
                    family[i] = node;
                    if (!node->isDestroyed && node->depth > depth) {
                        depth = node->depth;
                    }
                }
            }
            if (foundSelf) parent->children = family;
            return get_lua_object();
        }

        sol::object sendToBack() {
            if (parent == nullptr || isDestroyed) return get_lua_object();
            
            std::vector<Node*> family = parent->children;
            Node* node = nullptr;

            bool foundSelf = false;

            for (int i = family.size()-1; i >= 0; i--) {
                node = family[i];
                if (node == this) foundSelf = true;
                else if (foundSelf) {
                    family[i + 1] = node;
                    family[i] = node;
                    if (!node->isDestroyed && node->depth < depth) {
                        depth = node->depth;
                    }
                }
            }
            if (foundSelf) parent->children = family;
            return get_lua_object();
        }

        sol::object move(const Vector3& v) {
            pos += v;
            return get_lua_object();
        }
        sol::object move(const Vector2& v) {
            return move(Vector3(v, 0));
        }
        sol::object move(float _x, float _y, float _z) {
            return move(Vector3(_x, _y, _z));
        }
        sol::object move(float _x, float _y) {
            return move(Vector2(_x, _y));
        }

        sol::object goTo(const Vector3& v) {
            pos = v;
            return get_lua_object();
        }
        sol::object goTo(const Vector2& v) {
            return goTo(Vector3(v, pos.z));
        }
        sol::object goTo(float _x, float _y, float _z) {
            return goTo(Vector3(_x, _y, _z));
        }
        sol::object goTo(float _x, float _y) {
            return goTo(Vector2(_x, _y));
        }

        sol::object setScale(float _x, float _y) {
            scale.x = _x;
            scale.y = _y;
            return get_lua_object();
        }
        sol::object setScale(float _g) {
            return setScale(_g, _g);
        }

        sol::object rotate(float _r) {
            rotation += _r;
            return get_lua_object();
        }

        template <typename T>
        T as();

        sol::object get_lua_object();
        
        explicit operator std::string() const {
            if (string_equal(baseNodeID, nodeID)) {
                return string_concat(
                    "(", baseNodeID, ": \"", id, "\")"
                );
            }
            return string_concat(
                "(", baseNodeID, ", ", 
                nodeID, ": \"",
                id, "\")"
            );
        }
        friend std::ostream& operator<<(std::ostream& os, const Node& e) {
            return os << static_cast<std::string>(e);
        }
        
        static void clean_node_list(std::vector<Amara::Node*>& list) {
            Amara::Node* node;
			for (auto it = list.begin(); it != list.end();) {
				node = *it;
				if (node == nullptr || node->isDestroyed) {
					it = list.erase(it);
					continue;
				}
				++it;
			}
        }

        virtual ~Node() {}

        static void bindLua(sol::state& lua) {
            sol::usertype<Node> node_type = lua.new_usertype<Node>("Node",
                "id", &Node::id,
                "baseNodeID", sol::readonly(&Node::baseNodeID),
                "nodeID", sol::readonly(&Node::nodeID),
                "parent", sol::readonly(&Node::parent),
                "props", &Node::props,
                "bind", &Node::props,
                "call", &Node::props,
                "pos", &Node::pos,
                "x", sol::property([](Node& e, float val) { e.pos.x = val; }, [](Node& e) { return e.pos.x; }),
                "y", sol::property([](Node& e, float val) { e.pos.y = val; }, [](Node& e) { return e.pos.y; }),
                "z", sol::property([](Node& e, float val) { e.pos.z = val; }, [](Node& e) { return e.pos.z; }),
                "move", sol::overload(
                    sol::resolve<sol::object(const Vector3&)>(&Node::move),
                    sol::resolve<sol::object(const Vector2&)>(&Node::move),
                    sol::resolve<sol::object(float, float, float)>(&Node::move),
                    sol::resolve<sol::object(float, float)>(&Node::move)
                ),
                "goTo", sol::overload(
                    sol::resolve<sol::object(const Vector3&)>(&Node::goTo),
                    sol::resolve<sol::object(const Vector2&)>(&Node::goTo),
                    sol::resolve<sol::object(float, float, float)>(&Node::goTo),
                    sol::resolve<sol::object(float, float)>(&Node::goTo)
                ),
                "scale", &Node::scale,
                "scaleX", sol::property([](Node& e, float val) { e.scale.x = val; }, [](Node& e) { return e.scale.x; }),
                "scaleY", sol::property([](Node& e, float val) { e.scale.y = val; }, [](Node& e) { return e.scale.y; }),
                "setScale", sol::overload(
                    sol::resolve<sol::object(float, float)>(&Node::setScale),
                    sol::resolve<sol::object(float)>(&Node::setScale)
                ),
                "rotation", &Node::rotation,
                "rotate", &Node::rotate,
                "scrollFactor", &Node::scrollFactor,
                "zoomFactor", &Node::zoomFactor,
                "cameraFollowOffset", &Node::cameraFollowOffset,
                "cameraFollowOffsetX", sol::property([](Node& e, float val) { e.cameraFollowOffset.x = val; }, [](Node& e) { return e.cameraFollowOffset.x; }),
                "cameraFollowOffsetY", sol::property([](Node& e, float val) { e.cameraFollowOffset.y = val; }, [](Node& e) { return e.cameraFollowOffset.y; }),
                "configure", sol::overload(
                    sol::resolve<sol::object(sol::object)>(&Node::luaConfigure),
                    sol::resolve<sol::object(std::string, sol::object)>(&Node::luaConfigure)
                ),
                "configure_override", &Node::configure_override,
                "super_configure", &Node::super_configure,
                "toData", &Node::toData,
                "depth", &Node::depth,
                "yDepthLocked", &Node::yDepthLocked,
                "zDepthLocked", &Node::zDepthLocked,
                "onPreload", &Node::luaPreload,
                "onCreate", &Node::luaCreate,
                "onUpdate", &Node::luaUpdate,
                "onDestroy", &Node::luaDestroy,
                "createChild", &Node::luaCreateChild,
                "addChild", &Node::addChild,
                "isDestroyed", sol::readonly(&Node::isDestroyed),
                "destroy", &Node::destroy,
                "depthSortEnabled", &Node::depthSortEnabled,
                "string", [](Amara::Node* e){
                    return std::string(*e);
                }
            );

            lua.new_usertype<std::vector<Amara::Node*>>("NodeVector",
                "size", &std::vector<Amara::Node*>::size,
                sol::meta_function::length, &std::vector<Amara::Node*>::size,
                sol::meta_function::index, [](std::vector<Amara::Node*>& vec, sol::object getter) -> sol::object {
                    if (getter.is<size_t>()) {
                        size_t index = getter.as<size_t>();
                        std::vector<Amara::Node*> copylist = vec;
                        clean_node_list(copylist);
                        if (index > 0 && index <= vec.size()) {
                            return copylist[index-1]->get_lua_object();
                        }
                    }
                    else if (getter.is<std::string>()) {
                        std::string gid = getter.as<std::string>();
                        for (Amara::Node* node: vec) {
                            if (node->isDestroyed) continue;
                            if (string_equal(node->id, gid)) {
                                return node->get_lua_object();
                            }
                        }
                    }
                    return sol::nil;
                }, 
                "push", [](std::vector<Amara::Node*>& vec, Amara::Node* node) {
                    vec.push_back(node);
                },
                "get", [](std::vector<Amara::Node*>& vec, size_t index) -> sol::object {
                    std::vector<Amara::Node*> copylist = vec;
                    clean_node_list(copylist);
                    if (index > 0 && index <= vec.size()) {
                        return copylist[index-1]->get_lua_object();
                    }
                    return sol::nil;
                },
                "find", [](std::vector<Amara::Node*>& vec, std::string gid) -> sol::object {
                    for (Amara::Node* node: vec) {
                        if (node->isDestroyed) continue;
                        if (string_equal(node->id, gid)) {
                            return node->get_lua_object();
                        }
                    }
                    return sol::nil;
                },
                "remove", [](std::vector<Amara::Node*>& vec, size_t index) {
                    if (index > 0 && index <= vec.size()) {
                        vec.erase(vec.begin() + index-1);
                    }
                },
                "clear", [](std::vector<Amara::Node*>& vec) {
                    // This is dangerous.
                    vec.clear();
                },
                "string", [](std::vector<Amara::Node*>& vec) -> std::string {
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

            node_type["children"] = sol::readonly(&Node::children);
        }
    };

    bool is_node(sol::object obj) {
        return obj.is<Amara::Node>();
    }
    std::string node_to_string(sol::object obj) {
        return std::string(obj.as<Amara::Node>());
    }

    struct sort_entities_by_depth {
		inline bool operator() (Amara::Node* node1, Amara::Node* node2) {
			if (node1 == nullptr) return true;
			if (node2 == nullptr) return true;
            if (node1->isDestroyed || !node1->depthSortEnabled) return true;
			if (node2->isDestroyed || !node2->depthSortEnabled) return true;
            return (node1->depth < node2->depth);
		}
	};

    void Node::sortChildren() {
        std::stable_sort(children.begin(), children.end(), sort_entities_by_depth());
    }
}