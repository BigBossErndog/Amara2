namespace Amara {
public:
    class AudioMaster {
    public:
        float volume = 1;

        static void bindLua(sol::state& lua) {
            lua.new_usertype<AudioMaster>("AudioMaster",
                "volume", &AudioMaster::volume
            );
        }
    };
};