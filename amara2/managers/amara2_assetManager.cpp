namespace Amara {
    class AssetManager {
    public:
        Amara::GameProps* gameProps = nullptr;

        Amara::SinglePixelAsset* whitePixel = nullptr;
        Amara::FontAsset* defaultFont = nullptr;

        std::unordered_map<std::string, Amara::Asset*> assets;

        sol::object luaobject;
        
        AssetManager() = default;

        void init(Amara::GameProps* _gameProps) {
            gameProps = _gameProps;

            whitePixel = new Amara::SinglePixelAsset(gameProps);
            add("whitePixel", whitePixel);
            
            luaobject = sol::make_object(gameProps->lua, this);
        }
        
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

        void createTexture(std::string key, sol::table luaconfig);
        void updateTexture(std::string key, sol::table luaconfig);

        void setDefaultFont(std::string key) {
            if (has(key)) {
                defaultFont = get(key)->as<Amara::FontAsset*>();
                if (defaultFont == nullptr) {
                    fatal_error("Error: Asset \"", key, "\" is not a valid font asset.");
                }
            }
            else {
                fatal_error("Error: Asset \"", key, "\" was not found.");
            }
        }

        void clear() {
            for (auto it = assets.begin(); it != assets.end(); it++) {
                Amara::Asset* a = it->second;
                destroyAsset(a);
            }
            assets.clear();
        }

        ShaderProgram* getShaderProgram(std::string key);
        
        static void bind_lua(sol::state& lua) {
            lua.new_usertype<AssetManager>("AssetManager",
                "has", &AssetManager::has,
                "remove", &AssetManager::removeAsset,
                "clear", &AssetManager::clear,
                "createTexture", &AssetManager::createTexture,
                "updateTexture", &AssetManager::updateTexture,
                "setDefaultFont", &AssetManager::setDefaultFont,
                "defaultFont", sol::property(
                    [&lua] (AssetManager& self) -> sol::object {
                        if (self.defaultFont) return sol::make_object(lua, self.defaultFont->key);
                        return sol::nil;
                    },
                    [] (AssetManager& self, std::string font) {
                        self.setDefaultFont(font);
                    }
                ),
                "getShaderProgram", &AssetManager::getShaderProgram,
                "getTilemapData", [&lua](AssetManager* self, std::string key) -> sol::object {
                    if (self->has(key)) {
                        Amara::Asset* asset = self->get(key);
                        Amara::TMXTilemapAsset* tmx_asset = asset->as<Amara::TMXTilemapAsset*>();
                        if (tmx_asset != nullptr) {
                            sol::table tbl = lua.create_table();
                            tbl["key"] = key;
                            tbl["width"] = tmx_asset->width;
                            tbl["height"] = tmx_asset->height;
                            tbl["tileWidth"] = tmx_asset->tileWidth;
                            tbl["tileHeight"] = tmx_asset->tileHeight;
                            tbl["widthInPixels"] = tmx_asset->width * tmx_asset->tileWidth;
                            tbl["heightInPixels"] = tmx_asset->height * tmx_asset->tileHeight;
                            return tbl;
                        }
                    }
                    return sol::nil;
                }
            );
        }
    };

    template <typename T>
    T Asset::as() {
        return dynamic_cast<T>(this);
    }
}