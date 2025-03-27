namespace Amara {
    class AudioGroup: public Amara::Node {
    public:
        float volume = 1;

        AudioGroup(): Amara::Node() {
            set_base_node_id("AudioGroup");
            is_audio_group = true;
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<AudioGroup>("AudioGroup", 
                "volume", &AudioGroup::volume
            );
        }
    };
}