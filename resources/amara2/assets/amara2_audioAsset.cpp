namespace Amara {
    enum class AudioFileType {
        None,
        WAV,
        OGG,
        MP3
    };

    class AudioAsset: public Amara::Asset {
    public:
        AudioFileType audioType = AudioFileType::None;

        std::vector<float> samples;
        int sampleRate = 0;
        int channels = 0;

        int loopStart = 0;
        int loopEnd = 0;

        AudioAsset(Amara::GameProps* _gameProps): Amara::Asset(_gameProps) {
            type = AssetEnum::Audio;
            typeKey = "AudioAsset";
        }

        bool loadAudio(std::string _p) {
            path = gameProps->system->getAssetPath(_p);

            if (!gameProps->system->fileExists(path)) {
                debug_log("Error: Audio file not found \"", path, "\".");
                gameProps->breakWorld();
                return false;
            }

            samples.clear();
            sampleRate = 0;
            channels = 0;

            loopStart = 0;
            loopEnd = 0;

            if (String::endsWith(path, ".wav")) {
                SDL_AudioSpec spec;
                Uint8* buffer;
                Uint32 length;

                if (!SDL_LoadWAV_IO(SDL_IOFromFile(path.c_str(), "rb"), 1, &spec, &buffer, &length))
                    return false;

                if (spec.format != SDL_AUDIO_S16) {
                    SDL_free(buffer);
                    return false;
                }

                Sint16* src = (Sint16*)buffer;
                int count = length / sizeof(Sint16);
                samples.resize(count);
                for (int i = 0; i < count; ++i) {
                    samples[i] = src[i] / 32768.0f;
                }

                sampleRate = spec.freq;
                channels = spec.channels;
                SDL_free(buffer);

                loopEnd = samples.size();

                SDL_IOStream* rw = SDL_IOFromFile(path.c_str(), "rb");
                if (!rw) return false;

                char chunk[5] = {};
                while (SDL_ReadIO(rw, chunk, 4) == 4) {
                    chunk[4] = '\0';
                    Uint32 size = 0;

                    if (SDL_ReadIO(rw, &size, 4) != 4) {
                        break;
                    }

                    if (String::equal(std::string(chunk), "smpl")) {
                        Uint32 data_offset = 28;
                        if (size >= data_offset + 8) {
                            SDL_SeekIO(rw, data_offset, SDL_IO_SEEK_CUR);

                            Uint32 start = 0, end = 0;
                            if (SDL_ReadIO(rw, &start, 4) != 4 || SDL_ReadIO(rw, &end, 4) != 4) {
                                debug_log("Error: Failed to read loop start/end values.");
                                break;
                            }

                            loopStart = start;
                            loopEnd = end;

                            if (loopStart >= samples.size() || loopEnd > samples.size()) {
                                debug_log("Error: Loop start/end values are out of bounds.");
                                loopStart = 0;
                                loopEnd = samples.size();
                            }
                        } else {
                            debug_log("Error: smpl chunk is too small to contain loop data.");
                        }
                        break;
                    } else {
                        SDL_SeekIO(rw, size, SDL_IO_SEEK_CUR);
                    }
                }

                SDL_CloseIO(rw);

                audioType = AudioFileType::WAV;
                return true;
            }
            else if (String::endsWith(path, ".ogg")) {
                int error;
                stb_vorbis* v = stb_vorbis_open_filename(path.c_str(), &error, nullptr);
                if (!v) return false;

                stb_vorbis_info info = stb_vorbis_get_info(v);
                sampleRate = info.sample_rate;
                channels = info.channels;
                int count = stb_vorbis_stream_length_in_samples(v) * channels;
                samples.resize(count);
                stb_vorbis_get_samples_float_interleaved(v, channels, samples.data(), count);
                
                loopEnd = samples.size();

                stb_vorbis_comment comments = stb_vorbis_get_comment(v);
                for (int i = 0; i < comments.comment_list_length; ++i) {
                    std::string c = comments.comment_list[i];
                    if (String::startsWith(c, "LOOPSTART=")) loopStart = std::stoi(c.substr(10));
                    if (String::startsWith(c, "LOOPEND=")) loopEnd = std::stoi(c.substr(8));
                }
                
                stb_vorbis_close(v);

                audioType = AudioFileType::OGG;
                return true;
            }
            else if (String::endsWith(path, ".mp3")) {
                mp3dec_ex_t dec;
                if (mp3dec_ex_open(&dec, path.c_str(), MP3D_SEEK_TO_SAMPLE)) return false;

                int count = dec.samples * dec.info.channels;
                std::vector<short> temp(count);
                mp3dec_ex_read(&dec, temp.data(), count);

                samples.resize(count);
                for (int i = 0; i < count; ++i)
                    samples[i] = temp[i] / 32768.0f;

                sampleRate = dec.info.hz;
                channels = dec.info.channels;
                mp3dec_ex_close(&dec);

                loopEnd = samples.size();

                audioType = AudioFileType::MP3;
                return true;
            }

            debug_log("Error: Audio file is not supported \"", _p, "\"");
            gameProps->breakWorld();
            return false;
        }

        void clear() {
            samples.clear();
            sampleRate = 0;
            channels = 0;
        }

        virtual void destroy() override {
            clear();
            Amara::Asset::destroy();
        }
    };
}