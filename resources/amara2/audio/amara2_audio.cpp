namespace Amara {
    class Audio: public Amara::Node {
    public:
        AudioAsset* asset = nullptr;

        float volume = 1;
        float panning = 1;

        bool playing = false;

        bool loop = false;

        int position = 0;

        Audio(): Amara::Node() {
            set_base_node_id("Audio");
            is_audio = true;
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "loop")) loop = config["loop"];
            return Amara::Node::configure(config);
        }

        virtual void update(double deltaTime) override {
            gameProps->audioData.volume = gameProps->audioData.volume * volume;
            gameProps->audioData.panning = (gameProps->audioData.panning + panning)/2.0f;
            
            Amara::Node::update(deltaTime);

            gameProps->audioData.audio = this;
            gameProps->audioQueue.push_back(gameProps->audioData);
        }

        void update_group();

        virtual void play() {
            playing = true;
            paused = false;
            update_group();
        }

        virtual void stop() {
            playing = false;
        }

        virtual void reset() {
            position = 0;
            playing = false;
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Audio>("Audio",
                sol::base_classes, sol::bases<Amara::Node>(),
                "volume", &Audio::volume,
                "playing", &Audio::playing
            );
        }
    };
}