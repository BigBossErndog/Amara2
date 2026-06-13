namespace Amara {
    class AssetManager {
    public:
        Amara::GameProps* gameProps = nullptr;

        Amara::SinglePixelAsset* whitePixel = nullptr;
        Amara::FontAsset* defaultFont = nullptr;

        std::unordered_map<std::string, Amara::Asset*> assets;

        sol::object luaobject;
        sol::table shader_table;
        
        AssetManager() = default;

        void init(Amara::GameProps* _gameProps) {
            gameProps = _gameProps;

            whitePixel = new Amara::SinglePixelAsset(gameProps);
            add("whitePixel", whitePixel);
            
            sol::state& lua = gameProps->lua;
            
            luaobject = sol::make_object(lua, this);
            
            shader_table = lua.create_table();
            sol::table shader_meta = lua.create_table();
            shader_meta["__index"] = [this](sol::table tbl, sol::object key) -> sol::object {
                if (key.is<std::string>()) {
                    #ifdef AMARA_OPENGL
                    return this->getShaderProgram(key.as<std::string>())->lua_object;
                    #endif
                }
                return sol::nil;
            };
            shader_table[sol::metatable_key] = shader_meta;
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
        
        sol::object getTextureData(std::string key) {
            if (has(key)) {
                Amara::ImageAsset* image = get(key)->as<Amara::ImageAsset*>();
                if (image == nullptr) {
                    fatal_error("Error: Asset \"", key, "\" is not a valid texture.");
                }
                nlohmann::json details = nlohmann::json::object();
                details["key"] = key;
                details["width"] = image->width;
                details["height"] = image->height;
                
                Amara::SpritesheetAsset* spritesheet = image->as<Amara::SpritesheetAsset*>();
                if (spritesheet != nullptr) {
                    details["frameWidth"] = spritesheet->frameWidth;
                    details["frameHeight"] = spritesheet->frameHeight;
                }
                return json_to_lua(gameProps->lua, details);
            }
            return sol::nil;
        }

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

        void resizeTexture(std::string key, int width, int height) {
            if (has(key)) {
                Amara::ImageAsset* image = get(key)->as<Amara::ImageAsset*>();
                if (image == nullptr) {
                    fatal_error("Error: Asset \"", key, "\" is not a valid texture.");
                }
                image->resize(width, height, true);
            }
            else {
                fatal_error("Error: Asset \"", key, "\" was not found.");
            }
        }
        bool resizeToPNG(std::string key, int width, int height, std::string path) {
            if (has(key)) {
                Amara::ImageAsset* image = get(key)->as<Amara::ImageAsset*>();
                if (image == nullptr) {
                    fatal_error("Error: Asset \"", key, "\" is not a valid texture.");
                }
                unsigned char* new_data = image->resize(width, height);
                int png_len = 0;
                unsigned char* png_data = stbi_write_png_to_mem(
                    new_data,
                    width * image->channels,
                    width,
                    height,
                    image->channels,
                    &png_len
                );

                std::string result(reinterpret_cast<char*>(png_data), png_len);
                STBI_FREE(png_data);
                delete[] new_data;

                return gameProps->system->writeFile(path, result);
            }
            else {
                fatal_error("Error: Asset \"", key, "\" was not found.");
            }
            return false;
        }
        bool textureToPNG(std::string key, std::string path) {
            if (has(key)) {
                Amara::ImageAsset* image = get(key)->as<Amara::ImageAsset*>();
                if (image == nullptr) {
                    fatal_error("Error: Asset \"", key, "\" is not a valid texture.");
                }
                
                int png_len = 0;
                unsigned char* png_data = stbi_write_png_to_mem(
                    image->imageData,
                    image->width * image->channels,
                    image->width,
                    image->height,
                    image->channels,
                    &png_len
                );

                std::string result(reinterpret_cast<char*>(png_data), png_len);
                STBI_FREE(png_data);

                return gameProps->system->writeFile(path, result);
            }
            else {
                fatal_error("Error: Asset \"", key, "\" was not found.");
            }
            return false;
        }


        void clear() {
            for (auto it = assets.begin(); it != assets.end(); it++) {
                Amara::Asset* a = it->second;
                destroyAsset(a);
            }
            assets.clear();
        }

        #ifdef AMARA_OPENGL
        ShaderProgram* getShaderProgram(std::string key);
        #endif

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
                #ifdef AMARA_OPENGL
                "getShaderProgram", [](Amara::AssetManager& a, sol::object key) -> sol::object {
                    if (key.is<std::string>()) {
                        ShaderProgram* shader = a.getShaderProgram(key.as<std::string>());
                        if (shader) return shader->lua_object;
                    }
                    return sol::nil;
                },
                #endif
                "shaderPrograms", sol::readonly(&Amara::AssetManager::shader_table),
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
                },
                "getTextureData", &Amara::AssetManager::getTextureData,
                "resizeTexture", &Amara::AssetManager::resizeTexture,
                "resizeTextureToPNG", &Amara::AssetManager::resizeToPNG,
                "textureToPNG", &Amara::AssetManager::textureToPNG
            );
        }
    };

    template <typename T>
    T Asset::as() {
        return dynamic_cast<T>(this);
    }
}