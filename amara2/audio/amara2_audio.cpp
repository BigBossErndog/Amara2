namespace Amara {
    class Audio: public Amara::Node {
    public:
        Amara::AudioAsset* audio = nullptr;

        float volume = 1;
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
            if (json_has(config, "panning")) setPanning(config["panning"]);
            if (json_has(config, "position")) setPosition(config["position"]);
            
            if (json_is(config, "playing")) play();

            return Amara::Node::configure(config);
        }

        virtual void update(double deltaTime) override {
            gameProps->audioData.volume = gameProps->audioData.volume * volume;
            if (gameProps->audioData.panning != 0 && panning != 0) {
                gameProps->audioData.panning = (gameProps->audioData.panning + panning)/2.0f;
            }
            else if (panning != 0) {
                gameProps->audioData.panning = panning;
            }
            
            Amara::Node::update(deltaTime);

            if (playing && audio != nullptr && stream != nullptr) {
                int queued = SDL_GetAudioStreamQueued(stream);
                if (queued == -1) {
                    debug_log("Error: Failed to stream audio \"", audio->key, "\".");
                    stop();
                    return;
                }
                else if (queued < chunk_bytes) {
                    stream_expiry_counter = 0;

                    const auto& samples = audio->samples;

                    int endPoint = (int)samples.size();
                    if (loop && audio->loopEnd * audio->channels < endPoint) {
                        endPoint = audio->loopEnd * audio->channels;
                    }

                    size_t remaining_samples = endPoint - position;
                    size_t samples_to_write = fmin(remaining_samples, (size_t)chunk_samples);

                    AudioData& audioData = gameProps->audioData;

                    if (samples_to_write == 0) {
                        playing = false;
                    }
                    else {
                        for (size_t j = 0; j < chunk_samples; ++j) {
                            stream_chunk[j] = samples[(position + j) % samples.size()];  // loop back
                        }
                        
                        float leftGain = (1.0f - audioData.panning) * 0.5f * audioData.volume;
                        float rightGain = (1.0f + audioData.panning) * 0.5f * audioData.volume;
    
                        if (audio->channels == 2) {
                            for (size_t f = 0; f < chunk_samples / 2; ++f) {
                                stream_chunk[f * 2]     *= leftGain;
                                stream_chunk[f * 2 + 1] *= rightGain;
                            }
                        }
                        else {
                            for (float& sample: stream_chunk) {
                                sample *= volume;
                            }
                        }
                        
                        SDL_PutAudioStreamData(stream, stream_chunk.data(), samples_to_write * sizeof(float));
    
                        position += samples_to_write;
                        
                        if (position >= endPoint) {
                            if (loop) {
                                setPosition(audio->loopStart);

                                if (funcs.hasFunction("onLoop")) {
                                    funcs.callFunction("onLoop");
                                }
                            }
                            else {
                                setPosition(0);
                                playing = false;

                                if (funcs.hasFunction("onComplete")) {
                                    funcs.callFunction("onComplete");
                                }
                            }
                        }
                    }
                }
                else if (stream != nullptr) {
                    stream_expiry_counter += deltaTime;
                    if (stream_expiry_counter > stream_expiry_time) {
                        destroyAudioStream();
                    }
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
            if (audio == nullptr) {
                fatal_error("Error: Asset \"", key, "\" is not an audio asset.");
                gameProps->breakWorld();
                return false;
            }

            chunk_samples = chunk_frames * audio->channels;
            chunk_bytes = chunk_samples * sizeof(float);
            stream_chunk.resize(chunk_samples);

            duration = audio->samples.size() / (float)(audio->sampleRate * audio->channels);

            if (id.empty()) id = audio->key;

            return true;
        }

        void update_group();

        void createAudioStream() {
            destroyAudioStream();
            if (audio == nullptr) return;

            SDL_zero(spec);
            spec.format = SDL_AUDIO_F32;
            spec.channels = (Uint8)audio->channels;
            spec.freq = audio->sampleRate;

            if (gameProps->audioData.device == 0) {
                fatal_error("Error: Audio device not set.");
                gameProps->breakWorld();
                return;
            }

            stream = SDL_CreateAudioStream(&spec, NULL);
            if (stream == nullptr) {
                fatal_error("Error: Couldn't create audio stream: ", SDL_GetError());
                gameProps->breakWorld();
                return;
            } 
            else if (!SDL_BindAudioStream(gameProps->audioData.device, stream)) {
                fatal_error("Error: Couldn't bind audio stream: ", SDL_GetError());
                gameProps->breakWorld();
                return;
            }
        }

        void destroyAudioStream() {
            if (stream != nullptr) {
                SDL_UnbindAudioStream(stream);
                SDL_DestroyAudioStream(stream);
                stream = nullptr;
            }
        }

        virtual void play() {
            if (audio == nullptr) {
                fatal_error("Error: Attempted to play audio node without an audio asset.");
                return;
            }
            if (stream == nullptr) {
                createAudioStream();
            }

            if (!playing) {
                playDuration = 0;

                if (funcs.hasFunction("onPlay")) {
                    funcs.callFunction("onPlay");
                }
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
            if (audio == nullptr) {
                position = 0;
                return;
            }
            if (loop) {
                position = (_position * audio->channels) % audio->samples.size();
            }
            else {
                position = fmin((_position * audio->channels), (int)audio->samples.size());
            }
        }

        void setVolume(float _volume) {
            volume = std::clamp(_volume, 0.0f, 1.0f);
        }
        void setPanning(float _pan) { 
            panning = std::clamp(_pan, -1.0f, 1.0f); 
        }

        virtual void destroy() override {
            destroyAudioStream();
            Amara::Node::destroy();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Audio>("Audio",
                sol::base_classes, sol::bases<Amara::Node>(),
                "volume", sol::property([] (Audio& a) -> float { return a.volume; }, [](Audio& a, float v) { a.setVolume(v); }),
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