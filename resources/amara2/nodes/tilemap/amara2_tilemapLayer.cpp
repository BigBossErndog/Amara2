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

        std::vector<Tile> tiles;

        TilemapLayer(): Amara::TextureContainer() {
            set_base_node_id("TilemapLayer");
            origin = Vector2(0, 0);
            paintOnce();
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

        virtual void drawCanvasContents(const Rectangle& v) override {
            if (image) {
                SDL_FRect srcRect, destRect;
                SDL_FPoint dorigin = { tileWidth/2.0f, tileHeight/2.0f };
                
                for (int x = 0; x < mapWidth; ++x) {
                    for (int y = 0; y < mapHeight; ++y) {
                        Amara::Tile& tile = tiles[y * mapWidth + x];
                        if (tile.tileID == -1) continue;

                        srcRect = {
                            static_cast<float>((tile.tileID % (textureWidth / tileWidth)) * tileWidth),
                            static_cast<float>(floor(tile.tileID / (textureWidth / tileWidth)) * tileHeight),
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

        static void bindLua(sol::state& lua) {
            lua.new_usertype<TilemapLayer>("TilemapLayer",
                sol::base_classes, sol::bases<Amara::TextureContainer, Amara::Node>(),
                "tileWidth", sol::readonly(&TilemapLayer::tileWidth),
                "tileHeight", sol::readonly(&TilemapLayer::tileHeight),
                "mapWidth", sol::readonly(&TilemapLayer::mapWidth),
                "mapHeight", sol::readonly(&TilemapLayer::mapHeight),
                "texture", sol::property([](Amara::TilemapLayer& t) -> std::string { if (t.image) return t.image->key; else return ""; }, [](Amara::TilemapLayer& t, std::string key) { t.setTexture(key); }),
                "setTexture", sol::resolve<bool(std::string)>(&TilemapLayer::setTexture)
            );
        }
    };
}