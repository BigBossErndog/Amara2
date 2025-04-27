namespace Amara {
    struct LoadTask {
        int type = (int)AssetEnum::None;
        std::string key;
        std::string path;

        bool replace = false;
        int failAttempts = 0;

        bool in_progress = false;

        int frameWidth = 0;
        int frameHeight = 0;

        sol::object config;

        int fontSize = 0;
    };
    
    class Loader: public Action {
    public:
        std::vector<LoadTask> tasks;

        int loadRate = 0;
        int maxFailAttempts = 1;
        
        bool replaceExisting = true;

        Loader(): Amara::Action() {
            set_base_node_id("Loader");
            is_load_task = true;
        }

        template <typename T>
        T* createAsset(std::string key) {
            if (gameProps->assets->has(key)) {
                T* asset = gameProps->assets->get(key)->as<T*>();
                if (asset) return asset;
            }
            return new T(gameProps);
        }

        bool processTask(const LoadTask& task) {
            if (!task.in_progress) {
                bool existing = gameProps->assets->has(task.key);
                if (existing) {
                    if (replaceExisting) {
                        debug_log("Note: Asset \"", task.key, "\" already exists. Overwriting Asset.");
                        debug_log("(Use \"node.load.replaceExisting = false\" to disable overwriting of assets.)");
                    }
                    else {
                        debug_log("Note: Asset \"", task.key, "\" already exists. Ignoring load task.");
                        debug_log("(Use \"node.load.replaceExisting = true\" to enable overwriting of assets.)");
                        return true;
                    }
                }
            }

            bool success = false;

            switch (task.type) {
                case (int)AssetEnum::Image: {
                    ImageAsset* imgAsset = createAsset<ImageAsset>(task.key);
                    success = imgAsset->loadImage(task.path);
                    if (success) gameProps->assets->add(task.key, imgAsset);
                    break;
                }
                case (int)AssetEnum::Spritesheet: {
                    SpritesheetAsset* sprAsset = createAsset<SpritesheetAsset>(task.key);
                    success = sprAsset->loadSpritesheet(task.path, task.frameWidth, task.frameHeight);
                    if (success) gameProps->assets->add(task.key, sprAsset);
                    break;
                }
                case (int)AssetEnum::Font: {
                    FontAsset* fontAsset = createAsset<FontAsset>(task.key);
                    success = fontAsset->loadFont(task.path, task.fontSize);
                    if (success) gameProps->assets->add(task.key, fontAsset);
                    break;
                }
                case (int)AssetEnum::TMXTilemap: {
                    TMXTilemapAsset* tilemapAsset = createAsset<TMXTilemapAsset>(task.key);
                    success = tilemapAsset->loadTmx(task.path);
                    if (success) gameProps->assets->add(task.key, tilemapAsset);
                    break;
                }
                case (int)AssetEnum::ShaderProgram: {
                    #ifdef AMARA_OPENGL
                    if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                        ShaderProgram* shaderProgram = gameProps->shaders->createShaderProgram(task.key, task.config);
                        if (shaderProgram) success = true;
                    }
                    else {
                        debug_log("Error: Could not load shader program. Graphics.OpenGL is not set.");
                    }
                    #else
                    debug_log("Error: Could not load shader program. OpenGL is not enabled this build.");
                    #endif
                    break;
                }
                default:
                    success = loadPlugins(task);
                    break;
            }

            if (!success && !task.in_progress) debug_log("Error: Failed to load asset \"", task.key, "\" from ", task.path);
            return success;
        }

        bool loadPlugins(const LoadTask& task);

        sol::object image(std::string key, std::string path) {
            LoadTask task;
            task.key = key;
            task.path = path;
            task.type = (int)AssetEnum::Image;
            
            queueTask(task);
            return get_lua_object();
        }

        sol::object spritesheet(std::string key, std::string path, int frameWidth, int frameHeight) {
            LoadTask task;
            task.key = key;
            task.path = path;
            task.type = (int)AssetEnum::Spritesheet;
            
            task.frameWidth = frameWidth;
            task.frameHeight = frameHeight;

            queueTask(task);
            return get_lua_object();
        }

        sol::object font(std::string key, std::string path, int fontSize) {
            LoadTask task;
            task.key = key;
            task.path = path;
            task.fontSize = fontSize;
            task.type = (int)AssetEnum::Font;

            queueTask(task);
            return get_lua_object();
        }

        sol::object tilemap(std::string key, std::string path) {
            LoadTask task;
            task.key = key;
            task.path = path;

            if (String::endsWith(path, ".tmx")) task.type = (int)AssetEnum::TMXTilemap;

            queueTask(task);
            return get_lua_object();
        }

        sol::object shaderProgram(std::string key, sol::table config) {
            LoadTask task;
            task.key = key;
            task.type = (int)AssetEnum::ShaderProgram;
            task.config = config;
            task.path = gameProps->lua["table"]["to_string"](config);

            queueTask(task);
            return get_lua_object();
        }

        sol::object custom(std::string key, int type, sol::table config) {
            LoadTask task;
            task.key = key;
            task.type = type;
            task.config = config;

            queueTask(task);
            return get_lua_object();
        }

        void queueTask(const LoadTask& task) {
            if (loadRate > 0) tasks.push_back(task);
            else processTask(task);
        }
        
        virtual void act(double deltaTime) override {
            Amara::Action::act(deltaTime);

            if (hasStarted) {
                int processedTasks = 0;

                for (auto it = tasks.begin(); it != tasks.end();) {
                    LoadTask& task = *it;
                    if (processTask(task)) {
                        it = tasks.erase(it);
                    }
                    else if (task.in_progress) {
                        break;
                    }
                    else {
                        LoadTask& task = *it;
                        task.failAttempts += 1;
                        if (task.failAttempts >= maxFailAttempts) {
                            debug_log("Error: Given up on load task \"", task.key, "\" ", task.path);
                            it = tasks.erase(it);
                        }
                        else {
                            ++it;
                        }
                    }

                    processedTasks += 1;
                    if (loadRate > 0 && processedTasks >= loadRate) {
                        break;
                    }
                }
            }
            if (tasks.size() == 0) complete();
        }

        virtual sol::object complete() override {
            if (parent && parent->loader == this) {
                parent->loader = nullptr;
            }
            if (actor && actor->loader == this) {
                actor->loader = nullptr;
            }

            return Amara::Action::complete();
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Loader>("Loader",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
                "replaceExisting", &Loader::replaceExisting,
                "image", &Loader::image,
                "spritesheet", &Loader::spritesheet,
                "font", &Loader::font,
                "tilemap", &Loader::tilemap,
                "shaderProgram", &Loader::shaderProgram,
                "custom", &Loader::custom,
                "loadRate", sol::property([](Amara::Loader& t) -> int { return t.loadRate; }, [](Amara::Loader& t, int v) { t.loadRate = v; }),
                "maxFailAttempts", sol::property([](Amara::Loader& t) -> int { return t.maxFailAttempts; }, [](Amara::Loader& t, int v) { t.maxFailAttempts = v; })
            );
            
            sol::usertype<Node> node_type = lua["Node"];
            node_type["load"] = sol::property([](Amara::Node& node) -> sol::object {
                if (node.loader == nullptr) {
                    node.loader = node.createChild("Loader")->as<Amara::Loader*>();
                }
                return node.loader->get_lua_object();
            });
            node_type["finishedLoading"] = sol::property(&Amara::Node::finishedLoading);
        }
    };

    bool Amara::Node::finishedLoading() {
        return loader == nullptr || loader->completed;
    }
}