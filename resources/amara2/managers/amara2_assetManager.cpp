namespace Amara {
    class AssetManager {
    public:
        Amara::GameProps* gameProps = nullptr;

        std::unordered_map<std::string, Amara::Asset*> assets;
        
        AssetManager() = default;

        Amara::Asset* get(std::string key) {
            if (assets.find(key) != assets.end()) {
                return assets[key];
            }
            return nullptr;
        }

        bool has(std::string key) {
            return assets.find(key) != assets.end();
        }

        void add(std::string key, Amara::Asset* asset) {
            if (has(key) && get(key) != asset) {
                destroyAsset(get(key));
            }
            asset->key = key;
            assets[key] = asset;
        }

        void removeAsset(std::string key) {
            if (has(key)) destroyAsset(assets[key]);
            assets.erase(key);
        }

        void destroyAsset(Amara::Asset* asset) {
            if (asset->destroyed) return;
            asset->destroy();
            gameProps->queue_asset_garbage(asset);
        }

        void clear() {
            for (auto it = assets.begin(); it != assets.end(); it++) {
                Amara::Asset* a = it->second;
                destroyAsset(a);
            }
            assets.clear();
        }
        
        static void bindLua(sol::state& lua) {
            lua.new_usertype<AssetManager>("AssetManager",
                "has", &AssetManager::has
            );
        }
    };

    template <typename T>
    T Asset::as() {
        return dynamic_cast<T>(this);
    }
}