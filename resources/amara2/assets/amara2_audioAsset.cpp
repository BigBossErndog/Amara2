namespace Amara {
    enum class AudioFileType {
        None,
        WAV,
        OGG
    };

    class AudioAsset: public Amara::Asset {
    public:
        AudioFileType audioType = AudioFileType::None;

        std::vector<float> samples;
        int sampleRate = 0;
        int channels = 0;

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

            if (String::endsWith(path, ".wav")) {

                audioType = AudioFileType::WAV;
            }
            else if (String::endsWith(path, ".ogg")) {
                
                audioType = AudioFileType::OGG;
            }

            debug_log("Error: Audio file is not supported \"", _p, "\"");
            gameProps->breakWorld();
            return false;
        }
    };
}