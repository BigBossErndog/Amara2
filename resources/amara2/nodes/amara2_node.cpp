namespace Amara {
    class Scene;
    class World;
    class StateMachine;
    class Loader;

    class Node {
    public:
        Amara::GameProps* gameProps = nullptr;

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
        MessageBox input;

        Amara::StateMachine* stateMachine = nullptr;
        Amara::Loader* loader = nullptr;

        Vector3 pos = { 0, 0, 0 };
        Vector2 scale = { 1, 1 };
        float rotation = 0;

        Vector2 cameraFollowOffset = { 0, 0 };

        float alpha = 1;
        
        float depth = 0.0f;
        bool yDepthLocked = false;
        bool zDepthLocked = false;

        bool depthSortSelfEnabled = true;
        bool depthSortChildrenEnabled = true;

        bool fixedToCamera = false;

        bool destroyed = false;
        bool paused = false;
        bool visible = true;
        bool actuated = false;

        double lifeTime = 0;

        bool is_camera = false;
        bool is_action = false;
        bool is_autoprogress = false;

        bool is_animation = false;

        bool is_audio = false;
        bool is_audio_group = false;

        bool is_world = false;

        std::deque<std::string> inheritanceChain;

        #ifdef AMARA_OPENGL
        ShaderProgram* shaderProgram = nullptr;
        #endif

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
                    gameProps->breakWorld();
                }
            }
        }

        virtual void init() {
            messages.gameProps = gameProps;
            update_properties();
            get_lua_object();
        }

        virtual nlohmann::json toJSON() {
            nlohmann::json data = nlohmann::json::object({
                { "id", id },
                { "nodeID", nodeID },
                { "baseNodeID", baseNodeID },
                { "x", pos.x },
                { "y", pos.y },
                { "z", pos.z },
                { "alpha", alpha },
                { "depth", depth },
                { "rotation", rotation },
                { "anchorX", passOn.anchor.x },
                { "anchorY", passOn.anchor.y },
                { "scaleX", scale.x },
                { "scaleY", scale.y },
                { "fixedToCamera", fixedToCamera },
                { "yDepthLocked", yDepthLocked },
                { "zDepthLocked", zDepthLocked },
                { "paused", paused },
                { "visible", visible },
                { "depthSortSelfEnabled", depthSortSelfEnabled },
                { "depthSortChildrenEnabled", depthSortChildrenEnabled },
                { "props", lua_to_json(props) }
            });

            if (shaderProgram) {
                data["shaderProgram"] = shaderProgram->key;
            }

            return data;
        }

        sol::object toData() {
            return json_to_lua(gameProps->lua, toJSON());
        }

        bool isActive() {
            return !destroyed && !paused && visible;
        }
        void activate() {
            paused = false;
            visible = true;
        }
        void deactivate() {
            paused = true;
            visible = false;
        }

        virtual Amara::Node* configure(nlohmann::json config) {
            update_properties();
            if (config.is_string()) {
                std::string path = config.get<std::string>();
                if (String::endsWith(path, ".json")) {
                    configure(gameProps->system->readJSON(path));
                }
                else if (String::endsWith(path, ".lua") || String::endsWith(path, ".luac")) {
                    configure(lua_to_json(gameProps->scripts->run(path)));
                }
                return this;
            }

            if (json_has(config, "id")) id = config["id"];
            if (json_has(config, "x")) pos.x = config["x"];
            if (json_has(config, "y")) pos.y = config["y"];
            if (json_has(config, "z")) pos.z = config["z"];

            if (json_has(config, "scaleX")) scale.x = config["scaleX"];
            if (json_has(config, "scaleY")) scale.y = config["scaleY"];
            if (json_has(config, "scale")) scale = config["scale"];
            
            if (json_has(config, "rotation")) rotation = config["rotation"];

            if (json_has(config, "alpha")) alpha = config["alpha"];
            if (json_has(config, "depth")) depth = config["depth"];
            
            if (json_has(config, "paused")) paused = config["paused"];

            if (json_has(config, "active")) {
                if (config["active"]) activate();
                else deactivate();
            }

            if (json_has(config, "visible")) visible = config["visible"];

            if (json_has(config, "fixedToCamera")) fixedToCamera = config["fixedToCamera"];

            if (json_has(config, "yDepthLocked")) yDepthLocked = config["yDepthLocked"];
            if (json_has(config, "zDepthLocked")) zDepthLocked = config["zDepthLocked"];

            if (json_has(config, "depthSortSelfEnabled")) depthSortSelfEnabled = config["depthSortSelfEnabled"];
            if (json_has(config, "depthSortChildrenEnabled")) depthSortChildrenEnabled = config["depthSortChildrenEnabled"];

            if (json_has(config, "shaderProgram")) setShaderProgram(config["shaderProgram"]);

            if (json_has(config, "onPreload") && config["onPreload"].is_string()) {
                luaPreload = string_to_lua_object(gameProps->lua, config["onPreload"].get<std::string>());
            }
            if (json_has(config, "onCreate") && config["onCreate"].is_string()) {
                luaCreate = string_to_lua_object(gameProps->lua, config["onCreate"].get<std::string>());
            }
            if (json_has(config, "onUpdate") && config["onUpdate"].is_string()) {
                luaUpdate = string_to_lua_object(gameProps->lua, config["onUpdate"].get<std::string>());
            }
            if (json_has(config, "onDestroy") && config["onDestroy"].is_string()) {
                luaDestroy = string_to_lua_object(gameProps->lua, config["onDestroy"].get<std::string>());
            }

            if (json_has(config, "props")) {
                nlohmann::json data = config["props"];
                if (data.is_object()) {
                    for (auto it = data.begin(); it != data.end(); ++it) {
                        props[it.key()] = json_to_lua(gameProps->lua,it.value());
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
                        if (String::equal("onPreload", key)) luaPreload = func;
                        else if (String::equal("onCreate", key)) luaCreate = func;
                        else if (String::equal("onUpdate", key)) luaUpdate = func;
                        else if (String::equal("onDestroy", key)) luaDestroy = func;
                        luaConfigure(key, val);
                    }
                    else if (val.is<sol::userdata>()) {
                        luaConfigure(it.first.as<std::string>(), val);
                    }
                }
            }

            if (config.is<std::string>()) {
                std::string path = config.as<std::string>();
                if (String::endsWith(path, ".json")) {
                    luaConfigure(json_to_lua(gameProps->lua, gameProps->system->readJSON(path)));
                    return get_lua_object();
                }
                std::string script_path = gameProps->system->getScriptPath(path);
                if (String::endsWith(script_path, ".lua") || String::endsWith(script_path, ".luac")) {
                    luaConfigure(gameProps->scripts->run(path));
                    return get_lua_object();
                }
                std::string str_config = config.as<std::string>();
                if (nlohmann::json::accept(str_config)) {
                    configure(nlohmann::json::parse(str_config));
                    return get_lua_object();
                }
                sol::object lua_config = string_to_lua_object(gameProps->lua, str_config);
                if (lua_config.is<sol::table>()) {
                    luaConfigure(lua_config);
                    return get_lua_object();
                }
                
                return get_lua_object();
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
                    gameProps->breakWorld();
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
                    gameProps->breakWorld();
                }
            }
        }
        
        virtual void update(double deltaTime) {}
        virtual void update_properties() {}
        virtual void pass_on_properties() {
            if (fixedToCamera && !gameProps->passOn.insideTextureContainer) {
                gameProps->passOn.reset();
            }
            passOn = gameProps->passOn;
            
            if (passOnPropsEnabled) {
                passOn.alpha *= alpha;
                
                passOn.rotation += rotation;

                passOn.anchor = Vector3(
                    rotateAroundAnchor(
                        gameProps->passOn.anchor, 
                        Vector2( 
                            (gameProps->passOn.anchor.x + pos.x*gameProps->passOn.scale.x), 
                            (gameProps->passOn.anchor.y + pos.y*gameProps->passOn.scale.y)
                        ), 
                        gameProps->passOn.rotation
                    ),
                    passOn.anchor.z + pos.z
                );

                passOn.scale = Vector2(
                    gameProps->passOn.scale.x * scale.x,
                    gameProps->passOn.scale.y * scale.y
                );

                gameProps->passOn = passOn;
            }
        }
        void reset_pass_on_props() {
            gameProps->passOn.reset();
        }
 
        virtual void run(double deltaTime) {
            update_properties();
            if (!actuated) {
                preload();
                if (!destroyed) create();
                actuated = true;
            }
            if (destroyed) return;
            
            if (messages.active) messages.run();
            if (input.active) input.run();

            if (destroyed) return;

            update(deltaTime);

            if (!destroyed && luaUpdate.valid()) {
                try {
                    sol::protected_function_result result = luaUpdate(get_lua_object(), deltaTime);
                    if (!result.valid()) {
                        sol::error err = result;
                        throw std::runtime_error("Lua Error: " + std::string(err.what()));  
                    }
                } catch (const std::exception& e) {
                    debug_log(e.what());
                    gameProps->breakWorld();
                }
            }
            if (yDepthLocked) depth = pos.y;
            else if (zDepthLocked) depth = pos.z;
            
            if (!destroyed) runChildren(deltaTime);
            clean_node_list(children);

            lifeTime += deltaTime;
            if (is_world) gameProps->worldLifetime = lifeTime;
        }

        bool finishedLoading();

        void runChildren(double deltaTime) {
            children_copy_list = children;

            Amara::Node* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
				if (destroyed) break;
                update_properties();

                child = *it;
				if (child == nullptr || child->destroyed || child->parent != this || child->paused) {
					++it;
					continue;
				}
                
				child->run(deltaTime);
				++it;
				if (destroyed) break;
			}
        }

        virtual void draw(const Rectangle& v) {
            if (destroyed) return;
            update_properties();
            drawObjects(v);
        }
        virtual void drawObjects(const Rectangle& v) {
            if (fixedToCamera && !gameProps->passOn.insideTextureContainer) {
                gameProps->passOn.reset();
            }
            passOn = gameProps->passOn;

            #ifdef AMARA_OPENGL
            ShaderProgram* rec_shader = gameProps->currentShaderProgram;
            if (gameProps->graphics == GraphicsEnum::OpenGL && shaderProgram && shaderProgram != gameProps->currentShaderProgram) {
                gameProps->currentShaderProgram = shaderProgram;
            }
            #endif
            
            drawSelf(v);

            if (depthSortChildrenEnabled) sortChildren();
            drawChildren(v);

            #ifdef AMARA_OPENGL
            if (rec_shader && shaderProgram && shaderProgram != rec_shader) {
                gameProps->currentShaderProgram = rec_shader;
            }
            #endif
        }
        virtual void drawSelf(const Rectangle& v) {}
        virtual void drawChildren(const Rectangle& v) {
            children_copy_list = children;

            pass_on_properties();

            Amara::Node* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = *it;
				if (child == nullptr || child->destroyed || !child->visible || child->parent != this) {
					++it;
					continue;
				}

                update_properties();
				child->draw(v);

                gameProps->passOn = passOn;
				++it;
			}
        }

        void sortChildren();

        #ifdef AMARA_OPENGL
        bool setShaderProgram(std::string key) {
            if (gameProps->graphics != GraphicsEnum::OpenGL || gameProps->glContext == NULL) return false;
            
            shaderProgram = gameProps->shaders->getShaderProgram(key);
            if (shaderProgram == nullptr) {
                debug_log("Error: Shader program \"", key, "\" not found.");
                return false;
            }
            return true;
        }
        Amara::ShaderProgram* createShaderProgram() {

        }
        #endif

        virtual Amara::Node* addChild(Amara::Node* node) {
            if (destroyed || node->parent == this) return node;
            
            update_properties();
            node->gameProps = gameProps;
            node->world = world;
            node->scene = scene;
            node->parent = this;
            children.push_back(node);

            return node;
        }
        Amara::Node* createChild(std::string);
        sol::object luaCreateChild(std::string, sol::object config);

        void removeChild(Amara::Node* find) {
            if (find->parent == this) find->parent = nullptr;
            if (destroyed) return;

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

        Amara::Node* findChild(std::string gid) {
            Amara::Node* child;
			for (auto it = children.rbegin(); it != children.rend();) {
                child = *it;
				if (String::equal(child->id, gid)) {
					return child;
				}
				++it;
			}
            return nullptr;
        }

        Amara::Node* getChild(std::string gid) {
            std::string findKey;
            std::string nextKey;
            bool forwardSlash = false;

            for (char c: gid) {
                if (forwardSlash) {
                    nextKey += c;
                }
                else if (c == '/') {
                    forwardSlash = true;
                }
                else findKey += c;
            }

            if (parent && String::equal(findKey, "..")) {
                if (forwardSlash && !nextKey.empty()) return parent->getChild(nextKey);
                else return parent;
            }
            if (String::equal(findKey, ".")) {
                if (forwardSlash && !nextKey.empty()) return getChild(nextKey);
                else return this;
            }

            Amara::Node* found = findChild(findKey);
            if (found != nullptr) {
                if (forwardSlash && !nextKey.empty()) return found->getChild(nextKey);
                else return found;
            }
            return nullptr;
        }
        sol::object luaGetChild(std::string gid) {
            Amara::Node* child = getChild(gid);
            if (child) return child->get_lua_object();
            return sol::nil;
        }

        bool isProp(std::string key) {
            sol::object p = props[key];
            if (p.is<bool>()) {
                return p.as<bool>();
            }
            return false;
        }

        void switchParent(Amara::Node* other) {
            if (destroyed || other->destroyed || other == parent) return;
            if (other->parent && other->parent == this) {
                if (parent) other->switchParent(parent);
                else removeChild(other);
            }
            if (parent) parent->removeChild(this);
            other->addChild(this);
        }

        virtual void destroy() {
            if (destroyed) return;
            update_properties();
            destroyed = true;

            if (luaDestroy.valid()) {
                try {
                    sol::protected_function_result result = luaDestroy(get_lua_object());
                    if (!result.valid()) {
                        sol::error err = result;
                        throw std::runtime_error("Lua Error: " + std::string(err.what()));  
                    }
                } catch (const std::exception& e) {
                    debug_log(e.what());
                    gameProps->breakWorld();
                }
            }

            messages.destroy();
            input.destroy();

            if (parent) parent->removeChild(this);

            destroyChildren();

            gameProps->queue_garbage(this);
        }
        void destroyChildren() {
            Amara::Node* child;
            for (auto it = children.begin(); it != children.end();) {
                child = *it;
                if (child->parent == this && !child->destroyed) {
                    child->destroy();
                }
				++it;
			}
            children.clear();
        }

        sol::object bringToFront() {
            if (parent == nullptr || destroyed) return get_lua_object();
            
            std::vector<Node*> family = parent->children;
            Node* node = nullptr;

            bool foundSelf = false;

            for (int i = 0; i < family.size(); i++) {
                node = family[i];
                if (node == this) foundSelf = true;
                else if (foundSelf) {
                    family[i - 1] = node;
                    family[i] = this;
                }
                if (!node->destroyed && node->depth > depth) {
                    depth = node->depth;
                }
            }
            if (foundSelf) parent->children = family;
            return get_lua_object();
        }

        sol::object sendToBack() {
            if (parent == nullptr || destroyed) return get_lua_object();
            
            std::vector<Node*> family = parent->children;
            Node* node = nullptr;

            bool foundSelf = false;

            for (int i = family.size()-1; i >= 0; i--) {
                node = family[i];
                if (node == this) foundSelf = true;
                else if (foundSelf) {
                    family[i + 1] = node;
                    family[i] = this;
                }
                if (!node->destroyed && node->depth < depth) {
                    depth = node->depth;
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

        sol::object rotate(float _r) {
            rotation += _r;
            return get_lua_object();
        }

        void stopActing() {
            for (Amara::Node* node: children) {
                if (node->is_action && !node->destroyed) node->destroy();
            }
        }

        sol::object pause() {
            paused = true;
            return get_lua_object();
        }
        sol::object resume() {
            paused = false;
            return get_lua_object();
        }
        sol::object togglePause() {
            paused = !paused;
            return get_lua_object();
        }

        template <typename T>
        T as();

        sol::object get_lua_object();
        
        explicit operator std::string() const {
            if (String::equal(baseNodeID, nodeID)) {
                return String::concat(
                    "(", baseNodeID, ": \"", id, "\")"
                );
            }
            return String::concat(
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
				if (node == nullptr || node->destroyed) {
					it = list.erase(it);
					continue;
				}
				++it;
			}
        }

        virtual ~Node() {}

        static void bind_lua(sol::state& lua) {
            sol::usertype<Node> node_type = lua.new_usertype<Node>("Node",
                "id", &Node::id,
                "baseNodeID", sol::readonly(&Node::baseNodeID),
                "nodeID", sol::readonly(&Node::nodeID),
                "parent", sol::readonly(&Node::parent),
                "props", &Node::props,
                "pos", sol::property([](Node& e, sol::object val) { e.pos = val; }, [](Node& e) { return e.pos; }),
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
                "scale", sol::property([](Node& e, sol::object val) { e.scale = val; }, [](Node& e) { return e.scale; }),
                "scaleX", sol::property([](Node& e, float val) { e.scale.x = val; }, [](Node& e) { return e.scale.x; }),
                "scaleY", sol::property([](Node& e, float val) { e.scale.y = val; }, [](Node& e) { return e.scale.y; }),
                "rotation", &Node::rotation,
                "rotate", &Node::rotate,
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
                "alpha", &Node::alpha,
                "depth", &Node::depth,
                "yDepthLocked", &Node::yDepthLocked,
                "zDepthLocked", &Node::zDepthLocked,
                "fixedToCamera", &Node::fixedToCamera,
                "onPreload", &Node::luaPreload,
                "onCreate", &Node::luaCreate,
                "onUpdate", &Node::luaUpdate,
                "onDestroy", &Node::luaDestroy,
                "createChild", &Node::luaCreateChild,
                "addChild", &Node::addChild,
                "get", &Node::luaGetChild,
                "destroyed", sol::readonly(&Node::destroyed),
                "destroy", &Node::destroy,
                "destroyChildren", &Node::destroyChildren,
                "depthSortSelfEnabled", &Node::depthSortSelfEnabled,
                "depthSortChildrenEnabled", &Node::depthSortChildrenEnabled,
                "bringToFront", &Node::bringToFront,
                "sendToBack", &Node::sendToBack,
                "switchParent", &Node::switchParent,
                #ifdef AMARA_OPENGL
                "shaderProgram", sol::readonly(&Node::shaderProgram),
                "setShaderProgram", &Node::setShaderProgram,
                #endif
                "stopActing", &Node::stopActing,
                "pause", &Node::pause,
                "resume", &Node::resume,
                "togglePause", &Node::togglePause,
                "active", sol::property([](Node& e) { return e.isActive(); }, [](Node& e, bool val) { if (val) e.activate(); else e.deactivate(); }),
                "activate", &Node::activate,
                "deactivate", &Node::deactivate,
                "paused", &Node::paused,
                "visible", &Node::visible,
                "string", [](Amara::Node* e) {
                    return std::string(*e);
                },

                "assets", sol::property([](Node& e) { return e.gameProps->assets; }),
                "shaders", sol::property([](Node& e) { return e.gameProps->shaders; }),
                "audio", sol::property([](Node& e) { return e.gameProps->audio; }),
                "animations", sol::property([](Node& e) { return e.gameProps->animations; }),
                "controls", sol::property([](Node& e) { return e.gameProps->controls; })
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
                            if (node->destroyed) continue;
                            if (String::equal(node->id, gid)) {
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
                        if (node->destroyed) continue;
                        if (String::equal(node->id, gid)) {
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
            if (node1->destroyed || !node1->depthSortSelfEnabled) return true;
			if (node2->destroyed || !node2->depthSortSelfEnabled) return true;
            return (node1->depth < node2->depth);
		}
	};

    void Node::sortChildren() {
        std::stable_sort(children.begin(), children.end(), sort_entities_by_depth());
    }
}