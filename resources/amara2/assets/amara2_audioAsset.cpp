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
            
            if (String::endsWith(path, ".wav")) {
                
            }
            else if (String::endsWith(path, ".ogg")) {

            }

            debug_log("Error: Audio file is not supported ", _p);
            gameProps->breakWorld();
            return false;
        }
    };
}