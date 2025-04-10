namespace Amara {
    class NodeDescriptor {
    public:
        nlohmann::json data;
        std::string nodeID;
        std::string baseNodeID;
    };

    class NodeFactory {
    public:
        std::unordered_map<std::string, NodeDescriptor> descriptors;
        std::unordered_map<std::string, std::function<Node*()>> factory;
        std::unordered_map<std::string, std::string> readScripts;
        std::unordered_map<std::string, sol::function> compiledScripts;
        static inline std::unordered_map<std::string, std::function<sol::object(Node*)>> nodeRegistry;

        sol::table props;

        bool exists(std::string key) {
            if (factory.find(key) != factory.end()) return true;
            if (compiledScripts.find(key) != compiledScripts.end())  return true;
            if (readScripts.find(key) != readScripts.end()) return true;
            if (descriptors.find(key) != descriptors.end()) return true; 

            return false;
        }

        void add(std::string key, sol::function func) {
            compiledScripts[key] = func;
        }

        bool load(std::string key, std::string path) {
            if (factory.find(key) != factory.end()) {
                debug_log("Error: \"", key, "\" is a reserved node name.");
                return false;
            }
            
            std::string script_path = Props::files->getScriptPath(path);

            if (!Props::files->fileExists(script_path)) {
                debug_log("Error: Failed to load Node \"", key, "\" from \"", path, "\". File not found.");
                return false;
            }
            
            if (string_endsWith(script_path, ".lua")) {
                readScripts[key] = script_path;
            }
            else if (string_endsWith(script_path, ".luac")) {
                compiledScripts[key] = Props::files->load_script(script_path);
            }
            else if (string_endsWith(script_path, ".amara")) {
                NodeDescriptor desc;
                nlohmann::json data = Props::files->readJSON(script_path);
                desc.data = data;
                
                desc.nodeID = data["nodeID"];
                desc.baseNodeID = data["baseNodeID"];

                descriptors[desc.nodeID] = desc;
            }

            return true;
        }

        Amara::Node* prepNode(Amara::Node* node, std::string key) {
            node->nodeID = key;
            node->init();
            return node;
        }

        Amara::Node* create(std::string key) {
            auto it = factory.find(key);
            if (it != factory.end() && it->second) {
                return prepNode(it->second(), key);
            }
            
            if (compiledScripts.find(key) != compiledScripts.end()) {
                try {
                    sol::object result = compiledScripts[key]();
                    return prepNode(result.as<Amara::Node*>(), key);
                }
                catch (const sol::error& e) {
                    debug_log("Failed to create Node \"", key, "\".");
                    Props::breakWorld();
                    return nullptr;
                }
            }
            else if (readScripts.find(key) != readScripts.end()) {
                try {
                    sol::object result = Props::files->run(readScripts[key]);
                    return prepNode(result.as<Amara::Node*>(), key);
                }
                catch (const sol::error& e) {
                    debug_log("Failed to create Node \"", key, "\" from script \"", Props::files->getScriptPath(readScripts[key]), "\".");
                    Props::breakWorld();
                    return nullptr;
                }
            }
            else if (descriptors.find(key) != descriptors.end()) {
                NodeDescriptor& desc = descriptors[key];

                Amara::Node* node = create(desc.baseNodeID);
                if (node) node->configure(desc.data);

                return prepNode(node, key);
            }
            
            std::string script_path = Props::files->getScriptPath(key);
            if (Props::files->fileExists(script_path)) {
                if (string_endsWith(script_path, ".lua") || string_endsWith(script_path, ".luac")) {
                    sol::object result = Props::files->run(script_path);
                    Amara::Node* node = result.as<Amara::Node*>();
                    return prepNode(node, node->baseNodeID);
                }
                else if (string_endsWith(script_path, ".amara")) {
                    NodeDescriptor desc;
                    nlohmann::json data = Props::files->readJSON(script_path);
                    desc.data = data;
                    
                    desc.nodeID = data["nodeID"];
                    desc.baseNodeID = data["baseNodeID"];

                    Amara::Node* node = create(desc.baseNodeID);
                    if (node) node->configure(desc.data);

                    return prepNode(node, key);
                }
            }
            
            debug_log("Error: Node with key \"", key, "\" was not found.");
            Props::breakWorld();
            return nullptr;
        }
        
        sol::object luaCreate(std::string key) {
            Amara::Node* node = create(key);
            return node->get_lua_object();
        }

        sol::object castLuaNode(Amara::Node* node, std::string key) {
            auto it = nodeRegistry.find(key);
            if (it != nodeRegistry.end()) {
                return it->second(node);
            }
            else {
                debug_log("Error: Node type with key \"", node->baseNodeID, "\" was not registered.");
            }
            return sol::lua_nil;
        }

        template <typename T>
        void registerNode(std::string key) {
            factory[key] = []() -> T* { return new T(); };
            
            nodeRegistry[key] = [](Node* e) -> sol::object {
                if (T* derived = dynamic_cast<T*>(e)) {
                    return sol::make_object(Props::lua(), derived);
                }
                return sol::lua_nil;
            };
        }

        void prepareEntities() {
            registerNode<Amara::Node>("Node");

            registerNode<Amara::Text>("Text");

            registerNode<Amara::Camera>("Camera");
            registerNode<Amara::Scene>("Scene");

            registerNode<Amara::Action>("Action");
            registerNode<Amara::Tween>("Tween");
            registerNode<Amara::StateMachine>("StateMachine");
            
            registerNode<Amara::Loader>("Loader");

            registerNode<Amara::Sprite>("Sprite");
            registerNode<Amara::Animation>("Animation");

            registerNode<Amara::TextureContainer>("TextureContainer");

            registerNode<Amara::Audio>("Audio");
            registerNode<Amara::AudioGroup>("AudioGroup");
            
            registerNode<Amara::World>("World");
        }

        void clear() {
            readScripts.clear();
            compiledScripts.clear();
        }

        ~NodeFactory() {
            clear();
        }

        static void bindLua(sol::state& lua) {
            Amara::Node::bindLua(lua);
            Amara::Text::bindLua(lua);
            
            Amara::Camera::bindLua(lua);
            Amara::Scene::bindLua(lua);
            
            Amara::Action::bindLua(lua);
            Amara::Tween::bindLua(lua);
            Amara::StateMachine::bindLua(lua);

            Amara::Loader::bindLua(lua);

            Amara::Sprite::bindLua(lua);
            Amara::Animation::bindLua(lua);

            Amara::TextureContainer::bindLua(lua);
            
            Amara::Audio::bindLua(lua);
            Amara::AudioGroup::bindLua(lua);

            Amara::World::bindLua(lua);

            lua.new_usertype<NodeFactory>("NodeFactory",
                "load", &NodeFactory::load,
                "props", &NodeFactory::props,
                "add", &NodeFactory::add,
                "create", &NodeFactory::luaCreate
            );
        }
    };

    Amara::Node* Node::createChild(std::string key) {
        Amara::Node* node = Props::factory->create(key);
        if (node) addChild(node);
        return node;
    }

    sol::object Node::luaCreateChild(std::string key) {
        Amara::Node* node = createChild(key);
        if (node) return node->get_lua_object();
        return sol::nil;
    }

    template <typename T>
    T Node::as() {
        return dynamic_cast<T>(this);
    }

    sol::object Node::get_lua_object() {
        if (luaobject.valid()) return luaobject;

        luaobject = Props::factory->castLuaNode(this, baseNodeID);
        
        props = Props::lua().create_table();

        sol::table props_meta = Props::lua().create_table();
        
        props_meta["__newindex"] = [this](sol::table tbl, sol::object key, sol::object value) {
            if (value.is<sol::function>()) {
                sol::function callback = value.as<sol::function>();
                sol::function func = sol::make_object(Props::lua(), [this, callback](sol::variadic_args va)->sol::object {
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