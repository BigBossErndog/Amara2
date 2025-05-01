namespace Amara {
    class AudioGroup: public Amara::Audio {
    public:
        Amara::Audio* currentlyPlaying = nullptr;

        AudioGroup(): Amara::Audio() {
            set_base_node_id("AudioGroup");
            is_audio_group = true;
        }

        virtual void play() override {
            if (currentlyPlaying) {
                currentlyPlaying->play();
            }
        }
        void play(std::string gid) {
            Amara::Node* node = getChild(gid);
            if (node) {
                Amara::Audio* audio = node->as<Amara::Audio*>();
                if (audio) {
                    audio->play();
                    playing = true;
                }
            }
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<AudioGroup>("AudioGroup", 
                sol::base_classes, sol::bases<Amara::Audio, Amara::Node>(),
                "play", sol::overload(
                    sol::resolve<void()>(&AudioGroup::play),
                    sol::resolve<void(std::string)>(&Audiogroup::play)
                )
            );
        }
    };

    void Amara::Audio::update_group() {
        if (playing && parent && parent->is_audio_group) {
            Amara::AudioGroup* group = parent->as<Amara::AudioGroup*>();
            if (group) {
                group->currentlyPlaying = this;
            }
        }
    }
}