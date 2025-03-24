namespace Amara {
    enum class AssetEnum {
        None,
        Surface,
        Image,
        Spritesheet
    };

    class Asset {
    public:
        Asset() = default;
        
        AssetEnum type = AssetEnum::None;
        std::string key;

        bool isDestroyed = false;
    };
}