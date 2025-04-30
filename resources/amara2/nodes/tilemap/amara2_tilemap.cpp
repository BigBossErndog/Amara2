namespace Amara {
    class Tilemap: public Amara::Group {
    public:
        Amara::Asset* asset = nullptr;
        Amara::TMXTilemapAsset* tmxAsset = nullptr;

        Amara::ImageAsset* image = nullptr;

        int mapWidth = 0;
        int mapHeight = 0;

        int tileWidth = 0;
        int tileHeight = 0;

        int pixelWidth = 0;
        int pixelHeight = 0;

        Tilemap(): Amara::Group() {
            set_base_node_id("Tilemap");
        }

        virtual Amara::Node* configure(nlohmann::json config) {
            Amara::Node::configure(config);

            if (json_has(config, "texture")) setTexture(config["texture"]);
            if (json_has(config, "tilemap")) createTilemap(config["tilemap"]);

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

            return true;
        }

        bool createTilemap(std::string key) {
            asset = nullptr;
            tmxAsset = nullptr;

            if (gameProps->assets->has(key)) {
                asset = gameProps->assets->get(key);
                tmxAsset = asset->as<Amara::TMXTilemapAsset*>();

                if (tmxAsset) {
                    createTMXTilemap();
                }

                return true;
            }
            debug_log("Error: Asset \"", key, "\" was not found.");
            return false;
        }

        void createObjects(sol::protected_function func) {
            if (!tmxAsset) {
                debug_log("Error: No tilemap set.");
                return;
            }
            if (!func.valid()) {
                debug_log("Error: Tilemap.createObjects requires a function to be passed as an argument.");
                return;
            }
            if (tmxAsset) {
                for (int i = 0; i < tmxAsset->objectGroups.size(); ++i) {
                    const Amara::TMXObjectGroup& objectGroup = tmxAsset->objectGroups[i];
                    for (int j = 0; j < objectGroup.objects.size(); ++j) {
                        const Amara::TMXObject& object = objectGroup.objects[j];
                        nlohmann::json config = nlohmann::json::object();
                        config["x"] = object.x;
                        config["y"] = object.y;
                        config["width"] = object.width;
                        config["height"] = object.height;
                        config["rotation"] = object.rotation;
                        config["visible"] = object.visible;

                        config["gid"] = object.gid;
                        config["name"] = object.name;
                        config["type"] = object.type;

                        try {
                            sol::protected_function_result result = func(get_lua_object(), json_to_lua(gameProps->lua, config));
                            if (!result.valid()) {
                                sol::error err = result;
                                throw std::runtime_error("Lua Error: " + std::string(err.what()));  
                            }
                        } catch (const std::exception& e) {
                            debug_log(e.what());
                            gameProps->breakWorld();
                        }
                    }
                }
            }
        }

        void createTMXTilemap() {
            if (tmxAsset) {
                const unsigned int FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
                const unsigned int FLIPPED_VERTICALLY_FLAG   = 0x40000000;
                const unsigned int FLIPPED_DIAGONALLY_FLAG   = 0x20000000;
                const unsigned int ALL_FLIP_FLAGS = FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG | FLIPPED_DIAGONALLY_FLAG;
                
                mapWidth = tmxAsset->width;
                mapHeight = tmxAsset->height;

                tileWidth = tmxAsset->tileWidth;
                tileHeight = tmxAsset->tileHeight;

                pixelWidth = mapWidth * tileWidth;
                pixelHeight = mapHeight * tileHeight;
                
                for (int layerIndex = 0; layerIndex < tmxAsset->layers.size(); ++layerIndex) {
                    const Amara::TMXTileLayer& layer = tmxAsset->layers[layerIndex];
                    Amara::TilemapLayer* layerNode = createChild("TilemapLayer")->as<Amara::TilemapLayer*>();
                    
                    layerNode->setTexture(image);
                    layerNode->id = layer.name;
                    layerNode->alpha = layer.opacity;
                    layerNode->mapWidth = mapWidth;
                    layerNode->mapHeight = mapHeight;
                    layerNode->tileWidth = tmxAsset->tileWidth;
                    layerNode->tileHeight = tmxAsset->tileHeight;
                    layerNode->width = layer.width * tmxAsset->tileWidth;
                    layerNode->height = layer.height * tmxAsset->tileHeight;

                    for (unsigned int ty = 0; ty < layer.height; ++ty) {
                        for (unsigned int tx = 0; tx < layer.width; ++tx) {
                            unsigned int index = ty * layer.width + tx;
                            if (index >= layer.data.size()) {
                                Amara::debug_log("Warning: Index out of bounds for layer data.");
                                continue;
                            }

                            unsigned int gid = layer.data[index];

                            Amara::Tile tile;
                            if (gid == 0) {
                                layerNode->tiles.push_back(tile);
                                continue;
                            }

                            bool flipped_horizontally = (gid & FLIPPED_HORIZONTALLY_FLAG);
                            bool flipped_vertically   = (gid & FLIPPED_VERTICALLY_FLAG);
                            bool flipped_diagonally   = (gid & FLIPPED_DIAGONALLY_FLAG); // Less common, might need special rotation/texture coord handling

                            unsigned int cleanGid = gid & ~ALL_FLIP_FLAGS;

                            const Amara::TMXTileset* tileset = tmxAsset->findTilesetForGid(cleanGid);
                            if (!tileset) {
                                Amara::debug_log("Warning: Could not find tileset for GID: ", cleanGid);
                                continue;
                            }

                            unsigned int localTileId = cleanGid - tileset->firstGid;

                            const TMXTileData* tileData = tmxAsset->getTileData(cleanGid);
                            if (tileData && tileData->animation.has_value()) {
                                if (layerNode->tmxAnimations.find(localTileId) == layerNode->tmxAnimations.end()) {
                                    const TMXAnimation& animDefinition = tileData->animation.value();
                                    layerNode->setAnimation(localTileId, animDefinition);
                                }
                            }

                            tile.tileID = localTileId;
                            tile.pos.x = tx;
                            tile.pos.y = ty;
                            
                            if (flipped_diagonally) {
                                tile.rotation = M_PI / 2.0f; 
                                tile.scale = Amara::Vector2( 1.0f, -1.0f ); 
                            }
                            
                            if (flipped_horizontally) {
                                tile.scale.x *= -1.0f;
                            }
    
                            if (flipped_vertically) {
                                tile.scale.y *= -1.0f;
                            }

                            layerNode->tiles.push_back(tile);
                        }
                    }
                }
            }
        }

        Rectangle getRectangle() {
            return Rectangle(
                pos.x, pos.y,
                pixelWidth*scale.x, 
                pixelHeight*scale.y
            );
        }

        static void bindLua(sol::state& lua) {
            lua.new_enum("TMXObjectType", 
                "Rectangle", Amara::TMXObjectType::Rectangle,
                "Ellipse", Amara::TMXObjectType::Ellipse,
                "Polygon", Amara::TMXObjectType::Polygon,
                "Polyline", Amara::TMXObjectType::Polyline,
                "Text", Amara::TMXObjectType::Text
            );

            lua.new_usertype<Tilemap>("Tilemap",
                sol::base_classes, sol::bases<Amara::Group, Amara::Node>(),
                "texture", sol::property([](Amara::Tilemap& t) -> std::string { if (t.image) return t.image->key; else return ""; }, [](Amara::Tilemap& t, std::string key) { t.setTexture(key); }),
                "setTexture", sol::resolve<bool(std::string)>(&Tilemap::setTexture),
                "tilemap", sol::property([](Amara::Tilemap& t) -> std::string { if (t.asset) return t.asset->key; else return ""; }, [](Amara::Tilemap& t, std::string key) { t.createTilemap(key); }),
                "createTilemap", sol::resolve<bool(std::string)>(&Tilemap::createTilemap),
                "width", sol::readonly(&Tilemap::mapWidth),
                "height", sol::readonly(&Tilemap::mapHeight),
                "tileWidth", sol::readonly(&Tilemap::tileWidth),
                "tileHeight", sol::readonly(&Tilemap::tileHeight),
                "pixelWidth", sol::readonly(&Tilemap::pixelWidth),
                "pixelHeight", sol::readonly(&Tilemap::pixelHeight),
                "rect", sol::property(&Tilemap::getRectangle)
            );
        }
    };
}