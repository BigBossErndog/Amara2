namespace Amara {
    enum class AssetEnum {
        None,
        Surface,
        Image,
        Spritesheet,

    };

    class Asset {
    public:
        AssetEnum type = AssetEnum::None;
        std::string key;

        bool isDestroyed = false;

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Asset>("Asset",
                "key", &Asset::key
            );
        }
    };
}