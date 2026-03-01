namespace Amara {
    class Audio: public Amara::Node {
    public:
        Amara::AudioAsset* audio = nullptr;

        float volume = 1;
        float masterVolume = 1;
        float panning = 0;
        float pitch = 1.0f;
        float rec_pitch = 1.0f;
        
        bool playing = false;
        bool loop = false;

        int position = 0;

        SDL_AudioStream* stream = nullptr;
        std::vector<float> stream_chunk;
        SDL_AudioSpec spec;

        const int chunk_frames = 4096;
        int chunk_samples = 0;
        int chunk_bytes = 0;

        float duration = 0;

        const float stream_expiry_time = 60;
        float stream_expiry_counter = 0;

        int loopStart = 0;
        int loopEnd = 0;
        int totalFrames = 0;
        int sampleRate = 0;
        int channels = 0;
        
        bool temporary = false;

        Audio(): Amara::Node() {
            set_base_node_id("Audio");
            is_audio = true;
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "id")) id = json_extract(config, "id");
            if (json_has(config, "loop")) loop = config["loop"];
            if (json_has(config, "audio")) setAudio(config["audio"]);
            if (json_has(config, "volume")) setVolume(config["volume"]);
            if (json_has(config, "masterVolume")) setMasterVolume(config["masterVolume"]);
            if (json_has(config, "panning")) setPanning(config["panning"]);
            if (json_has(config, "pitch")) setPitch(config["pitch"]);
            if (json_has(config, "position")) setPosition(config["position"]);
            if (json_is(config, "playing")) play();
            if (json_has(config, "temporary")) temporary = config["temporary"];
            
            return Amara::Node::configure(config);
        }

        virtual void update(double deltaTime) override {
            gameProps->audioData.volume = gameProps->audioData.volume * volume * masterVolume;
            gameProps->audioData.panning = panning * (gameProps->audioData.panning + 1);
            gameProps->audioData.pitch = pitch * gameProps->audioData.pitch;
            
            Amara::Node::update(deltaTime);

            if (!playing || audio == nullptr || stream == nullptr) return;

            size_t queued = (size_t)SDL_GetAudioStreamQueued(stream);
            if (queued == (size_t)-1) {
                debug_log("Error: Failed to stream audio \"", audio->key, "\".");
                stop();
                return;
            }

            if (queued >= chunk_bytes) {
                stream_expiry_counter += deltaTime;
                if (stream_expiry_counter > stream_expiry_time && SDL_GetAudioStreamQueued(stream) <= 0) {
                    destroyAudioStream();
                } else {
                    stream_expiry_counter = 0;
                }
                return;
            }

            if (rec_pitch != gameProps->audioData.pitch) {
                SDL_SetAudioStreamFrequencyRatio(stream, gameProps->audioData.pitch);
                rec_pitch = gameProps->audioData.pitch;
            }

            stream_expiry_counter = 0;

            const auto& samples = audio->samples;
            int total_samples = (int)samples.size();
            int loop_start_sample = loopStart * channels;
            int loop_end_sample = (loop && loopEnd > loopStart && (loopEnd + 1) * channels <= total_samples)
                ? (loopEnd + 1) * channels
                : total_samples;
            
            int samples_remaining = chunk_samples;
            while (samples_remaining > 0 && playing) {
                if (!loop && position >= total_samples) {
                    if (SDL_GetAudioStreamAvailable(stream) <= 0) {
                        setPosition(0);
                        playing = false;
                        if (funcs.hasFunction("onComplete")) funcs.callFunction("onComplete");
                        if (temporary) destroy();
                    }
                    break;
                }

                int endPoint = loop ? loop_end_sample : total_samples;
                int samples_to_end = endPoint - position;
                int samples_to_write = std::min(samples_remaining, samples_to_end);

                if (samples_to_write > 0) {
                    float leftGain  = (1.0f - panning) * 0.5f * gameProps->audioData.volume;
                    float rightGain = (1.0f + panning) * 0.5f * gameProps->audioData.volume;

                    if (channels == 1) {
                        for (int i = 0; i < samples_to_write; ++i) {
                            stream_chunk[i] = samples[position + i] * gameProps->audioData.volume;
                        }
                    } else if (channels == 2) {
                        int frames_to_write = samples_to_write / 2;
                        for (int f = 0; f < frames_to_write; ++f) {
                            int idx = position + f * 2;
                            stream_chunk[f*2]   = samples[idx]     * leftGain;
                            stream_chunk[f*2+1] = samples[idx + 1] * rightGain;
                        }
                    }

                    SDL_PutAudioStreamData(stream, stream_chunk.data(), samples_to_write * sizeof(float));
                    position += samples_to_write;
                    samples_remaining -= samples_to_write;
                }

                if (loop && position >= loop_end_sample) {
                    int overshoot = position - loop_end_sample;
                    position = loop_start_sample + overshoot;
                    if (funcs.hasFunction("onLoop")) funcs.callFunction("onLoop");
                }
            }
            
            update_group();
        }

        bool setAudio(std::string key) {
            audio = nullptr;
            destroyAudioStream();
            if (destroyed) return false;
            if (!gameProps->assets->has(key)) {
                fatal_error("Error: Asset \"", key, "\" not found.");
                gameProps->breakWorld();
                return false;
            }
            audio = gameProps->assets->get(key)->as<Amara::AudioAsset*>();
            if (!audio || audio->samples.empty()) {
                fatal_error("Error: Audio asset invalid or empty.");
                gameProps->breakWorld();
                return false;
            }
            
            channels = audio->channels;

            chunk_samples = chunk_frames * channels;
            chunk_bytes   = chunk_samples * sizeof(float);
            stream_chunk.resize(chunk_samples);
            
            duration = audio->samples.size() / (float)(audio->sampleRate * channels);
            loopStart = std::max(0, audio->loopStart);
            loopEnd = std::min((int)audio->samples.size() / channels, audio->loopEnd);
            totalFrames = audio->totalFrames;
            sampleRate = audio->sampleRate;
            
            if (id.empty()) id = audio->key;

            return true;
        }

        virtual void runChildren(double deltaTime) override {
            if (children.empty()) return;
            AudioData rec_audio_data = gameProps->audioData;
            children_copy_list = children;

            for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                if (destroyed) break;
                update_properties();

                Amara::Node* child = *it;
                if (!child || child->destroyed || child->parent != this || child->paused) {
                    ++it;
                    continue;
                }
                if (child->pauseOnce) {
                    child->pauseOnce = false;
                    ++it;
                    continue;
                }

                child->run(deltaTime * child->speed);
                gameProps->audioData = rec_audio_data;
                ++it;
                if (destroyed) break;
            }
        }

        void createAudioStream() {
            destroyAudioStream();
            if (!audio) return;

            SDL_zero(spec);
            spec.format   = SDL_AUDIO_F32;
            spec.channels = (Uint8)channels;
            spec.freq     = audio->sampleRate;

            if (gameProps->audioData.device == 0) {
                fatal_error("Error: Audio device not set.");
                gameProps->breakWorld();
                return;
            }

            SDL_AudioSpec devspec;
            SDL_zero(devspec);
            int sample_frames = 0; 
            if (!SDL_GetAudioDeviceFormat(gameProps->audioData.device, &devspec, &sample_frames)) {
                fatal_error("Error: Couldn't get audio device format: ", SDL_GetError());
                gameProps->breakWorld();
                return;
            }

            stream = SDL_CreateAudioStream(&spec, &devspec);
            if (!stream) {
                fatal_error("Error: Couldn't create audio stream: ", SDL_GetError());
                gameProps->breakWorld();
                return;
            }
            if (!SDL_BindAudioStream(gameProps->audioData.device, stream)) {
                fatal_error("Error: Couldn't bind audio stream: ", SDL_GetError());
                gameProps->breakWorld();
                return;
            }
        }

        void destroyAudioStream() {
            if (stream) {
                SDL_UnbindAudioStream(stream);
                SDL_DestroyAudioStream(stream);
                stream = nullptr;
            }
        }

        void update_group();

        virtual sol::object play() {
            if (!audio) {
                fatal_error("Error: Attempted to play audio node without an audio asset.");
                return sol::nil;
            }
            if (!stream) createAudioStream();

            if (!playing) {
                if (funcs.hasFunction("onPlay")) funcs.callFunction("onPlay");
            }
            else if (!paused) {
                debug_log("Note: Audio \"", id, "\" is already playing.");
            }

            playing = true;
            paused = false;
            update_group();
            
            return get_lua_object();
        }

        virtual void stop() {
            setPosition(0);
            playing = false;
            update_group();
            if (temporary) destroy();
        }

        virtual void restart() {
            setPosition(0);
            play();
        }

        void setPosition(int _position) {
            if (!audio) {
                position = 0;
                return;
            }
            if (stream) {
                SDL_ClearAudioStream(stream);
            }
            int max_position = audio->samples.size() / channels;
            _position = std::clamp(_position, 0, max_position);
            position = _position * channels;
        }

        void setVolume(float _volume) { volume = std::clamp(_volume, 0.0f, 1.0f); }
        void setMasterVolume(float _mvolume) { masterVolume = std::clamp(_mvolume, 0.0f, 1.0f); }
        void setPanning(float _pan) { panning = std::clamp(_pan, -1.0f, 1.0f); }

        void setPitch(float _pitch) {
            pitch = std::max(0.0f, _pitch);
        }

        virtual void destroy() override {
            playing = false;
            destroyAudioStream();
            update_group();
            Amara::Node::destroy();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Audio>("Audio",
                sol::base_classes, sol::bases<Amara::Node>(),
                "volume", sol::property([] (Audio& a) -> float { return a.volume; }, [](Audio& a, float v) { a.setVolume(v); }),
                "masterVolume", sol::property([] (Audio& a) -> float { return a.masterVolume; }, [](Audio& a, float v) { a.setMasterVolume(v); }),
                "panning", sol::property([] (Audio& a) -> float { return a.panning; }, [](Audio& a, float v) { a.setPanning(v); }),
                "pitch", sol::property([] (Audio& a) -> float { return a.pitch; }, [](Audio& a, float v) { a.setPitch(v); }),
                "playing", sol::readonly(&Audio::playing),
                "loop", &Audio::loop,
                "loopStart", &Audio::loopStart,
                "loopEnd", &Audio::loopEnd,
                "totalFrames", sol::readonly(&Audio::totalFrames),
                "sampleRate", sol::readonly(&Audio::sampleRate),
                "channels", sol::readonly(&Audio::channels),
                "duration", sol::readonly(&Audio::duration),
                "position", sol::property([] (Audio& a) -> int { return a.position; }, [](Audio& a, int v) { a.setPosition(v); }),
                "audio", sol::property([] (Audio& a) -> std::string { if (a.audio) return a.audio->key; else return ""; }, [](Audio& a, std::string key) { a.setAudio(key); }),
                "setAudio", sol::resolve<bool(std::string)>(&Audio::setAudio),
                "play", &Audio::play,
                "stop", &Audio::stop,
                "restart", &Audio::restart,
                "temporary", &Audio::temporary
            );
        }
    };
}