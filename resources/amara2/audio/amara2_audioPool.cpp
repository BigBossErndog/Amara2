namespace Amara {
    class AudioPool: public Amara::AudioGroup {
    public:
        std::vector<Amara::Audio*> pool;
        std::vector<Amara::Audio*> freeQueue;

        bool randomOrder = false;

        virtual Amara::Node* configure(nlohmann::json config) {
            if (json_has(config, "randomOrder")) randomOrder = config["randomOrder"];
            
            return Amara::Audio::configure(config);
        }

        virtual void play() override {
            freeQueue.clear();
            for (Amara::Audio* audio: pool) {
                if (audio->destroyed || audio->parent != this)
                if (!audio->playing) {
                    freeQueue.push_back(audio);
                }
            }
            if (freeQueue.size() > 0) {
                if (!randomOrder) {
                    freeQueue[0]->play();
                }
                playing = true;
            }
        }

        virtual Amara::Node* addChild(Amara::Node* node) {
            if (!destroyed && node) {
                Amara::Audio* audio = node->as<Amara::Audio*>();
                if (audio) {
                    pool.push_back(audio);
                }
            }

            return Amara::Audio::addChild();
        }

        virtual void run() {
            Amara::Audio::run();

            playing = false;
            for (auto it = pool.begin(); it != pool.end();) {
                if (audio->destroyed || audio->parent != this) {
                    it = pool.erase(it);
                    continue;
                }
                if (audio->playing) playing = true;
                ++it;
            }
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<AudioPool>("Audio",
                sol::base_classes, sol::bases<Amara::AudioGroup, Amara::Audio, Amara::Node>(),
                "play", &AudioPool::play
            );
        }
    };
}