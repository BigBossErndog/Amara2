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
            paused = false;
            Amara::Node* node = getChild(gid);
            if (node) {
                Amara::Audio* audio = node->as<Amara::Audio*>();
                if (audio) {
                    audio->play();
                    playing = true;
                    return;
                }
            }
            debug_log("Error: Audio child \"", gid, "\" of group \"", id, "\" not found.");
        }

        void pause() {
            Amara::Audio::pause();
        }
        void pause(std::string gid) {
            Amara::Node* node = getChild(gid);
            if (node) {
                Amara::Audio* audio = node->as<Amara::Audio*>();
                if (audio) {
                    audio->pause();
                    return;
                }
            }
        }
        
        void resume() {
            Amara::Audio::resume();
        }
        void resume(std::string gid) {
            Amara::Node* node = getChild(gid);
            if (node) {
                Amara::Audio* audio = node->as<Amara::Audio*>();
                if (audio) {
                    audio->resume();
                    return;
                }
            }
        }
        
        void togglePause() {
            Amara::Audio::togglePause();
        }
        void togglePause(std::string gid) {
            Amara::Node* node = getChild(gid);
            if (node) {
                Amara::Audio* audio = node->as<Amara::Audio*>();
                if (audio) {
                    audio->togglePause();
                    return;
                }
            }
        }

        void stop() {
            Amara::Audio::stop();
        }
        void stop(std::string gid) {
            Amara::Node* node = getChild(gid);
            if (node) {
                Amara::Audio* audio = node->as<Amara::Audio*>();
                if (audio) {
                    audio->stop();
                    return;
                }
            }
        }

        void restart() {
            Amara::Audio::restart();
        }
        void restart(std::string gid) {
            Amara::Node* node = getChild(gid);
            if (node) {
                Amara::Audio* audio = node->as<Amara::Audio*>();
                if (audio) {
                    audio->restart();
                    return;
                }
            }
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<AudioGroup>("AudioGroup", 
                sol::base_classes, sol::bases<Amara::Audio, Amara::Node>(),
                "play", sol::overload(
                    sol::resolve<void()>(&AudioGroup::play),
                    sol::resolve<void(std::string)>(&AudioGroup::play)
                ),
                "pause", sol::overload(
                    sol::resolve<void()>(&AudioGroup::pause),
                    sol::resolve<void(std::string)>(&AudioGroup::pause)
                ),
                "resume", sol::overload(
                    sol::resolve<void()>(&AudioGroup::resume),
                    sol::resolve<void(std::string)>(&AudioGroup::resume)
                ),
                "togglePause", sol::overload(
                    sol::resolve<void()>(&AudioGroup::togglePause),
                    sol::resolve<void(std::string)>(&AudioGroup::togglePause)
                ),
                "stop", sol::overload(
                    sol::resolve<void()>(&AudioGroup::stop),
                    sol::resolve<void(std::string)>(&AudioGroup::stop)
                ),
                "restart", sol::overload(
                    sol::resolve<void()>(&AudioGroup::restart),
                    sol::resolve<void(std::string)>(&AudioGroup::restart)
                )
            );
        }
    };

    void Amara::Audio::update_group() {
        if (parent && parent->is_audio_group) {
            Amara::AudioGroup* group = parent->as<Amara::AudioGroup*>();
            if (group) {
                if (playing) group->currentlyPlaying = this;
                else if (group->currentlyPlaying == this) group->currentlyPlaying = nullptr;
            }
        }
    }
}