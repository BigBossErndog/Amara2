namespace Amara {
    class AudioMaster: public Amara::AudioGroup {
    public:
        float volume = 1;

        std::mutex mutex;

        virtual void update(double deltaTime) {
            gameProps->audioData.reset();

            Amara::Node::update(deltaTime);
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<AudioMaster>("AudioMaster",
                sol::base_classes, sol::bases<Amara::Node>(),
                "volume", &AudioMaster::volume
            );
        }
    };
};