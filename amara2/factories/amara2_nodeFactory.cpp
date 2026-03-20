namespace Amara {
    #ifdef AMARA_PLUGINS
    NodeFactory* Plugins::nodeFactory = nullptr;
    #endif
    
    class NodeDescriptor {
    public:
        nlohmann::json data = nullptr;
        sol::object definition = sol::nil;

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
        sol::object luaobject;
        
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
                fatal_error("Error: \"", key, "\" is a reserved node name.");
                return false;
            }
            
            std::string script_path = gameProps->system->getScriptPath(path);

            if (!gameProps->system->exists(script_path)) {
                fatal_error("Error: Failed to load Node \"", key, "\" from \"", path, "\". File not found.");
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
        bool load(std::string path) {
            std::string script_path = gameProps->system->getScriptPath(path);

            if (!gameProps->system->exists(script_path)) {
                fatal_error("Error: Failed to load Node from \"", path, "\". File not found.");
                return false;
            }
            
            if (String::endsWith(script_path, ".lua") || String::endsWith(script_path, ".luac")) {
                gameProps->system->run(script_path);
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
            if (!String::equal(node->nodeID, key) && !String::equal(node->baseNodeID, key)) {
                node->inheritanceChain.push_back(node->nodeID);
            }
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
                
                if (node) {
                    node->nodeID = key;
                    if (!desc.data.is_null()) node->configure(desc.data);
                    if (desc.definition.valid()) {
                        node->luaConfigure(lua_deep_copy(gameProps->lua, desc.definition));
                    }

                    return prepNode(node, key);
                }
            }
            
            std::string script_path = gameProps->system->getScriptPath(key);
            if (gameProps->system->exists(script_path)) {
                if (String::endsWith(script_path, ".lua") || String::endsWith(script_path, ".luac")) {
                    sol::object result = gameProps->system->run(script_path);
                    
                    if (result.is<Amara::Node*>()) {
                        Amara::Node* node = result.as<Amara::Node*>();
                        return prepNode(node, node->baseNodeID);
                    }
                }
                else if (String::endsWith(script_path, ".amara") || String::endsWith(script_path, ".json")) {
                    NodeDescriptor desc;
                    nlohmann::json data = gameProps->system->readJSON(script_path);
                    desc.data = data;
                    
                    desc.nodeID = data["nodeID"];
                    desc.baseNodeID = data["baseNodeID"];

                    Amara::Node* node = create(desc.baseNodeID);
                    node->gameProps = gameProps;
                    if (node) {
                        node->nodeID = key;
                        node->configure(desc.data);
                        return prepNode(node, key);
                    }
                }
            }
            
            fatal_error("Error: NodeID \"", key, "\" was not found.");
            gameProps->breakWorld();
            return nullptr;
        }
        
        sol::object luaCreate(std::string key, sol::object config) {
            Amara::Node* node = create(key);
            if (!node) return sol::nil;

            if (config.valid()) {
                node->luaConfigure(config);
            }

            return node->get_lua_object();
        }

        void defineNode(std::string nodeName, std::string baseName, sol::object config) {
            NodeDescriptor desc;
            desc.nodeID = nodeName;
            desc.baseNodeID = baseName;
            desc.definition = config;

            descriptors[nodeName] = desc;
        }

        sol::object castLuaNode(Amara::Node* node, std::string key) {
            auto it = nodeRegistry.find(key);
            if (it != nodeRegistry.end()) {
                return it->second(node);
            }
            else {
                fatal_error("Error: Node type with key \"", node->baseNodeID, "\" was not registered.");
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
            registerNode<Amara::DelayNode>("DelayNode");
            registerNode<Amara::RepeatAction>("RepeatAction");
            registerNode<Amara::ProcessNode>("ProcessNode");

            registerNode<Amara::TextInput>("TextInput");
            
            registerNode<Amara::Loader>("Loader");
            
            registerNode<Amara::AutoProgress>("AutoProgress");

            registerNode<Amara::Sprite>("Sprite");
            registerNode<Amara::RepeatSprite>("RepeatSprite");
            registerNode<Amara::ParticleEmitter>("ParticleEmitter");
            registerNode<Amara::Animation>("Animation");

            registerNode<Amara::FillRect>("FillRect");
            registerNode<Amara::FillCircle>("FillCircle");

            registerNode<Amara::Container>("Container");

            registerNode<Amara::TextureContainer>("TextureContainer");
            
            registerNode<Amara::QuadSprite>("QuadSprite");
            registerNode<Amara::QuadContainer>("QuadContainer");
            
            #ifdef AMARA_OPENGL
            registerNode<Amara::ShaderContainer>("ShaderContainer");
            registerNode<Amara::ShaderLayer>("ShaderLayer");
            registerNode<Amara::ShaderCamera>("ShaderCamera");
            #endif
            
            registerNode<Amara::TilemapLayer>("TilemapLayer");
            registerNode<Amara::Tilemap>("Tilemap");

            registerNode<Amara::NineSlice>("NineSlice");
            
            registerNode<Amara::PathFinder>("PathFinder");

            registerNode<Amara::Collider>("Collider");

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
            Amara::DelayNode::bind_lua(lua);
            Amara::RepeatAction::bind_lua(lua);
            Amara::ProcessNode::bind_lua(lua);

            Amara::TextInput::bind_lua(lua);
            
            Amara::Loader::bind_lua(lua);

            Amara::AutoProgress::bind_lua(lua);

            Amara::Sprite::bind_lua(lua);
            Amara::RepeatSprite::bind_lua(lua);
            Amara::ParticleEmitter::bind_lua(lua);
            Amara::Animation::bind_lua(lua);
            
            Amara::FillRect::bind_lua(lua);
            Amara::FillCircle::bind_lua(lua);

            Amara::Container::bind_lua(lua);
            
            Amara::TextureContainer::bind_lua(lua);
            
            Amara::QuadSprite::bind_lua(lua);
            Amara::QuadContainer::bind_lua(lua);
                        
            #ifdef AMARA_OPENGL
            Amara::ShaderContainer::bind_lua(lua);
            Amara::ShaderLayer::bind_lua(lua);
            Amara::ShaderCamera::bind_lua(lua);
            #endif
            
            Amara::TilemapLayer::bind_lua(lua);
            Amara::Tilemap::bind_lua(lua);

            Amara::NineSlice::bind_lua(lua);

            Amara::PathFinder::bind_lua(lua);
            Amara::Collider::bind_lua(lua);
            
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
                "load", sol::overload(
                    sol::resolve<bool(std::string, std::string)>(&NodeFactory::load),
                    sol::resolve<bool(std::string)>(&NodeFactory::load)
                ),
                "add", &NodeFactory::add,
                "create", &NodeFactory::luaCreate,
                "define", &NodeFactory::defineNode,
                "exists", &NodeFactory::exists
            );
        }

        void clear() {
            readScripts.clear();
            compiledScripts.clear();
            descriptors.clear();
            factory.clear();
            nodeRegistry.clear();
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
            if (config.valid()) {
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
            if (value.is<Amara::Node>() && key.is<std::string>()) {
                Amara::Node& node = value.as<Amara::Node&>();
                if (node.id.empty()) {
                    node.id = key.as<std::string>();
                }
            }
            tbl.raw_set(key, value);
        };
        props[sol::metatable_key] = props_meta;

        proxy = gameProps->lua.create_table();
        
        sol::table proxy_meta = gameProps->lua.create_table();
        
        proxy_meta["__newindex"] = [this](sol::table tbl, sol::object key, sol::object value) {
            if (key.is<std::string>() && this->setter_map.find(key.as<std::string>()) != this->setter_map.end()) {
                try {
                    sol::protected_function func = this->setter_map[key.as<std::string>()];
                    sol::protected_function_result result = func(this->get_lua_object(), value);
                    if (!result.valid()) {
                        sol::error err = result;
                        throw std::runtime_error(std::string(err.what()));
                    }
                }
                catch (const std::exception& e) {
                    fatal_error(e.what());
                }
            }
            else {
                this->props[key] = value;
            }
        };
        proxy_meta["__index"] = [this](sol::table tbl, sol::object key) -> sol::object {
            sol::object value = this->props[key];
            if (value.valid()) {
                if (value.is<Amara::Node>()) {
                    Amara::Node& node = value.as<Amara::Node&>();
                    if (node.destroyed) {
                        this->props[key] = sol::nil;
                    }
                    else {
                        return value;
                    }
                }
                else {
                    return value;
                }
            }
            if (key.is<std::string>()) {
                Amara::Node* node = this->getChild(key.as<std::string>());
                if (node && !node->destroyed) return node->get_lua_object();
            }
            return sol::nil;
        };
        proxy[sol::metatable_key] = proxy_meta;
        
        setter_table = gameProps->lua.create_table();
        sol::table setter_table_meta = gameProps->lua.create_table();
        setter_table_meta["__newindex"] = [this](sol::table tbl, sol::object key, sol::object value) {
            if (!key.is<std::string>() || !value.is<sol::function>()) {
                fatal_error("Error: Setter expected a function assignment.");
                return;
            }
            std::string str_key = key.as<std::string>();
            this->setter_map[str_key] = value.as<sol::function>();
        };
        setter_table[sol::metatable_key] = setter_table_meta;

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