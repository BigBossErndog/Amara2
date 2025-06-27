namespace Amara {
    #ifdef AMARA_PLUGINS
    NodeFactory* Plugins::nodeFactory = nullptr;
    #endif
    
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

            if (!gameProps->system->exists(script_path)) {
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
            if (gameProps->system->exists(script_path)) {
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
            if (!node) return sol::nil;

            if (!config.is<sol::nil_t>()) {
                node->luaConfigure(config);
            }

            return node->get_lua_object();
        }

        sol::object luaCreate(std::string nodeName, std::string baseName, sol::object config) {
            Amara::Node* node = create(baseName);
            if (!node) return sol::nil;

            if (!config.is<sol::nil_t>()) {
                node->luaConfigure(config);
            }
            
            return prepNode(node, nodeName)->get_lua_object();
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
            registerNode<Amara::NodePool>("NodePool");

            registerNode<Amara::Text>("Text");

            registerNode<Amara::Camera>("Camera");
            registerNode<Amara::Scene>("Scene");

            registerNode<Amara::Action>("Action");
            registerNode<Amara::WaitAction>("WaitAction");
            registerNode<Amara::Tween>("Tween");
            registerNode<Amara::StateMachine>("StateMachine");
            registerNode<Amara::DelayedNode>("DelayedNode");
            registerNode<Amara::PeriodicAction>("PeriodicAction");
            registerNode<Amara::ProcessNode>("ProcessNode");

            registerNode<Amara::TextInput>("TextInput");
            
            registerNode<Amara::Loader>("Loader");
            
            registerNode<Amara::AutoProgress>("AutoProgress");

            registerNode<Amara::Sprite>("Sprite");
            registerNode<Amara::Animation>("Animation");

            registerNode<Amara::FillRect>("FillRect");

            registerNode<Amara::Container>("Container");

            registerNode<Amara::TextureContainer>("TextureContainer");
            registerNode<Amara::ShaderContainer>("ShaderContainer");

            registerNode<Amara::TilemapLayer>("TilemapLayer");
            registerNode<Amara::Tilemap>("Tilemap");

            registerNode<Amara::NineSlice>("NineSlice");
            
            registerNode<Amara::PathFinder>("PathFinder");

            registerNode<Amara::Audio>("Audio");
            registerNode<Amara::AudioPool>("AudioPool");
            registerNode<Amara::AudioGroup>("AudioGroup");
            registerNode<Amara::AudioMaster>("AudioMaster");

            registerNode<Amara::Transition>("Transition");
            registerNode<Amara::FillTransition>("FillTransition");

            #ifdef AMARA_WEB_SERVER
            registerNode<Amara::WebServer>("WebServer");
            #endif
            
            registerNode<Amara::World>("World");

            #ifdef AMARA_PLUGINS
            Plugins::nodeFactory = this;
            Plugins::registerNodes();
            #endif
        }

        static void bind_lua(sol::state& lua) {
            Amara::Node::bind_lua(lua);

            Amara::Group::bind_lua(lua);
            Amara::CopyNode::bind_lua(lua);
            Amara::NodePool::bind_lua(lua);

            Amara::Text::bind_lua(lua);
            
            Amara::Camera::bind_lua(lua);
            Amara::Scene::bind_lua(lua);
            
            Amara::Action::bind_lua(lua);
            Amara::WaitAction::bind_lua(lua);
            Amara::Tween::bind_lua(lua);
            Amara::StateMachine::bind_lua(lua);
            Amara::DelayedNode::bind_lua(lua);
            Amara::PeriodicAction::bind_lua(lua);
            Amara::ProcessNode::bind_lua(lua);

            Amara::TextInput::bind_lua(lua);
            
            Amara::Loader::bind_lua(lua);

            Amara::AutoProgress::bind_lua(lua);

            Amara::Sprite::bind_lua(lua);
            Amara::Animation::bind_lua(lua);

            Amara::FillRect::bind_lua(lua);

            Amara::Container::bind_lua(lua);
            
            Amara::TextureContainer::bind_lua(lua);
            Amara::ShaderContainer::bind_lua(lua);
            
            Amara::TilemapLayer::bind_lua(lua);
            Amara::Tilemap::bind_lua(lua);

            Amara::NineSlice::bind_lua(lua);

            Amara::PathFinder::bind_lua(lua);
            
            Amara::Audio::bind_lua(lua);
            Amara::AudioPool::bind_lua(lua);
            Amara::AudioGroup::bind_lua(lua);
            Amara::AudioMaster::bind_lua(lua);

            Amara::Transition::bind_lua(lua);
            Amara::FillTransition::bind_lua(lua);
            
            #ifdef AMARA_WEB_SERVER
            Amara::WebServer::bind_lua(lua);
            #endif

            Amara::World::bind_lua(lua);
            
            #ifdef AMARA_PLUGINS
            Plugins::bind_lua(lua);
            #endif

            lua.new_usertype<NodeFactory>("NodeFactory",
                "load", &NodeFactory::load,
                "add", &NodeFactory::add,
                "create", sol::overload(
                    sol::resolve<sol::object(std::string, sol::object)>(&NodeFactory::luaCreate),
                    sol::resolve<sol::object(std::string, std::string, sol::object)>(&NodeFactory::luaCreate)
                )
            );
        }

        void clear() {
            readScripts.clear();
            compiledScripts.clear();
        }

        ~NodeFactory() {
            clear();
        }
    };

    Amara::Node* Node::createChild(std::string key) {
        Amara::Node* node = gameProps->factory->create(key);
        if (node) addChild(node);
        return node;
    }

    sol::object Node::luaCreateChild(std::string key, sol::object config) {
        Amara::Node* node = gameProps->factory->create(key);
        if (node) {
            if (!config.is<sol::nil_t>()) {
                node->luaConfigure(config);
            }
            addChild(node);
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
                sol::function func = sol::make_object(this->gameProps->lua, [this, callback](sol::variadic_args va)->sol::object {
                    std::vector<sol::object> remaining_args_vector;

                    if (va.size() > 0) {
                        for (auto it = va.begin() + 1; it != va.end(); ++it) {
                            remaining_args_vector.push_back(*it);
                        }
                    }

                    return callback(this->get_lua_object(), sol::as_args(remaining_args_vector));
                });
                tbl.raw_set(key, func);
            }
            else tbl.raw_set(key, value);
        };
        props[sol::metatable_key] = props_meta;

        return luaobject;
    }

    #ifdef AMARA_PLUGINS
    template <typename T>
    void Plugins::registerNode(std::string key) {
        Plugins::nodeFactory->registerNode<T>(key);
    }
    #endif

    bool Amara::Loader::loadPlugins(const LoadTask& task) {
        #ifdef AMARA_PLUGINS
        Plugins::load(task);
        #endif
    }
}