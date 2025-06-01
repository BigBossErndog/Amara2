namespace Amara {
    struct Tile {
        int tileID = -1;
        Vector2 pos = { 0, 0 };
        Vector2 scale = { 1, 1 };
        float rotation = 0;
    };

    class TilemapLayer: public Amara::TextureContainer {
    public:
        ImageAsset* image = nullptr;

        int textureWidth = 0;
        int textureHeight = 0;

        int mapWidth = 0;
        int mapHeight = 0;

        int tileWidth = 0;
        int tileHeight = 0;

        int widthInPixels = 0;
        int heightInPixels = 0;

        std::vector<Tile> tiles;

        std::unordered_map<int, TMXAnimation> tmxAnimations;

        TilemapLayer(): Amara::TextureContainer() {
            set_base_node_id("TilemapLayer");
            origin = Vector2(0, 0);
            tmxAnimations.clear();
            drawOnce();
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            Amara::TextureContainer::configure(config);
            
            if (json_has(config, "tileWidth")) tileWidth = config["tileWidth"];
            if (json_has(config, "tileHeight")) tileHeight = config["tileHeight"];
            if (json_has(config, "width")) mapWidth = json_extract(config, "width");
            if (json_has(config, "height")) mapHeight = json_extract(config, "height");

            Amara::Tile tile;
            tiles.resize(mapWidth * mapHeight, tile);

            if (json_has(config, "tiles")) {
                nlohmann::json tileData = json_extract(config, "tiles");
                if (tileData.is_array()) {
                    if (tileData.size() != mapWidth * mapHeight) {
                        debug_log("Error: Given tile data does not match given TilemapLayer width and height.");
                    }
                    else {
                        for (int i = 0; i < tileData.size(); ++i) {
                            if (tileData[i].is_number()) {
                                setTile(
                                    i % mapWidth, i / mapWidth, 
                                    sol::make_object(gameProps->lua, tileData[i].get<int>())
                                );
                            }
                        }
                    }
                }
            }

            width = mapWidth * tileWidth;
            height = mapHeight * tileHeight;
            widthInPixels = width;
            heightInPixels = height;

            if (json_has(config, "texture")) setTexture(config["texture"]);

            update_canvas = true;

            return this;
        }

        bool setTexture(std::string key) {
            image = nullptr;

            if (!gameProps->assets->has(key)) {
                debug_log("Error: Asset \"", key, "\" was not found.");
                return false;
            }

            image = gameProps->assets->get(key)->as<ImageAsset*>();
            
            if (image == nullptr) {
                debug_log("Error: Asset \"", key, "\" is not a valid texture asset.");
                return false;
            }
            textureWidth = image->width;
            textureHeight = image->height;

            return true;
        }
        void setTexture(Amara::ImageAsset* _image) {
            image = _image;
            textureWidth = image->width;
            textureHeight = image->height;
        }

        void setAnimation(int tileID, const TMXAnimation& animation) {
            tmxAnimations[tileID] = animation;
            TMXAnimation& anim = tmxAnimations[tileID];
            anim.progress = 0;
            anim.currentIndex = 0;
            anim.currentTileId = anim.frames[0].tileId;
        }

        void setTile(int gx, int gy, sol::object sol_config) {
            if (gx < 0 || gx >= mapWidth || gy < 0 || gy >= mapHeight) {
                debug_log("Error: (", gx, ", ", gy, ") is out of bounds of TilemapLayer.");
                return;
            }
            nlohmann::json config = lua_to_json(sol_config);
            
            Amara::Tile& tile = tiles[gy * mapWidth + gx];
            
            if (config.is_number()) {
                tile.tileID = config.get<int>();
            }

            drawOnce();
        }

        void processAnimations(double deltaTime) {
            for (auto it = tmxAnimations.begin(); it != tmxAnimations.end(); ++it) {
                TMXAnimation& animation = it->second;
                double progress = animation.progress += deltaTime * 1000;
                progress = fmod(progress, animation.totalDuration);
                animation.currentIndex = 0;
                while (progress > animation.frames[animation.currentIndex].duration) {
                    progress -= animation.frames[animation.currentIndex].duration;
                    ++animation.currentIndex;
                }
                animation.currentTileId = animation.frames[animation.currentIndex].tileId;
                drawOnce();
            }
        }

        virtual void update(double deltaTime) override {
            processAnimations(deltaTime);
            Amara::TextureContainer::update(deltaTime);
        }

        virtual void drawCanvasContents(const Rectangle& v) override {
            if (image) {
                SDL_FRect srcRect, destRect;
                SDL_FPoint dorigin = { tileWidth/2.0f, tileHeight/2.0f };
                
                int frame = 0;
                for (int x = 0; x < mapWidth; ++x) {
                    for (int y = 0; y < mapHeight; ++y) {
                        Amara::Tile& tile = tiles[y * mapWidth + x];
                        if (tile.tileID == -1) continue;

                        frame = tile.tileID;
                        if (tmxAnimations.find(tile.tileID) != tmxAnimations.end()) {
                            frame = tmxAnimations[tile.tileID].currentTileId;
                        }

                        srcRect = {
                            static_cast<float>((frame % (textureWidth / tileWidth)) * tileWidth),
                            static_cast<float>(floor(frame / (textureWidth / tileWidth)) * tileHeight),
                            static_cast<float>(tileWidth),
                            static_cast<float>(tileHeight)
                        };
                        destRect = {
                            tile.pos.x*tileWidth + tileWidth/2.0f - (tileWidth * 0.5f)*tile.scale.x,
                            tile.pos.y*tileHeight + tileHeight/2.0f - (tileHeight * 0.5f)*tile.scale.y,
                            tileWidth*tile.scale.x,
                            tileHeight*tile.scale.y
                        };

                        if (image->texture && gameProps->renderer) {
                            // 2D Rendering
                            SDL_SetTextureScaleMode(image->texture, SDL_SCALEMODE_NEAREST);
            
                            SDL_RenderTextureRotated(
                                gameProps->renderer, 
                                image->texture,
                                &srcRect,
                                &destRect,
                                getDegrees(tile.rotation),
                                &dorigin,
                                SDL_FLIP_NONE
                            );
                        }
                        else if (image->gpuTexture && gameProps->gpuDevice) {
                            // GPU Rendering
                        }
                        #ifdef AMARA_OPENGL
                        else if (image->glTextureID != 0 && gameProps->glContext != NULL) {
                            Quad srcQuad = Quad(
                                { srcRect.x/textureWidth, srcRect.y/textureHeight },
                                { (srcRect.x+srcRect.w)/textureWidth, srcRect.y/textureHeight },
                                { (srcRect.x+srcRect.w)/textureWidth, (srcRect.y+srcRect.h)/textureHeight },
                                { srcRect.x/textureWidth, (srcRect.y+srcRect.h)/textureHeight }
                            );
                            Quad destQuad = glTranslateQuad(v, rotateQuad(
                                Quad(destRect),
                                Vector2(
                                    destRect.x + dorigin.x,
                                    destRect.y + dorigin.y
                                ),
                                tile.rotation
                            ), true);
            
                            vertices = {
                                destQuad.p1.x, destQuad.p1.y, srcQuad.p1.x, srcQuad.p1.y,
                                destQuad.p2.x, destQuad.p2.y, srcQuad.p2.x, srcQuad.p2.y,
                                destQuad.p3.x, destQuad.p3.y, srcQuad.p3.x, srcQuad.p3.y,
                                destQuad.p4.x, destQuad.p4.y, srcQuad.p4.x, srcQuad.p4.y
                            };
            
                            gameProps->renderBatch->pushQuad(
                                gameProps->currentShaderProgram,
                                image->glTextureID,
                                vertices, 1, tint,
                                v, true,
                                Amara::BlendMode::Alpha
                            );
                        }
                        #endif   
                    }
                }
                Amara::TextureContainer::drawCanvasContents(v);
            }
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<TilemapLayer>("TilemapLayer",
                sol::base_classes, sol::bases<Amara::TextureContainer, Amara::Node>(),
                "tileWidth", sol::readonly(&TilemapLayer::tileWidth),
                "tileHeight", sol::readonly(&TilemapLayer::tileHeight),
                "width", sol::readonly(&TilemapLayer::mapWidth),
                "height", sol::readonly(&TilemapLayer::mapHeight),
                "widthInPixels", sol::readonly(&TilemapLayer::widthInPixels),
                "heightInPixels", sol::readonly(&TilemapLayer::heightInPixels),
                "texture", sol::property([](Amara::TilemapLayer& t) -> std::string { if (t.image) return t.image->key; else return ""; }, [](Amara::TilemapLayer& t, std::string key) { t.setTexture(key); }),
                "setTexture", sol::resolve<bool(std::string)>(&TilemapLayer::setTexture),
                "setTile", &TilemapLayer::setTile
            );
        }
    };
}