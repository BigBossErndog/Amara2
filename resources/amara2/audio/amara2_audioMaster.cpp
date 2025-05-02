namespace Amara {
    class AudioMaster: public Amara::AudioGroup {
    public:
        float volume = 1;

        SDL_AudioDeviceID device = 0;

        std::vector<Amara::AudioData> audioQueue;
        std::vector<float> buffer;

        AudioMaster(): Amara::AudioGroup() {
            set_base_node_id("AudioMaster");
            id = "AudioMaster";
        }

        virtual void update_properties() override {
            gameProps->audioData.device = device;
        }

        virtual void init() override {
            Amara::AudioGroup::init();
            if (device == 0) {
                device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
                if (device == 0) {
                    debug_log("Error: Failed to open audio device: ", SDL_GetError());
                    gameProps->breakWorld();
                    return;
                }
                gameProps->audioData.device = device;
            }
        }

        virtual void run(double deltaTime) {
            gameProps->audioData.reset();

            Amara::Node::run(deltaTime);

            audioQueue = gameProps->audio_buffer;
            gameProps->audio_buffer.clear();
        }

        virtual void destroy() {
            if (device != 0) {
                SDL_CloseAudioDevice(device);
                device = 0;
            }
            Amara::Node::destroy();
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<AudioMaster>("AudioMaster",
                sol::base_classes, sol::bases<Amara::AudioGroup, Amara::Node>(),
                "volume", &AudioMaster::volume
            );
        }
    };
};