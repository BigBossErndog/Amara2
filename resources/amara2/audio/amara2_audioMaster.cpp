namespace Amara {
    class AudioMaster: public Amara::Node {
    public:
        float volume = 1;

        static void bindLua(sol::state& lua) {
            lua.new_usertype<AudioMaster>("AudioMaster",
                sol::base_classes, sol::bases<Amara::Node>(),
                "volume", &AudioMaster::volume
            );
        }
    };
};