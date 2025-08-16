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
        
        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "key")) {
                setAnimation(config["key"]);
            }
            else {
                setAnimation(config);
            }
            return Amara::Action::configure(config);
        }

        void setAnimation(nlohmann::json config) {
            progress = 0;
            repeats = 0;
            frameRate = 60;
            yoyo = false;
            waitingYoyo = false;
            animKey.clear();
            frames.clear();

            if (config.is_string()) {
                animKey = config.get<std::string>();

                if (sprite && !animKey.empty()) {
                    if (sprite->spritesheet) {
                        AnimationData* animData = gameProps->animations->get(sprite->spritesheet->key, animKey);
                                
                        if (animData == nullptr) {
                            fatal_error("Error: Node ", *sprite, " could not be animated.\n", "Animation with key \"", animKey, "\" was not found on texture \"", sprite->spritesheet->key, "\".");
                            return;
                        }

                        frames = animData->frames;
                        repeats = animData->repeats;
                        frameRate = animData->frameRate;
                        yoyo = animData->yoyo;
                        
                        anim_configured = true;
                    }
                    else {
                        fatal_error("Error: Node ", *sprite, " does not have a valid texture for animation.");
                        gameProps->breakWorld();
                        return;
                    }
                }
            }
            else if (config.is_object()) {
                AnimationData anim = gameProps->animations->createAnimation(config);

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
                fatal_error("Error: Node ", *actor, " cannot be animated.");
                gameProps->breakWorld();
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
            
            if (has_started) {
                if (sprite == nullptr || sprite->spritesheet == nullptr) {
                    fatal_error("Error: No sprite provided for animation.");
                    complete();
                }
                else {
                    progress += frameRate*deltaTime/frames.size();

                    if (progress >= 1) {
                        if (waitingYoyo) {
                            progress = fmod(progress, 1);
                            waitingYoyo = false;
                            std::reverse(frames.begin(), frames.end());
                        }
                        else if (repeats != 0) {
                            repeats -= 1;
                            progress = fmod(progress, 1);
                            if (yoyo) {
                                std::reverse(frames.begin(), frames.end());
                            }
                            waitingYoyo = yoyo;
                        }
                        else {
                            progress = 1;
                            complete();
                        }
                    }

                    if (sprite) {
                        index = progress >= 1 ? (frames.size()-1): floor(progress * (float)frames.size());
                        sprite->frame = frames[index];
                    }
                }
            }
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Animation>("Animation",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
                "setAnimation", &Animation::setAnimation,
                "progress", sol::readonly(&Animation::progress),
                "key", sol::readonly(&Animation::animKey)
            );

            sol::usertype<Amara::Sprite> sprite_type = lua["Sprite"];
            sprite_type["hasAnimation"] = [](Amara::Sprite& sprite, std::string key) -> bool {
                if (sprite.spritesheet && !key.empty()) {
                    AnimationData* animData = sprite.gameProps->animations->get(sprite.spritesheet->key, key);
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
            sprite_type["animation"] = sol::property(
                [](Amara::Sprite& s) -> sol::object {
                    for (Amara::Node* node: s.children) {
                        if (!node->is_animation || node->destroyed) continue;
                        Amara::Animation* anim = node->as<Amara::Animation*>();
                        if (!anim->completed) return anim->get_lua_object();
                    }
                    return sol::nil;
                },
                [](Amara::Sprite& s, sol::object v) {
                    s.animate(v);
                }
            );
        }
    };

    Amara::Action* Amara::Sprite::animate(nlohmann::json config) {
        for (Amara::Node* node: children) {
            if (node->is_animation && !node->destroyed) {
                if (config.is_string() && String::equal(config, node->as<Animation*>()->animKey)) {
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
        
        Amara::Animation* anim = createChild("Animation")->as<Amara::Animation*>();
        anim->setAnimation(config);
        return anim;
    }
}