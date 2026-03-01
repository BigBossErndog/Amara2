namespace Amara {
    class AudioGroup: public Amara::Audio {
    public:
        Amara::Audio* currentlyPlaying = nullptr;

        AudioGroup(): Amara::Audio() {
            set_base_node_id("AudioGroup");
            is_audio_group = true;
        }

        virtual sol::object play() override {
            if (currentlyPlaying) {
                return currentlyPlaying->play();
            }
            return sol::nil;
        }
        sol::object play(std::string gid) {
            paused = false;
            Amara::Node* node = getChild(gid);
            if (node) {
                Amara::Audio* audio = node->as<Amara::Audio*>();
                if (audio) {
                    audio->play();
                    playing = true;
                    return audio->get_lua_object();
                }
            }
            
            if (gameProps->assets->has(gid)) {
                Amara::AudioAsset* asset = gameProps->assets->get(gid)->as<Amara::AudioAsset*>();
                if (asset) {
                    Amara::Audio* audio = createChild("Audio")->as<Amara::Audio*>();
                    audio->temporary = true;
                    audio->setAudio(gid);
                    audio->play();
                    playing = true;
                    return audio->get_lua_object();
                }
            }
            fatal_error("Error: Audio child \"", gid, "\" of group \"", id, "\" not found.");
            return sol::nil;
        }
        
        sol::object play(sol::table config) {
            Amara::Audio* audio = createChild("Audio")->as<Amara::Audio*>();
            audio->temporary = true;
            audio->luaConfigure(config);
            audio->play();
            playing = true;
            return audio->get_lua_object();
        }
        
        void playAll() {
            paused = false;
            for (Amara::Node* child : children) {
                if (child == nullptr || child->destroyed || child->parent != this) continue;
                
                Amara::Audio* audio = child->as<Amara::Audio*>();
                if (audio) {
                    audio->play();
                }
            }
            playing = true;
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
        
        void stop() {
            Amara::Audio::stop();
            for (Amara::Node* child : children) {
                if (child == nullptr || child->destroyed || child->parent != this) continue;
                
                Amara::Audio* audio = child->as<Amara::Audio*>();
                if (audio) {
                    audio->stop();
                    if (currentlyPlaying == audio) currentlyPlaying = nullptr;
                }
            }
        }
        void stop(std::string gid) {
            Amara::Node* node = getChild(gid);
            if (node) {
                Amara::Audio* audio = node->as<Amara::Audio*>();
                if (audio) {
                    audio->stop();
                    if (currentlyPlaying == audio) currentlyPlaying = nullptr;
                    return;
                }
            }
        }

        bool isPlaying(std::string gid) {
            Amara::Node* node = getChild(gid);
            if (node) {
                Amara::Audio* audio = node->as<Amara::Audio*>();
                if (audio) {
                    return audio->playing;
                }
            }
            return false;
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

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<AudioGroup>("AudioGroup", 
                sol::base_classes, sol::bases<Amara::Audio, Amara::Node>(),
                "play", sol::overload(
                    sol::resolve<sol::object()>(&AudioGroup::play),
                    sol::resolve<sol::object(std::string)>(&AudioGroup::play),
                    sol::resolve<sol::object(sol::table)>(&AudioGroup::play)
                ),
                "isPlaying", &AudioGroup::isPlaying,
                "pause", sol::overload(
                    sol::resolve<void()>(&AudioGroup::pause),
                    sol::resolve<void(std::string)>(&AudioGroup::pause)
                ),
                "resume", sol::overload(
                    sol::resolve<void()>(&AudioGroup::resume),
                    sol::resolve<void(std::string)>(&AudioGroup::resume)
                ),
                "stop", sol::overload(
                    sol::resolve<void()>(&AudioGroup::stop),
                    sol::resolve<void(std::string)>(&AudioGroup::stop)
                ),
                "restart", sol::overload(
                    sol::resolve<void()>(&AudioGroup::restart),
                    sol::resolve<void(std::string)>(&AudioGroup::restart)
                ),
                "currentlyPlaying", sol::property([](const AudioGroup& ag) -> sol::object { 
                    if (ag.currentlyPlaying) return ag.currentlyPlaying->get_lua_object();
                    return sol::nil; 
                })
            );
        }
    };

    void Amara::Audio::update_group() {
        if (parent && parent->is_audio_group) {
            Amara::AudioGroup* group = parent->as<Amara::AudioGroup*>();
            if (group) {
                if (playing) {
                    group->playing = true;
                    group->currentlyPlaying = this;
                }
                else if (group->currentlyPlaying == this) {
                    group->currentlyPlaying = nullptr;
                }
            }
        }
    }
}