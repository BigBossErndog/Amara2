namespace Amara {
    enum class AudioFileType {
        WAV,
        OGG
    };

    class AudioAsset: public Amara::Asset {
    public:
        AudioAsset(): Amara::Asset() {
            type = AssetEnum::Audio;
            typeKey = "AudioAsset";
        }

        bool loadAudio(std::string _p) {
            std::string path = Props::system->getAssetPath(_p);

            if (string_endsWith(path, ".wav")) {
                
            }
            else if (string_endsWith(path, ".ogg")) {

            }

            debug_log("Error: Audio file is not supported ", _p);
            Props::breakWorld();
            return false;
        }
    };
}