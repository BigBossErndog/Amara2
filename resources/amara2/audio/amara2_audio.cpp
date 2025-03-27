namespace Amara {
    class Audio: public Amara::Node {
    public:
        float volume = 1;

        Audio(): Amara::Node() {
            set_base_node_id("Audio");
            is_audio = true;
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Audio>("Audio",
                "volume", &Audio::volume
            );
        }
    };
}