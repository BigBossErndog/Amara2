namespace Amara {
    class Scene;
    class World;
    class StateMachine;
    class Loader;
    class Transition;

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

        Amara::PassOnProps passOn;
        bool passOnPropsEnabled = true;
        
        Amara::MessageBox messages;

        Amara::NodeInput input;

        Amara::FunctionManager funcs;

        Amara::StateMachine* stateMachine = nullptr;
        Amara::Loader* loader = nullptr;

        Vector3 pos = { 0, 0, 0 };
        Vector3 worldPos = { 0, 0, 0 };

        Vector2 scale = { 1, 1 };
        float rotation = 0;

        Vector2 cameraFollowOffset = { 0, 0 };

        float alpha = 1;
        
        float depth = 0.0f;

        bool sortable = true;
        bool depthSortChildrenEnabled = true;

        bool fixedToCamera = false;

        bool destroyed = false;
        bool paused = false;
        bool visible = true;
        bool actuated = false;

        bool pauseOnce = false;

        double lifeTime = 0;

        double speed = 1;

        bool is_camera = false;
        bool is_action = false;
        bool is_autoprogress = false;

        bool is_animation = false;

        bool is_audio = false;
        bool is_audio_group = false;

        bool is_world = false;

        Amara::Node* collider = nullptr;
        
        std::deque<std::string> inheritanceChain;

        Transition* transition = nullptr;

        sol::table proxy;

        static std::unordered_map<std::string, bool> in_order_props;

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
            if (funcs.hasFunction("onCreate")) funcs.callFunction("onCreate");
        }

        virtual void init() {
            messages.init(gameProps, this);
            input.init(gameProps, this);
            funcs.init(gameProps, this);
            
            update_properties();
            get_lua_object();

            funcs.registerClass(nodeID);
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
                { "paused", paused },
                { "visible", visible },
                { "sortable", sortable },
                { "depthSortChildrenEnabled", depthSortChildrenEnabled },
                { "props", lua_to_json(props) }
            });

            #ifdef AMARA_OPENGL
            if (shaderProgram) {
                data["shaderProgram"] = shaderProgram->key;
            }
            #endif

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

        virtual Shape getShape() {
            return Vector2(0, 0);
        }
        virtual Shape getCollisionShape() {
            return Vector2(pos.x, pos.y);
        }

        bool collidesWith(Amara::Node* other) {
            return getCollisionShape().collidesWith(other->getCollisionShape());
        }
        bool overlaps(sol::object other) {
            if (other.is<Amara::Node*>()) {
                return collidesWith(other.as<Amara::Node*>());
            }
            if (other.is<Amara::Pointer>()) {
                Vector2 pos = other.as<Amara::Pointer>();
                return getCollisionShape().collidesWith(pos);
            }
            Shape shape = Shape(other);
            return getCollisionShape().collidesWith(shape);
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

            if (json_has(config, "pos")) pos = config["pos"];
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
            if (json_has(config, "cameraFollowOffset")) cameraFollowOffset = config["cameraFollowOffset"];
            if (json_has(config, "cameraFollowOffsetX")) cameraFollowOffset.x = config["cameraFollowOffsetX"];
            if (json_has(config, "cameraFollowOffsetY")) cameraFollowOffset.y = config["cameraFollowOffsetY"];

            if (json_has(config, "sortable")) sortable = config["sortable"];
            if (json_has(config, "depthSortChildrenEnabled")) depthSortChildrenEnabled = config["depthSortChildrenEnabled"];

            if (json_has(config, "speed")) {
                if (config["speed"].is_number()) {
                    speed = config["speed"];
                }
                else if (config["speed"].is_null()) {
                    speed = 1;
                }
            }

            if (json_is(config, "collider")) {
                if (collider == nullptr) {
                    collider = createChild("Collider");
                }
            }
            
            #ifdef AMARA_OPENGL
            if (json_has(config, "shaderProgram")) setShaderProgram(config["shaderProgram"]);
            #endif
            
            if (json_has(config, "input")) input.configure(config["input"]);
            
            return this;
        }

        Amara::Node* configure(std::string key, nlohmann::json value) {
            nlohmann::json config = nlohmann::json::object();   
            return configure(config);
        }

        virtual sol::object luaConfigure(sol::object config) {
            update_properties();

            if (config.is<sol::table>()) {
                sol::table tbl = config.as<sol::table>();
                std::vector<std::string> remove_keys;

                for (const auto& it: tbl) {
                    sol::object val = it.second;
                    if (val.is<sol::function>()) {
                        std::string key = it.first.as<std::string>();
                        sol::function func = val.as<sol::function>();
                        
                        funcs.setFunction(nodeID, key, func);
                        remove_keys.push_back(key);
                    }
                    else if (val.is<sol::userdata>()) {
                        std::string key = it.first.as<std::string>();
                        if (String::equal(key, "parent")) {
                            get_lua_object().as<sol::table>()["parent"] = val;
                        }
                        else {
                            luaConfigure(key, val);
                        }
                        remove_keys.push_back(key);
                    }
                    else if (val.is<sol::table>()) {
                        std::string key = it.first.as<std::string>();
                        if (String::equal(key, "props")) {
                            sol::table props_table = val.as<sol::table>();
                            for (const auto& prop_pair : props_table) {
                                props[prop_pair.first] = prop_pair.second;
                            }
                        }
                        else if (String::equal(key, "input")) {
                            input.configure(val);
                        }
                        else if (String::equal(key, "collider")) {
                            if (collider) {
                                collider->luaConfigure(val);
                            }
                            else {
                                collider = luaCreateChild("Collider", val).as<Amara::Node*>();
                            }
                        }
                        else if (in_order_props[key]) {
                            continue;
                        }
                        else {
                            luaConfigure(key, val);
                        }
                        remove_keys.push_back(key);
                    }
                }

                nlohmann::json json_config = lua_to_json(tbl);

                for (std::string& key: remove_keys) {
                    json_erase(json_config, key);
                }

                configure(json_config);
                if (funcs.hasFunction("onConfigure")) funcs.callFunction("onConfigure", config);
                
                return get_lua_object();
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
            if (funcs.hasFunction("onConfigure")) funcs.callFunction("onConfigure", config);
            
            return get_lua_object();
        }

        virtual sol::object luaConfigure(std::string key, sol::object val) {
            nlohmann::json config = nlohmann::json::object();
            config[key] = lua_to_json(val);
            configure(config);
            
            return get_lua_object();
        }

        sol::object configure_wrapper(sol::object config) {
            return luaConfigure(config);
        }

        sol::object configure_wrapper(std::string key, sol::object val) {
            return luaConfigure(key, val);
        }

        virtual void preload() {
            update_properties();
            if (funcs.hasFunction("onPreload")) funcs.callFunction("onPreload");
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
            pass_on_properties();
            worldPos = passOn.anchor;
            
            if (!actuated) {
                if (!destroyed && finishedLoading()) {
                    create();
                    actuated = true;
                }
            }
            if (destroyed) return;
            
            if (messages.active) messages.run();

            input.drag = Vector2(0, 0);
            if (input.active && !passOn.insideTextureContainer) {
                input.run(deltaTime);
                
                Amara::Pointer* lastPointer = input.lastInteraction.lastPointer;
                if (input.draggable && input.held && lastPointer != nullptr) {
                    Vector2 recPos = pos;
                    pos.x = input.rec_interact_pos.x + lastPointer->x - lastPointer->rec_pos.x;
                    pos.y = input.rec_interact_pos.y + lastPointer->y - lastPointer->rec_pos.y;
                    input.drag = pos - recPos;
                    if (input.isListening("onDrag")) {
                        input.handleMessage({ nullptr, "onDrag" });
                    }
                    input.dragging = true;
                }
            }

            if (destroyed) return;

            if (finishedLoading()) {
                update(deltaTime);

                if (!destroyed && funcs.hasFunction("onUpdate")) {
                    funcs.callFunction("onUpdate", deltaTime);
                }
            }
            
            if (!destroyed) runChildren(deltaTime);
            clean_node_list(children);

            lifeTime += deltaTime;
            if (is_world) gameProps->worldLifetime = lifeTime;
        }

        bool finishedLoading();

        virtual void runChildren(double deltaTime) {
            if (children.size() == 0) return;

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
                if (child->pauseOnce) {
                    child->pauseOnce = false;
                    ++it;
                    continue;
                }
                child->run(deltaTime * child->speed);

                gameProps->passOn = passOn;

				++it;
				if (destroyed) break;
			}
        }

        virtual void draw(const Rectangle& v) {
            if (destroyed) return;
            update_properties();

            if (funcs.hasFunction("preDraw")) {
                funcs.callFunction("preDraw", v);
            }

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
            if (children.size() == 0) return;

            children_copy_list = children;
            
            pass_on_properties();

            Amara::Node* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = *it;
				if (child == nullptr || child->destroyed || !child->visible || child->parent != this) {
					++it;
					continue;
				}
                
                if (passOn.insideCamera && child->is_camera) {
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
                fatal_error("Error: Shader program \"", key, "\" not found.");
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
            if (scene) node->scene = scene;
            node->parent = this;
            children.push_back(node);

            if (!node->actuated) {
                node->preload();
                if (!node->destroyed && node->finishedLoading()) {
                    node->create();
                    node->actuated = true;
                }
            }
            
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
				if (String::equal(child->id, gid) && !child->destroyed && child->parent == this) {
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

        void moveChildren(Amara::Node* other) {
            if (destroyed || other == this || other->destroyed) return;

            for (Amara::Node* child: children) {
                child->switchParent(other);
            }
        }

        virtual void destroy() {
            if (destroyed) return;
            update_properties();
            destroyed = true;

            if (funcs.hasFunction("onDestroy")) funcs.callFunction("onDestroy");
            
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
        
        sol::object pointTowards(float _x, float _y) {
            rotation = angleBetween(pos, Vector2(_x, _y));
            return get_lua_object();
        }
        sol::object pointTowards(sol::object v) {
            if (v.is<Node>()) {
                rotation = angleBetween(pos, v.as<Amara::Node*>()->pos);
            }
            else {
                Vector2 v2 = v;
                rotation = angleBetween(pos, v2);
            }
            
            return get_lua_object();
        }

        sol::object moveTowards(float _x, float _y, float speed) {
            float angle = angleBetween(pos, Vector2(_x, _y));
            return move(cos(angle) * speed, sin(angle) * speed);
        }
        sol::object moveTowards(sol::object v, float speed) {
            if (v.is<Node>()) {
                Vector2& other_pos = v.as<Amara::Node*>()->pos;
                return moveTowards(other_pos.x, other_pos.y, speed);
            }
            else {
                Vector2 v2 = v;
                return moveTowards(v2.x, v2.y, speed);
            }
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

        virtual void setSpeed(sol::object v) {
            if (v.is<int>()) {
                speed = v.as<int>();
            }
            else if (v.is<double>()) {
                speed = v.as<double>();
            }
            else if (!v.valid()) {
                speed = 1;
            }
        }

        template <typename T>
        T as();

        template <typename T>
        bool is() const {
            return dynamic_cast<const T*>(this) != nullptr;
        }

        sol::object get_lua_object();
        
        explicit operator std::string() const {
            std::string id_str = "";
            if (!id.empty()) id_str = String::concat(": \"", id, "\"");
            if (String::equal(baseNodeID, nodeID)) {
                return String::concat(
                    "(", baseNodeID, id_str, ")"
                );
            }
            return String::concat(
                "(",
                    baseNodeID, " -> ",
                    nodeID,
                    id_str,
                ")"
            );
        }
        friend std::ostream& operator<<(std::ostream& os, const Node& e) {
            return os << static_cast<std::string>(e);
        }

        std::string shortString() {
            return String::concat("(", nodeID, ")");
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

        void setFunction(std::string funcName, sol::function func) {
            funcs.setFunction(nodeID, funcName, func);
        }

        sol::object getClassFunctions(std::string key) {
            return funcs.getClassTable(key);
        }

        virtual ~Node() {}

        static void bind_lua(sol::state& lua) {
            sol::usertype<Node> node_type = lua.new_usertype<Node>("Node",
                "id", &Node::id,
                "baseNodeID", sol::readonly(&Node::baseNodeID),
                "nodeID", sol::readonly(&Node::nodeID),
                "parent", sol::property(
                    [](Node& e) { return e.parent->get_lua_object(); },
                    [](Node& e, sol::object val) {
                        if (val.is<Amara::Node>()) {
                            e.switchParent(val.as<Amara::Node*>());
                        }
                        else {
                            fatal_error("Error: Invalid value assigned as parent.");
                            e.gameProps->breakWorld();
                        }
                    }
                ),
                "props", &Node::props,
                "get", sol::readonly(&Amara::Node::proxy),
                "func", sol::property([](Node& e) {
                    return e.funcs.getClassTable(e.nodeID);
                }),
                "getClass", &Node::getClassFunctions,
                "classes", &Node::funcs,
                "pos", sol::property([](Node& e, sol::object val) { e.pos = val; }, [](Node& e) -> Vector2& { return e.pos; }),
                "worldPos", sol::readonly(&Node::worldPos),
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
                "scale", sol::property([](Node& e, sol::object val) { e.scale = val; }, [](Node& e) -> Vector2& { return e.scale; }),
                "scaleX", sol::property([](Node& e, float val) { e.scale.x = val; }, [](Node& e) { return e.scale.x; }),
                "scaleY", sol::property([](Node& e, float val) { e.scale.y = val; }, [](Node& e) { return e.scale.y; }),
                "rotation", &Node::rotation,
                "rotate", &Node::rotate,
                "pointTowards", sol::overload(
                    sol::resolve<sol::object(float, float)>(&Node::pointTowards),
                    sol::resolve<sol::object(sol::object)>(&Node::pointTowards)
                ),
                "moveTowards", sol::overload(
                    sol::resolve<sol::object(float, float, float)>(&Node::moveTowards),
                    sol::resolve<sol::object(sol::object, float)>(&Node::moveTowards)
                ),
                "cameraFollowOffset", &Node::cameraFollowOffset,
                "cameraFollowOffsetX", sol::property([](Node& e, float val) { e.cameraFollowOffset.x = val; }, [](Node& e) { return e.cameraFollowOffset.x; }),
                "cameraFollowOffsetY", sol::property([](Node& e, float val) { e.cameraFollowOffset.y = val; }, [](Node& e) { return e.cameraFollowOffset.y; }),
                "configure", sol::overload(
                    sol::resolve<sol::object(sol::object)>(&Node::configure_wrapper),
                    sol::resolve<sol::object(std::string, sol::object)>(&Node::configure_wrapper)
                ),
                "toData", &Node::toData,
                "alpha", &Node::alpha,
                "depth", &Node::depth,
                "fixedToCamera", &Node::fixedToCamera,
                "createChild", &Node::luaCreateChild,
                "addChild", &Node::addChild,
                "getChild", &Node::luaGetChild,
                "destroyed", sol::readonly(&Node::destroyed),
                "destroy", &Node::destroy,
                "destroyChildren", &Node::destroyChildren,
                "sortable", &Node::sortable,
                "depthSortChildrenEnabled", &Node::depthSortChildrenEnabled,
                "forceSortChildren", [](Amara::Node& n) {
                    n.sortChildren();
                },
                "bringToFront", &Node::bringToFront,
                "sendToBack", &Node::sendToBack,
                "switchParent", &Node::switchParent,
                "moveChildren", &Node::moveChildren,
                #ifdef AMARA_OPENGL
                "shaderProgram", sol::property([&](Node& e) { return e.shaderProgram; }, &Amara::Node::setShaderProgram),
                "setShaderProgram", &Node::setShaderProgram,
                #endif
                "stopActing", &Node::stopActing,
                "pause", &Node::pause,
                "resume", &Node::resume,
                "togglePause", &Node::togglePause,
                "active", sol::property([](Node& e) { return e.isActive(); }, [](Node& e, bool val) { if (val) e.activate(); else e.deactivate(); }),
                "activate", &Node::activate,
                "deactivate", &Node::deactivate,
                "actuated", sol::readonly(&Node::actuated),
                "paused", &Node::paused,
                "speed", sol::property([](Node& e, sol::object val) { e.setSpeed(val); }, [](Node& e) { return e.speed; }),
                "visible", &Node::visible,
                "lifeTime", sol::readonly(&Node::lifeTime),
                "string", [](Amara::Node* e) {
                    return std::string(*e);
                },
                "overlaps", &Node::overlaps,
                "collider", sol::property(
                    [](Node& e) -> sol::object { 
                        return (e.collider) ? e.collider->get_lua_object() : sol::nil;
                    },
                    [](Node& e, sol::object v) {
                        if (v.is<sol::table>()) {
                            if (e.collider) {
                                e.collider->luaConfigure(v);
                            }
                            else {
                                e.collider = e.luaCreateChild("Collider", v).as<Amara::Node*>();
                            }
                        }
                        else if (v.is<bool>() && v.as<bool>()) {
                            if (e.collider == nullptr) {
                                e.collider = e.createChild("Collider");
                            }
                        }
                    }
                ),

                "assets", sol::property([](Node& e) { return e.gameProps->assets; }),
                "shaders", sol::property([](Node& e) { return e.gameProps->shaders; }),
                "audio", sol::property([](Node& e) { return e.gameProps->audio; }),
                "animations", sol::property([](Node& e) { return e.gameProps->animations; }),
                "controls", sol::property([](Node& e) { return e.gameProps->controls; }),

                "messages", sol::readonly(&Node::messages),
                "input", sol::readonly(&Node::input)
            );

            lua.new_usertype<std::vector<Amara::Node*>>("NodeVector",
                "size", &std::vector<Amara::Node*>::size,
                sol::meta_function::length, &std::vector<Amara::Node*>::size,
                sol::meta_function::index, [](std::vector<Amara::Node*>& vec, sol::object getter) -> sol::object {
                    if (getter.is<size_t>()) {
                        size_t index = getter.as<size_t>();
                        if (index > 0 && index <= vec.size()) {
                            return vec[index-1]->get_lua_object();
                        }
                    }
                    else if (getter.is<std::string>()) {
                        std::string gid = getter.as<std::string>();
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

                        if (String::equal(findKey, ".")) {
                            if (forwardSlash && !nextKey.empty()) {
                                std::string new_gid = nextKey;
                                findKey.clear();
                                nextKey.clear();
                                forwardSlash = false;

                                for (char c: new_gid) {
                                    if (forwardSlash) {
                                        nextKey += c;
                                    }
                                    else if (c == '/') {
                                        forwardSlash = true;
                                    }
                                    else findKey += c;
                                }
                            }
                        }

                        Amara::Node* found = nullptr;
                        for (Amara::Node* node: vec) {
                            if (node->destroyed) continue;
                            if (String::equal(node->id, findKey)) {
                                found = node;
                            }
                        }
                        if (found != nullptr) {
                            if (forwardSlash && !nextKey.empty()) {
                                Amara::Node* child = found->getChild(nextKey);
                                if (child) return child->get_lua_object();
                            }
                            else return found->get_lua_object();
                        }
                    }
                    return sol::nil;
                }, 
                "push", [](std::vector<Amara::Node*>& vec, Amara::Node* node) {
                    vec.push_back(node);
                },
                "get", [](std::vector<Amara::Node*>& vec, size_t index) -> sol::object {
                    if (index > 0 && index <= vec.size()) {
                        return vec[index-1]->get_lua_object();
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
                "string", [](std::vector<Amara::Node*>& vec) -> std::string {
                    std::string output;
                    for (int i = 0; i < vec.size(); i++) {
                        output += std::string(*vec[i]);
                        if (i < vec.size()-1) {
                            output += "\n";
                        }
                    }
                    return output;
                },
                sol::meta_function::to_string, [](std::vector<Amara::Node*>& vec) -> std::string {
                    std::string output = "{";
                    for (int i = 0; i < vec.size(); i++) {
                        output += std::string(*vec[i]);
                        if (i < vec.size()-1) {
                            output += ", ";
                        }
                    }
                    output += '}';
                    return output;
                }
            );

            node_type["children"] = sol::readonly(&Node::children);
            node_type["child"] = sol::readonly(&Node::children);
        }
    };

    bool is_node(sol::object obj) {
        return obj.is<Amara::Node>();
    }
    std::string node_to_string(sol::object obj) {
        return std::string(obj.as<Amara::Node>());
    }
    std::string node_to_short_string(sol::object obj) {
        return std::string(obj.as<Amara::Node*>()->shortString());
    }
    
    struct sort_entities_by_depth {
		inline bool operator() (Amara::Node* node1, Amara::Node* node2) {
			if (node1 == nullptr || node1->destroyed) return false;
			if (node2 == nullptr || node2->destroyed) return true;

			if (node1->sortable != node2->sortable) {
				return node1->sortable;
			}

			if (!node1->sortable) return false;

            return (node1->depth < node2->depth);
		}
	};

    void Node::sortChildren() {
        std::stable_sort(children.begin(), children.end(), sort_entities_by_depth());
    }

    void MessageBox::handleMessage(const Message& msg) {
        if (messageBox.find(msg.key) != messageBox.end()) {
            const auto& list = messageBox[msg.key];
            for (const sol::protected_function& callback: list) {
                if (callback.valid()) {
                    try {
                        sol::protected_function_result result = callback(node->get_lua_object(), msg.data);
                        if (!result.valid()) {
                            sol::error err = result;
                            throw std::runtime_error(std::string(err.what()));  
                        }
                    }
                    catch (const std::exception& e) {
                        debug_log(e.what());
                        gameProps->breakWorld();
                    }
                }
            }
        }
    }

    sol::object Amara::FunctionMap::get_lua_object(Amara::Node* node) {
        if (node) return node->get_lua_object();
        return sol::nil;
    }

    sol::object Amara::FunctionManager::get_lua_object() {
        if (node) return node->get_lua_object();
        return sol::nil;
    }

    std::string Amara::FunctionManager::owner_node_string() {
        if (owner_node) return std::string(*owner_node);
        return "";
    }

    void Amara::AssetManager::createTexture(std::string key, sol::table luaconfig) {
        int w = 128, h = 12;
        
        nlohmann::json config = lua_to_json(luaconfig);
        if (json_has(config, "width")) w = config["width"];
        if (json_has(config, "w")) w = config["w"];

        if (json_has(config, "height")) h = config["height"]; 
        if (json_has(config, "h")) h = config["h"];

        ImageAsset* asset = new ImageAsset(gameProps);
        asset->width = w;
        asset->height = h;

        SDL_Texture* rec_target = nullptr;
        SDL_Rect prevSDLViewport;
        if (gameProps->renderer) {
            rec_target = SDL_GetRenderTarget(gameProps->renderer);
            SDL_GetRenderViewport(gameProps->renderer, &prevSDLViewport);

            SDL_Texture* canvas = SDL_CreateTexture(
                gameProps->renderer,
                SDL_PIXELFORMAT_RGBA32,
                SDL_TEXTUREACCESS_TARGET,
                w, h
            );

            SDL_SetRenderTarget(gameProps->renderer, canvas);
            SDL_SetTextureBlendMode(canvas, SDL_BLENDMODE_NONE);
            SDL_SetRenderDrawColor(gameProps->renderer, 255, 255, 255, 255);
            
            SDL_Rect setv = { 0, 0, w, h };
            SDL_SetRenderViewport(gameProps->renderer, &setv);

            SDL_RenderClear(gameProps->renderer);

            asset->texture = canvas;
        }
        #ifdef AMARA_OPENGL
        GLint prevBuffer = 0;
        GLint prevViewport[4];
        if (gameProps->glContext != NULL) {
            gameProps->renderBatch->flush();

            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevBuffer);
            glGetIntegerv(GL_VIEWPORT, prevViewport);

            GLuint glCanvasID = 0;
            GLuint glBufferID = 0;

            glMakeFrameBuffer(glCanvasID, glBufferID, w, h);
            glBindFramebuffer(GL_FRAMEBUFFER, glBufferID);

            #ifdef AMARA_OPENGL
            if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                gameProps->currentShaderProgram = nullptr;
            }
            #endif
            
            glViewport(0, 0, w, h);
            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            asset->glTextureID = glCanvasID;
        }
        #endif

        if (luaconfig["node"].valid()) {
            sol::object luaobj = luaconfig["node"];
            Amara::Node* node = luaobj.as<Amara::Node*>();
            
            if (node) {
                PassOnProps rec_props = gameProps->passOn;
                PassOnProps new_props;
                new_props.insideTextureContainer = true;

                gameProps->passOn = new_props;
                
                node->draw({0, 0, (float)w, (float)h});

                gameProps->passOn = rec_props;
            }
        }
        if (gameProps->renderer) {
            SDL_SetRenderTarget(gameProps->renderer, rec_target);
            SDL_SetRenderViewport(gameProps->renderer, &prevSDLViewport);
        }
        #ifdef AMARA_OPENGL
        if (gameProps->glContext != NULL) {
            gameProps->renderBatch->flush();
            glBindFramebuffer(GL_FRAMEBUFFER, prevBuffer);
            glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
        }
        #endif
        add(key, asset);
    }

    void Amara::AssetManager::updateTexture(std::string key, sol::table luaconfig) {
        if (!has(key)) fatal_error("Error: ImageAsset \"", key, "\" not found.");

        ImageAsset* asset = get(key)->as<ImageAsset*>();
        if (asset == nullptr) {
            fatal_error("Error: Asset with key \"", key, "\" is not an image.");
            return;
        }

        int w = asset->width, h = asset->height;
        
        SDL_Texture* rec_target = nullptr;
        SDL_Rect prevSDLViewport;
        if (gameProps->renderer) {
            rec_target = SDL_GetRenderTarget(gameProps->renderer);
            SDL_GetRenderViewport(gameProps->renderer, &prevSDLViewport);

            SDL_Texture* canvas = SDL_CreateTexture(
                gameProps->renderer,
                SDL_PIXELFORMAT_RGBA32,
                SDL_TEXTUREACCESS_TARGET,
                w, h
            );

            SDL_SetRenderTarget(gameProps->renderer, asset->texture);
            SDL_SetTextureBlendMode(canvas, SDL_BLENDMODE_NONE);
            SDL_SetRenderDrawColor(gameProps->renderer, 255, 255, 255, 255);
            
            SDL_Rect setv = { 0, 0, w, h };
            SDL_SetRenderViewport(gameProps->renderer, &setv);

            SDL_RenderClear(gameProps->renderer);
        }
        #ifdef AMARA_OPENGL
        GLint prevBuffer = 0;
        GLint prevViewport[4];
        if (gameProps->glContext != NULL) {
            gameProps->renderBatch->flush();

            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevBuffer);
            glGetIntegerv(GL_VIEWPORT, prevViewport);

            GLuint glCanvasID = 0;
            GLuint glBufferID = 0;

            glBindFramebuffer(GL_FRAMEBUFFER, glBufferID);

            #ifdef AMARA_OPENGL
            if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                gameProps->currentShaderProgram = nullptr;
            }
            #endif
            
            glViewport(0, 0, w, h);
            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            asset->glTextureID = glCanvasID;
        }
        #endif

        if (luaconfig["node"].valid()) {
            sol::object luaobj = luaconfig["node"];
            Amara::Node* node = luaobj.as<Amara::Node*>();
            
            if (node) {
                PassOnProps rec_props = gameProps->passOn;
                PassOnProps new_props;
                new_props.insideTextureContainer = true;

                gameProps->passOn = new_props;
                
                node->draw({0, 0, (float)w, (float)h});

                gameProps->passOn = rec_props;
            }
        }
        if (gameProps->renderer) {
            SDL_SetRenderTarget(gameProps->renderer, rec_target);
            SDL_SetRenderViewport(gameProps->renderer, &prevSDLViewport);
        }
        #ifdef AMARA_OPENGL
        if (gameProps->glContext != NULL) {
            gameProps->renderBatch->flush();
            glBindFramebuffer(GL_FRAMEBUFFER, prevBuffer);
            glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
        }
        #endif
    }

    std::unordered_map<std::string, bool> Amara::Node::in_order_props = {
        { "origin", true },
        { "scale", true },
        { "size", true },
        { "rect", true },
        { "view", true },
        { "pos", true },
        { "animation", true },
        { "tiles", true }
    };
}