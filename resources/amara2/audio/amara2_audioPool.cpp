namespace Amara {
    class AudioPool: public Amara::AudioGroup {
    public:
        std::vector<Amara::Audio*> pool;
        std::vector<Amara::Audio*> freeQueue;

        bool randomOrder = false;

        virtual Amara::Node* configure(nlohmann::json config) {
            if (json_has(config, "randomOrder")) randomOrder = config["randomOrder"];

            bool play_now = false;
            if (json_has(config, "playing")) {
                play_now = json_extract(config, "playing");
            }

            if (json_has(config, "audio")) {
                if (config["audio"].is_string()) {
                    std::string key = json_extract(config, "audio");
                    if (setAudio(key)) {
                        int poolSize = 1;
                        if (json_has(config, "poolSize")) poolSize = config["poolSize"];
                        for (int i = 0; i < poolSize; ++i) {
                            Amara::Audio* child = createChild("Audio")->as<Amara::Audio*>();
                            child->configure(config);
                            child->setAudio(key);
                        }
                    }
                }
            }
            
            if (play_now) play();
            
            return Amara::Audio::configure(config);
        }

        virtual void play() override {
            play_from_pool();
        }

        sol::object play_from_pool() {
            freeQueue.clear();
            for (Amara::Audio* audio: pool) {
                if (audio->destroyed || audio->parent != this) {
                    continue;
                }
                if (!audio->playing && !audio->paused) {
                    if (randomOrder) {
                        freeQueue.push_back(audio);
                    }
                    else {
                        audio->play();
                        playing = true;
                        return audio->get_lua_object();
                    }
                }
            }
            if (freeQueue.size() > 0) {
                // TODO: Randomize which audio is played from pool
                Amara::Audio* chosen = freeQueue[0];
                chosen->play();
                playing = true;
                return chosen->get_lua_object();
            }
            return sol::nil;
        }

        virtual void stop() override {
            Amara::Audio::stop();
            for (Amara::Audio* audio: pool) {
                audio->stop();
            }
        }

        virtual Amara::Node* addChild(Amara::Node* node) {
            if (!destroyed && node) {
                Amara::Audio* audio = node->as<Amara::Audio*>();
                if (audio) {
                    pool.push_back(audio);
                }
            }

            return Amara::Audio::addChild(node);
        }

        virtual void run(double deltaTime) override {
            Amara::Audio::run(deltaTime);

            playing = false;
            Amara::Audio* audio = nullptr;
            for (auto it = pool.begin(); it != pool.end();) {
                audio = *it;
                if (audio->destroyed || audio->parent != this) {
                    it = pool.erase(it);
                    continue;
                }
                if (audio->playing) playing = true;
                ++it;
            }
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<AudioPool>("AudioPool",
                sol::base_classes, sol::bases<Amara::Audio, Amara::Node>(),
                "play", &AudioPool::play_from_pool,
                "stop", &AudioPool::stop,
                "randomOrder", &AudioPool::randomOrder
            );
        }
    };
}