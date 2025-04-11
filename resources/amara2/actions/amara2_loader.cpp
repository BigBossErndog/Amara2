namespace Amara {
    struct LoadTask {
        AssetEnum type = AssetEnum::None;
        std::string key;
        std::string path;

        bool replace = false;
        int failAttempts = 0;

        int frameWidth = 0;
        int frameHeight = 0;

        int fontSize = 0;
    };
    
    class Loader: public Action {
    public:
        std::vector<LoadTask> tasks;

        int loadRate = 0;
        int maxFailAttempts = 2;

        bool replaceExisting = true;

        Loader() {
            set_base_node_id("Loader");
            is_load_task = true;
        }

        template <typename T>
        T* createAsset(std::string key) {
            if (Props::assets->has(key)) {
                T* asset = Props::assets->get(key)->as<T*>();
                if (asset) return asset;
            }
            return new T();
        }

        bool processTask(const LoadTask& task) {
            bool existing = Props::assets->has(task.key);
            if (existing) {
                if (replaceExisting) {
                    debug_log("Note: Asset \"", task.key, "\" already exists. Overwriting Asset.");
                    debug_log("(Use node.load:setReplaceExisting(false) to disable overwriting of assets.)");
                }
                else {
                    debug_log("Note: Asset \"", task.key, "\" already exists. Ignoring load task.");
                    debug_log("(Use node.load:setReplaceExisting(true) to enable overwriting of assets.)");
                    return true;
                }
            }

            bool success = false;

            switch (task.type) {
                case AssetEnum::Image: {
                    ImageAsset* imgAsset = createAsset<ImageAsset>(task.key);
                    success = imgAsset->loadImage(task.path);
                    if (success) Props::assets->add(task.key, imgAsset);
                    break;
                }
                case AssetEnum::Spritesheet: {
                    SpritesheetAsset* sprAsset = createAsset<SpritesheetAsset>(task.key);
                    success = sprAsset->loadSpritesheet(task.path, task.frameWidth, task.frameHeight);
                    if (success) Props::assets->add(task.key, sprAsset);
                    break;
                }
                case AssetEnum::Font: {
                    FontAsset* fontAsset = createAsset<FontAsset>(task.key);
                    success = fontAsset->loadFont(task.path, task.fontSize);
                    if (success) Props::assets->add(task.key, fontAsset);
                    break;
                }
                default:
                    break;
            }

            if (!success) debug_log("Error: Failed to load asset \"", task.key, "\" from ", task.path);
            return success;
        }

        sol::object image(std::string key, std::string path) {
            LoadTask task;
            task.key = key;
            task.path = path;
            task.type = AssetEnum::Image;
            
            queueTask(task);
            return get_lua_object();
        }

        sol::object spritesheet(std::string key, std::string path, int frameWidth, int frameHeight) {
            LoadTask task;
            task.key = key;
            task.path = path;
            task.type = AssetEnum::Spritesheet;

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
            task.type = AssetEnum::Font;

            queueTask(task);
            return get_lua_object();
        }

        sol::object setReplaceExisting(bool _r) {
            replaceExisting = _r;
            return get_lua_object();
        }

        sol::object setLoadRate(int _r) {
            loadRate = _r;
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
                    if (processTask(*it)) {
                        it = tasks.erase(it);
                    }
                    else {
                        LoadTask& task = *it;
                        task.failAttempts += 1;
                        if (task.failAttempts >= maxFailAttempts) {
                            debug_log("Error: Failed to load \"", task.key, "\" ", task.path);
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
                "setReplaceExisting", &Loader::setReplaceExisting,
                "image", &Loader::image,
                "spritesheet", &Loader::spritesheet,
                "font", &Loader::font,
                "setLoadRate", &Loader::setLoadRate
            );
            
            sol::usertype<Node> node_type = lua["Node"];
            node_type["load"] = sol::property([](Amara::Node& node) -> sol::object {
                if (node.loader == nullptr) {
                    node.loader = node.addChild(new Loader())->as<Amara::Loader*>();
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