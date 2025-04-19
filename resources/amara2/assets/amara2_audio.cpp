namespace Amara {
    enum class AudioFileType {
        WAV,
        OGG
    };

    class AudioAsset: public Amara::Asset {
    public:
        AudioAsset(Amara::GameProps* _gameProps): Amara::Asset(_gameProps) {
            type = AssetEnum::Audio;
            typeKey = "AudioAsset";
        }

        bool loadAudio(std::string _p) {
            std::string path = gameProps->system->getAssetPath(_p);
            
            if (string_endsWith(path, ".wav")) {
                
            }
            else if (string_endsWith(path, ".ogg")) {

            }

            debug_log("Error: Audio file is not supported ", _p);
            gameProps->breakWorld();
            return false;
        }
    };
}