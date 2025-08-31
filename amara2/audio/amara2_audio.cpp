namespace Amara {
    class Audio: public Amara::Node {
    public:
        Amara::AudioAsset* audio = nullptr;

        float volume = 1;
        float masterVolume = 1;
        float panning = 0;

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
        float playDuration = 0;

        const float stream_expiry_time = 60;
        float stream_expiry_counter = 0;

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
            if (json_has(config, "position")) setPosition(config["position"]);
            if (json_is(config, "playing")) play();
            return Amara::Node::configure(config);
        }

        virtual void update(double deltaTime) override {
            gameProps->audioData.volume = gameProps->audioData.volume * volume * masterVolume;
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

            stream_expiry_counter = 0;

            const auto& samples = audio->samples;
            int endPoint = (int)samples.size();
            if (loop && audio->loopEnd * audio->channels < endPoint) endPoint = audio->loopEnd * audio->channels;

            size_t remaining_samples = endPoint - position;
            size_t samples_to_write = std::min(remaining_samples, (size_t)chunk_samples);

            if (samples_to_write == 0) {
                playing = false;
                return;
            }

            float leftGain  = (1.0f - panning) * 0.5f * gameProps->audioData.volume;
            float rightGain = (1.0f + panning) * 0.5f * gameProps->audioData.volume;

            // Mono processing (unrolled)
            if (audio->channels == 1) {
                size_t i = 0;
                size_t limit = samples_to_write - samples_to_write % 4;
                for (; i < limit; i += 4) {
                    size_t idx = position + i;
                    stream_chunk[i]     = samples[idx]     * gameProps->audioData.volume;
                    stream_chunk[i + 1] = samples[idx + 1] * gameProps->audioData.volume;
                    stream_chunk[i + 2] = samples[idx + 2] * gameProps->audioData.volume;
                    stream_chunk[i + 3] = samples[idx + 3] * gameProps->audioData.volume;
                }
                for (; i < samples_to_write; ++i) {
                    stream_chunk[i] = samples[position + i] * gameProps->audioData.volume;
                }
            }
            // Stereo processing (unrolled)
            else if (audio->channels == 2) {
                size_t frames_to_write = samples_to_write / 2;
                size_t f = 0;
                size_t limit = frames_to_write - frames_to_write % 4;
                for (; f < limit; f += 4) {
                    size_t idx = position + f * 2;
                    stream_chunk[f*2]     = samples[idx]     * leftGain;
                    stream_chunk[f*2 + 1] = samples[idx + 1] * rightGain;
                    stream_chunk[f*2 + 2] = samples[idx + 2] * leftGain;
                    stream_chunk[f*2 + 3] = samples[idx + 3] * rightGain;
                    stream_chunk[f*2 + 4] = samples[idx + 4] * leftGain;
                    stream_chunk[f*2 + 5] = samples[idx + 5] * rightGain;
                    stream_chunk[f*2 + 6] = samples[idx + 6] * leftGain;
                    stream_chunk[f*2 + 7] = samples[idx + 7] * rightGain;
                }
                for (; f < frames_to_write; ++f) {
                    size_t idx = position + f * 2;
                    stream_chunk[f*2]   = samples[idx]     * leftGain;
                    stream_chunk[f*2+1] = samples[idx + 1] * rightGain;
                }
            }

            SDL_PutAudioStreamData(stream, stream_chunk.data(), samples_to_write * sizeof(float));
            position += (int)samples_to_write;

            if (position >= endPoint) {
                if (loop) {
                    setPosition(audio->loopStart);
                    if (funcs.hasFunction("onLoop")) funcs.callFunction("onLoop");
                } else {
                    setPosition(0);
                    playing = false;
                    if (funcs.hasFunction("onComplete")) funcs.callFunction("onComplete");
                }
            }
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

            chunk_samples = chunk_frames * std::max(2, audio->channels);
            chunk_bytes   = chunk_samples * sizeof(float);
            stream_chunk.resize(chunk_samples);

            duration = audio->samples.size() / (float)(audio->sampleRate * audio->channels);
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
            spec.channels = (Uint8)audio->channels;
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

        virtual void play() {
            if (!audio) {
                fatal_error("Error: Attempted to play audio node without an audio asset.");
                return;
            }
            if (!stream) createAudioStream();

            if (!playing) {
                playDuration = 0;
                if (funcs.hasFunction("onPlay")) funcs.callFunction("onPlay");
            }
            else if (!paused) {
                debug_log("Note: Audio \"", id, "\" is already playing.");
            }

            playing = true;
            paused = false;
            update_group();
        }

        virtual void stop() {
            setPosition(0);
            playing = false;
            update_group();
        }

        virtual void restart() {
            playDuration = 0;
            setPosition(0);
            play();
        }

        void setPosition(int _position) {
            if (!audio) {
                position = 0;
                return;
            }
            if (loop) {
                position = (_position * audio->channels) % audio->samples.size();
            } else {
                position = std::min((_position * audio->channels), (int)audio->samples.size());
            }
        }

        void setVolume(float _volume) { volume = std::clamp(_volume, 0.0f, 1.0f); }
        void setMasterVolume(float _mvolume) { masterVolume = std::clamp(_mvolume, 0.0f, 1.0f); }
        void setPanning(float _pan) { panning = std::clamp(_pan, -1.0f, 1.0f); }

        virtual void destroy() override {
            destroyAudioStream();
            Amara::Node::destroy();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Audio>("Audio",
                sol::base_classes, sol::bases<Amara::Node>(),
                "volume", sol::property([] (Audio& a) -> float { return a.volume; }, [](Audio& a, float v) { a.setVolume(v); }),
                "masterVolume", sol::property([] (Audio& a) -> float { return a.masterVolume; }, [](Audio& a, float v) { a.setMasterVolume(v); }),
                "panning", sol::property([] (Audio& a) -> float { return a.panning; }, [](Audio& a, float v) { a.setPanning(v); }),
                "playing", sol::readonly(&Audio::playing),
                "loop", &Audio::loop,
                "duration", sol::readonly(&Audio::duration),
                "position", sol::property([] (Audio& a) -> int { return a.position; }, [](Audio& a, int v) { a.setPosition(v); }),
                "audio", sol::property([] (Audio& a) -> std::string { if (a.audio) return a.audio->key; else return ""; }, [](Audio& a, std::string key) { a.setAudio(key); }),
                "setAudio", sol::resolve<bool(std::string)>(&Audio::setAudio),
                "play", &Audio::play,
                "stop", &Audio::stop,
                "restart", &Audio::restart
            );
        }
    };
}