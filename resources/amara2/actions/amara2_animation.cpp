namespace Amara {
    class Animation: public Amara::Action {
    public:
        Amara::Sprite* sprite = nullptr;

        std::vector<int> frames;
        int index = 0;
        float frameRate = 60;

        int repeats = 0;

        bool yoyo = false;
        bool waitingYoyo = false;

        double progress = 0;

        std::string animKey;

        Animation(): Amara::Action() {
            set_base_node_id("Animation");
        }

        void setAnimation(std::string key) {
            animKey = key;
        }

        virtual void prepare() override {
            bool ready = false;

            if (sprite == nullptr) {
                sprite = actor->as<Amara::Sprite*>();
            }
            if (sprite) {
                if (sprite->spritesheet) {
                    AnimationData* animData = Props::animations->get(sprite->spritesheet->key, animKey);
                    
                    if (animData == nullptr) {
                        debug_log("Error: Node ", *sprite, " could not be animated.");
                        debug_log("Error: Animation with key \"", animKey, "\" was not found on texture \"", sprite->spritesheet->key, "\".");
                    }

                    frames = animData->frames;
                    repeats = animData->repeats;
                    frameRate = animData->frameRate;
                    yoyo = animData->yoyo;
                    
                    ready = true;
                }
                else {
                    debug_log("Error: Node ", *sprite, " does not have a valid texture for animation.");
                }
            }
            else {
                debug_log("Error: Node ", *actor, " cannot be animated.");
            }
            if (!ready) complete();
            else Amara::Action::prepare();
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
            sprite_type["animate"] = [](Amara::Sprite& s, std::string key) {
                Amara::Animation* anim = s.addChild(new Animation())->as<Amara::Animation*>();
                anim->setAnimation(key);
                return anim->get_lua_object();
            };
        }
    };
}