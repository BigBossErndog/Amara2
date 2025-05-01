namespace Amara {
    NodeFactory* Plugins::nodeFactory = nullptr;

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

        Amara::GameProps* gameProps = nullptr;

        NodeFactory() = default;

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
            
            std::string script_path = gameProps->system->getScriptPath(path);

            if (!gameProps->system->fileExists(script_path)) {
                debug_log("Error: Failed to load Node \"", key, "\" from \"", path, "\". File not found.");
                return false;
            }
            
            if (String::endsWith(script_path, ".lua")) {
                readScripts[key] = script_path;
            }
            else if (String::endsWith(script_path, ".luac")) {
                compiledScripts[key] = gameProps->system->load_script(script_path);
            }
            else if (String::endsWith(script_path, ".amara")) {
                NodeDescriptor desc;
                nlohmann::json data = gameProps->system->readJSON(script_path);
                desc.data = data;
                
                desc.nodeID = data["nodeID"];
                desc.baseNodeID = data["baseNodeID"];

                descriptors[desc.nodeID] = desc;
            }

            return true;
        }

        Amara::Node* prepNode(Amara::Node* node, std::string key) {
            node->nodeID = key;
            node->gameProps = gameProps;
            node->init();
            return node;
        }

        Amara::Node* create(std::string key) {
            auto it = factory.find(key);
            if (it != factory.end() && it->second) {
                Amara::Node* node = it->second();
                return prepNode(node, key);
            }
            
            if (compiledScripts.find(key) != compiledScripts.end()) {
                try {
                    sol::object result = compiledScripts[key]();
                    return prepNode(result.as<Amara::Node*>(), key);
                }
                catch (const sol::error& e) {
                    debug_log("Failed to create Node \"", key, "\".");
                    gameProps->breakWorld();
                    return nullptr;
                }
            }
            else if (readScripts.find(key) != readScripts.end()) {
                try {
                    sol::object result = gameProps->system->run(readScripts[key]);
                    return prepNode(result.as<Amara::Node*>(), key);
                }
                catch (const sol::error& e) {
                    debug_log("Failed to create Node \"", key, "\" from script \"", gameProps->system->getScriptPath(readScripts[key]), "\".");
                    gameProps->breakWorld();
                    return nullptr;
                }
            }
            else if (descriptors.find(key) != descriptors.end()) {
                NodeDescriptor& desc = descriptors[key];

                Amara::Node* node = create(desc.baseNodeID);
                node->gameProps = gameProps;
                if (node) node->configure(desc.data);

                return prepNode(node, key);
            }
            
            std::string script_path = gameProps->system->getScriptPath(key);
            if (gameProps->system->fileExists(script_path)) {
                if (String::endsWith(script_path, ".lua") || String::endsWith(script_path, ".luac")) {
                    sol::object result = gameProps->system->run(script_path);
                    Amara::Node* node = result.as<Amara::Node*>();
                    return prepNode(node, node->baseNodeID);
                }
                else if (String::endsWith(script_path, ".amara")) {
                    NodeDescriptor desc;
                    nlohmann::json data = gameProps->system->readJSON(script_path);
                    desc.data = data;
                    
                    desc.nodeID = data["nodeID"];
                    desc.baseNodeID = data["baseNodeID"];

                    Amara::Node* node = create(desc.baseNodeID);
                    node->gameProps = gameProps;
                    if (node) node->configure(desc.data);

                    return prepNode(node, key);
                }
            }
            
            debug_log("Error: Node with key \"", key, "\" was not found.");
            gameProps->breakWorld();
            return nullptr;
        }
        
        sol::object luaCreate(std::string key, sol::object config) {
            Amara::Node* node = create(key);

            if (config.is<sol::table>()) node->luaConfigure(config);

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
            
            nodeRegistry[key] = [this](Node* e) -> sol::object {
                if (T* derived = dynamic_cast<T*>(e)) {
                    return sol::make_object(this->gameProps->lua, derived);
                }
                return sol::lua_nil;
            };
        }

        void prepareNodes() {
            registerNode<Amara::Node>("Node");
            registerNode<Amara::Group>("Group");
            registerNode<Amara::CopyNode>("CopyNode");

            registerNode<Amara::Text>("Text");

            registerNode<Amara::Camera>("Camera");
            registerNode<Amara::Scene>("Scene");

            registerNode<Amara::Action>("Action");
            registerNode<Amara::WaitAction>("WaitAction");
            registerNode<Amara::Tween>("Tween");
            registerNode<Amara::StateMachine>("StateMachine");
            
            registerNode<Amara::Loader>("Loader");
            
            registerNode<Amara::AutoProgress>("AutoProgress");

            registerNode<Amara::Sprite>("Sprite");
            registerNode<Amara::Animation>("Animation");

            registerNode<Amara::TextureContainer>("TextureContainer");

            registerNode<Amara::TilemapLayer>("TilemapLayer");
            registerNode<Amara::Tilemap>("Tilemap");
            
            registerNode<Amara::Audio>("Audio");
            registerNode<Amara::AudioPool>("AudioPool");
            registerNode<Amara::AudioGroup>("AudioGroup");
            registerNode<Amara::AudioMaster>("AudioMaster");

            #ifdef AMARA_WEB_SERVER
            registerNode<Amara::WebServer>("WebServer");
            #endif
            
            registerNode<Amara::World>("World");

            Plugins::nodeFactory = this;
            Plugins::registerNodes();
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
            Amara::Group::bindLua(lua);
            Amara::CopyNode::bindLua(lua);

            Amara::Text::bindLua(lua);
            
            Amara::Camera::bindLua(lua);
            Amara::Scene::bindLua(lua);
            
            Amara::Action::bindLua(lua);
            Amara::WaitAction::bindLua(lua);
            Amara::Tween::bindLua(lua);
            Amara::StateMachine::bindLua(lua);
            
            Amara::Loader::bindLua(lua);

            Amara::AutoProgress::bindLua(lua);

            Amara::Sprite::bindLua(lua);
            Amara::Animation::bindLua(lua);
            
            Amara::TextureContainer::bindLua(lua);

            Amara::TilemapLayer::bindLua(lua);
            Amara::Tilemap::bindLua(lua);
            
            Amara::Audio::bindLua(lua);
            Amara::AudioPool::bindLua(lua);
            Amara::AudioGroup::bindLua(lua);
            Amara::AudioMaster::bindLua(lua);

            #ifdef AMARA_WEB_SERVER
            Amara::WebServer::bindLua(lua);
            #endif

            Amara::World::bindLua(lua);
            
            Plugins::bindLua(lua);

            lua.new_usertype<NodeFactory>("NodeFactory",
                "load", &NodeFactory::load,
                "add", &NodeFactory::add,
                "create", &NodeFactory::luaCreate
            );
        }
    };

    Amara::Node* Node::createChild(std::string key) {
        Amara::Node* node = gameProps->factory->create(key);
        if (node) addChild(node);
        return node;
    }

    sol::object Node::luaCreateChild(std::string key, sol::object config) {
        Amara::Node* node = createChild(key);
        if (node) {
            if (config.is<sol::table>()) node->luaConfigure(config);
            return node->get_lua_object();
        }
        return sol::nil;
    }

    template <typename T>
    T Node::as() {
        return dynamic_cast<T>(this);
    }

    sol::object Node::get_lua_object() {
        if (luaobject.valid()) return luaobject;

        luaobject = gameProps->factory->castLuaNode(this, baseNodeID);
        
        props = gameProps->lua.create_table();

        sol::table props_meta = gameProps->lua.create_table();
        
        props_meta["__newindex"] = [this](sol::table tbl, sol::object key, sol::object value) {
            if (value.is<sol::function>()) {
                sol::function callback = value.as<sol::function>();
                sol::function func = sol::make_object(gameProps->lua, [this, callback](sol::variadic_args va)->sol::object {
                    return callback(this->get_lua_object(), sol::as_args(va));
                });
                tbl.raw_set(key, func);
            }
            else tbl.raw_set(key, value);
        };
        props[sol::metatable_key] = props_meta;

        return luaobject;
    }

    template <typename T>
    void Plugins::registerNode(std::string key) {
        Plugins::nodeFactory->registerNode<T>(key);
    }

    bool Amara::Loader::loadPlugins(const LoadTask& task) {
        Plugins::load(task);
    }
}