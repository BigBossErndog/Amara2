namespace Amara {
    class Animation: public Amara::Action {
    public:
        Amara::Sprite* sprite = nullptr;

        std::vector<int> frames;
        int numFrames = 0;

        int index = 0;
        float frameRate = 60;

        int repeats = 0;

        bool yoyo = false;
        bool waitingYoyo = false;

        double progress = 0;

        std::string animKey;
        bool anim_configured = false;

        Animation(): Amara::Action() {
            set_base_node_id("Animation");
            is_animation = true;
        }

        void setAnimation(nlohmann::json config) {
            progress = 0;
            repeats = 0;
            frameRate = 60;
            yoyo = false;
            waitingYoyo = false;
            frames.clear();

            if (config.is_string()) {
                animKey = config.get<std::string>();

                if (sprite && !animKey.empty()) {
                    if (sprite->spritesheet) {
                        AnimationData* animData = Props::animations->get(sprite->spritesheet->key, animKey);
                                
                        if (animData == nullptr) {
                            debug_log("Error: Node ", *sprite, " could not be animated.");
                            debug_log("Error: Animation with key \"", animKey, "\" was not found on texture \"", sprite->spritesheet->key, "\".");
                            Props::breakWorld();
                            return;
                        }

                        frames = animData->frames;
                        repeats = animData->repeats;
                        frameRate = animData->frameRate;
                        yoyo = animData->yoyo;

                        anim_configured = true;
                    }
                    else {
                        debug_log("Error: Node ", *sprite, " does not have a valid texture for animation.");
                        Props::breakWorld();
                    }
                }
            }
            else if (config.is_object()) {
                AnimationData anim;
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
                        Props::breakWorld();
                        return;
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
                        debug_log("Error: Animation couldn't be created from ", config.dump());
                        debug_log("Note: endFrame or numFrame must be defined.");
                        Props::breakWorld();
                        return;
                    }
                }
                else {
                    debug_log("Error: Animation couldn't be created from ", config.dump());
                    debug_log("Note: Animation frames must be defined.");
                    Props::breakWorld();
                    return;
                }
    
                if (json_has(config, "frameRate")) {
                    anim.frameRate = config["frameRate"];
                    if (anim.frameRate <= 0) {
                        debug_log("Error: Animation couldn't be created from ", config.dump());
                        debug_log("Note: frameRate must be more than 0.");
                        Props::breakWorld();
                        return;
                    }
                }
                
                anim.repeats = 0;
                if (json_has(config, "repeats")) {
                    anim.repeats = config["repeats"];
                }
    
                anim.yoyo = json_is(config, "yoyo");

                frames = anim.frames;
                repeats = anim.repeats;
                frameRate = anim.frameRate;
                yoyo = anim.yoyo;
                numFrames = anim.numFrames;

                anim_configured = true;
            }
        }

        virtual void prepare() override {
            bool ready = false;

            if (sprite == nullptr) {
                sprite = actor->as<Amara::Sprite*>();
            }
            if (sprite) {
                if (!animKey.empty()) setAnimation(animKey);
            }
            else {
                debug_log("Error: Node ", *actor, " cannot be animated.");
                Props::breakWorld();
                return;
            }
            if (!anim_configured) complete();
            else {
                waitingYoyo = yoyo;
                Amara::Action::prepare();
            }
        }

        virtual void act(double deltaTime) {
            Amara::Action::act(deltaTime);
            
            if (hasStarted) {
                if (sprite == nullptr || sprite->spritesheet == nullptr) {
                    debug_log("Error: No sprite provided for animation.");
                    complete();
                }
                else {
                    progress += frameRate*deltaTime/frames.size();
                    if (progress > 1) progress = 1;

                    if (sprite) {
                        index = progress == 1 ? (frames.size()-1): floor(progress * (float)frames.size());
                        sprite->frame = frames[index];
                    }

                    if (progress == 1) {
                        if (waitingYoyo) {
                            progress = 0;
                            waitingYoyo = false;
                            std::reverse(frames.begin(), frames.end());
                        }
                        else if (repeats != 0) {
                            repeats -= 1;
                            progress = 0;
                            if (yoyo) {
                                std::reverse(frames.begin(), frames.end());
                            }
                            waitingYoyo = yoyo;
                        }
                        else complete();
                    }
                }
            }
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Animation>("Animation",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
                "setAnimation", &Animation::setAnimation
            );

            sol::usertype<Amara::Sprite> sprite_type = lua["Sprite"];
            sprite_type["hasAnimation"] = [](Amara::Sprite& sprite, std::string key) -> bool {
                if (sprite.spritesheet && !key.empty()) {
                    AnimationData* animData = Props::animations->get(sprite.spritesheet->key, key);
                    if (animData) return true;
                }
                return false;
            };
            sprite_type["isAnimating"] = sol::property([](Amara::Sprite& s) {
                for (Amara::Node* node: s.children) {
                    if (!node->is_animation || node->destroyed) continue;
                    Amara::Animation* anim = node->as<Amara::Animation*>();
                    if (!anim->completed) return true;
                }
                return false;
            });
            sprite_type["animation"] = sol::property([](Amara::Sprite& s) -> sol::object {
                for (Amara::Node* node: s.children) {
                    if (!node->is_animation || node->destroyed) continue;
                    Amara::Animation* anim = node->as<Amara::Animation*>();
                    if (!anim->completed) return anim->get_lua_object();
                }
                return sol::nil;
            });
        }
    };

    Amara::Action* Amara::Sprite::animate(nlohmann::json config) {
        for (Amara::Node* node: children) {
            if (node->is_animation && !node->destroyed) {
                if (config.is_string() && string_equal(config, node->as<Animation*>()->animKey)) {
                    return node->as<Amara::Action*>();
                }
                else {
                    Amara::Animation* anim = node->as<Animation*>();
                    if (anim) {
                        anim->setAnimation(config);
                        return anim;
                    }
                }
            }
        }
        
        Amara::Animation* anim = addChild(new Animation())->as<Amara::Animation*>();
        anim->setAnimation(config);
        return anim;
    }
}