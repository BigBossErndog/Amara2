namespace Amara {
    struct AnimationData {
        AnimationData() = default;

        std::string key;
        std::vector<int> frames;
        float frameRate = 60;
        int repeats = 0;
        bool yoyo = false;

        int numFrames = 0;
    };

    struct TextureAnimations {
        TextureAnimations() = default;
        std::unordered_map<std::string, AnimationData> animations;
    };

    class AnimationFactory {
    public:
        std::unordered_map<std::string, TextureAnimations> textureMap;

        Amara::GameProps* gameProps = nullptr;

        AnimationFactory() = default;

        void clear() {
            textureMap.clear();
        }

        bool add(nlohmann::json config) {
            if (json_has(config, "textures")) {
                nlohmann::json list = config["textures"];
                config.erase("textures");
                if (list.is_string()) {
                    config["texture"] = list;
                    add(config);
                    return true;
                }
                else if (list.is_array()) {
                    for (int i = 0; i < list.size(); i++) {
                        config["texture"] = list[i];
                        add(config);
                    }
                    return true;
                }
                return false;
            }
            
            if (!json_has(config, "key")) {
                debug_log("Error: Animation couldn't be created from ", config.dump());
                return false;
            }
            std::string animKey = config["key"];

            if (!json_has(config, "texture")) {
                debug_log("Error: Animation couldn't be created from ", config.dump());
                return false;
            }
            std::string textureKey = config["texture"];
            
            AnimationData anim;
            anim.key = animKey;

            anim.frames.clear();

            if (json_has(config, "frames")) {
                nlohmann::json frames = config["frames"];
                if (frames.is_array()) {
                    for (int i = 0; i < frames.size(); i++) {
                        anim.frames.push_back(frames[i]);
                    }
                    anim.numFrames = frames.size();
                }
                else {
                    debug_log("Error: Animation couldn't be created from ", config.dump());
                    return false;
                }
            }
            else if (json_has(config, "startFrame")) {
                int startFrame = config["startFrame"];
                
                if (json_has(config, "endFrame")) {
                    int endFrame = config["endFrame"];
                    for (int i = startFrame; i <= endFrame; i++) {
                        anim.frames.push_back(i);
                    }
                    anim.numFrames = 1 + endFrame - startFrame;
                }
                else if (json_has(config, "numFrames")) {
                    int numFrames = config["numFrames"];
                    for (int i = 0; i < numFrames; i++) {
                        anim.frames.push_back(startFrame + i);
                    }
                    anim.numFrames = numFrames;
                }
                else {
                    fatal_error("Error: Animation couldn't be created from ", config.dump(), "\nNote: endFrame or numFrame must be defined.");
                    gameProps->breakWorld();
                    return false;
                }
            }
            else {
                fatal_error("Error: Animation couldn't be created from ", config.dump(), "\nNote: Animation frames must be defined.");
                gameProps->breakWorld();
                return false;
            }

            if (json_has(config, "frameRate")) {
                anim.frameRate = config["frameRate"];
                if (anim.frameRate <= 0) {
                    fatal_error("Error: Animation couldn't be created from ", config.dump(), "\nNote: frameRate must be more than 0.");
                    gameProps->breakWorld();
                    return false;
                }
            }

            if (json_has(config, "repeats")) {
                anim.repeats = config["repeats"];
            }

            if (json_has(config, "yoyo")) {
                anim.yoyo = config["yoyo"];
            }
            
            if (textureMap.find(textureKey) == textureMap.end()) textureMap[textureKey] = TextureAnimations();
            TextureAnimations& texAnims = textureMap[textureKey];
            texAnims.animations[animKey] = anim;

            return true;
        }

        bool lua_add(sol::object config) {
            return add(lua_to_json(config));
        }

        AnimationData* get(std::string textureKey, std::string animKey) {
            if (textureMap.find(textureKey) == textureMap.end()) {
                return nullptr;
            }
            TextureAnimations& texAnims = textureMap[textureKey];
            if (texAnims.animations.find(animKey) == texAnims.animations.end()) {
                return nullptr;
            }
            return &(texAnims.animations[animKey]);
        }
        
        static void bind_lua(sol::state& lua) {
            lua.new_usertype<AnimationFactory>("AnimationFactory",
                "add", &AnimationFactory::lua_add
            );
        }
    };
}