namespace Amara {
    class Tilemap: public Amara::Group, public Amara::CustomCollider {
    public:
        Amara::Asset* asset = nullptr;
        Amara::TMXTilemapAsset* tmxAsset = nullptr;

        Amara::ImageAsset* image = nullptr;

        int mapWidth = 0;
        int mapHeight = 0;

        int tileWidth = 0;
        int tileHeight = 0;

        int widthInPixels = 0;
        int heightInPixels = 0;

        int partitionWidth = 16;
        int partitionHeight = 16;

        float left = 0;
        float right = 0;
        float top = 0;
        float bottom = 0;

        sol::object objects = sol::nil;

        Vector2 origin = Vector2(0.5, 0.5);

        std::vector<unsigned int> fillMap;

        Tilemap(): Amara::Group() {
            set_base_node_id("Tilemap");
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            Amara::Group::configure(config);

            if (json_has(config, "origin")) origin = config["origin"];
            if (json_has(config, "texture")) setTexture(config["texture"]);
            if (json_has(config, "tilemap")) createTilemap(config["tilemap"]);

            update_size();

            return this;
        }

        virtual sol::object luaConfigure(std::string key, sol::object val) override {
            if (val.is<sol::function>()) {
                if (String::equal(key, "objects")) createObjects(val.as<sol::protected_function>());
                else if (String::equal(key, "createObjects")) createObjects(val.as<sol::protected_function>());
            }

            return Amara::Group::luaConfigure(key, val);
        }

        bool setTexture(std::string key) {
            image = nullptr;

            if (!gameProps->assets->has(key)) {
                fatal_error("Error: Asset \"", key, "\" was not found.");
                return false;
            }

            image = gameProps->assets->get(key)->as<ImageAsset*>();
            
            if (image == nullptr) {
                fatal_error("Error: Asset \"", key, "\" is not a valid texture asset.");
                return false;
            }

            return true;
        }

        void update_size() {
            left = pos.x - widthInPixels*scale.x*origin.x;
            right = pos.x + widthInPixels*scale.x*origin.x;
            top = pos.y - heightInPixels*scale.y*origin.y;
            bottom = pos.y + heightInPixels*scale.y*origin.y;
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
            fatal_error("Error: Asset \"", key, "\" was not found.");
            return false;
        }

        void createObjects(sol::protected_function func) {
            if (!tmxAsset) {
                fatal_error("Error: No tilemap set.");
                return;
            }
            if (!func.valid()) {
                fatal_error("Error: Tilemap.createObjects requires a function to be passed as an argument.");
                return;
            }
            if (tmxAsset) {
                const unsigned int FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
                const unsigned int FLIPPED_VERTICALLY_FLAG   = 0x40000000;
                const unsigned int FLIPPED_DIAGONALLY_FLAG   = 0x20000000;
                const unsigned int ALL_FLIP_FLAGS = FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG | FLIPPED_DIAGONALLY_FLAG;
                
                for (int i = 0; i < tmxAsset->objectGroups.size(); ++i) {
                    const Amara::TMXObjectGroup& objectGroup = tmxAsset->objectGroups[i];
                    for (int j = 0; j < objectGroup.objects.size(); ++j) {
                        const Amara::TMXObject& object = objectGroup.objects[j];
                        nlohmann::json config = nlohmann::json::object();

                        unsigned int gid = object.gid;

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

                        config["id"] = localTileId;

                        config["layer"] = objectGroup.name;
                        
                        config["x"] = object.x;
                        config["y"] = object.y;

                        config["tileX"] = floor(object.x / tmxAsset->tileWidth);
                        config["tileY"] = floor((object.y - tmxAsset->tileHeight) / tmxAsset->tileHeight);

                        config["width"] = object.width;
                        config["height"] = object.height;
                        config["rotation"] = object.rotation;
                        config["visible"] = object.visible;

                        config["flippedHorizontally"] = flipped_horizontally;
                        config["flippedVertically"] = flipped_vertically;
                        config["flippedDiagonally"] = flipped_diagonally;
                        
                        config["name"] = object.name;
                        config["type"] = object.type;

                        for (auto it = object.properties.begin(); it != object.properties.end(); ++it) {
                            TMXProperty prop = it->second;
                            config[prop.name] = nlohmann::json::parse(prop.value);
                        }
                        
                        try {
                            sol::protected_function_result result = func(json_to_lua(gameProps->lua, config));
                            if (!result.valid()) {
                                sol::error err = result;
                                throw std::runtime_error(std::string(err.what()));
                            }
                        } 
                        catch (const sol::error& e) {
                            fatal_error(e.what());
                            gameProps->breakWorld();
                        }
                        catch (const std::exception& e) {
                            fatal_error(e.what());
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

                widthInPixels = mapWidth * tileWidth;
                heightInPixels = mapHeight * tileHeight;

                fillMap.clear();
                fillMap.resize(mapWidth * mapHeight, 0);
                
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
                    layerNode->widthInPixels = layer.width * tmxAsset->tileWidth;
                    layerNode->heightInPixels = layer.height * tmxAsset->tileHeight;
                    layerNode->partitionWidth = partitionWidth;
                    layerNode->partitionHeight = partitionHeight;
                    layerNode->origin = origin;
                    
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

                            if (tile.tileID >= 0) {
                                fillMap[ty * mapWidth + tx] = 1;
                            }
                            
                            layerNode->tiles.push_back(tile);
                        }
                    }
                }

                if (tmxAsset->objectGroups.size() > 0) {
                    objects = gameProps->lua.create_table();
                }
                else {
                    objects = sol::object(gameProps->lua, sol::nil);
                }

                for (int i = 0; i < tmxAsset->objectGroups.size(); ++i) {
                    const Amara::TMXObjectGroup& objectGroup = tmxAsset->objectGroups[i];
                    
                    sol::table objectLayer = gameProps->lua.create_table();
                    objects.as<sol::table>()[objectGroup.name] = objectLayer;
                    
                    for (int j = 0; j < objectGroup.objects.size(); ++j) {
                        const Amara::TMXObject& object = objectGroup.objects[j];
                        nlohmann::json config = nlohmann::json::object();

                        unsigned int gid = object.gid;

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

                        config["id"] = localTileId;

                        config["layer"] = objectGroup.name;
                        
                        config["x"] = object.x;
                        config["y"] = object.y;

                        config["tileX"] = floor(object.x / tmxAsset->tileWidth);
                        config["tileY"] = floor((object.y - tmxAsset->tileHeight) / tmxAsset->tileHeight);

                        config["width"] = object.width;
                        config["height"] = object.height;
                        config["rotation"] = object.rotation;
                        config["visible"] = object.visible;

                        config["flippedHorizontally"] = flipped_horizontally;
                        config["flippedVertically"] = flipped_vertically;
                        config["flippedDiagonally"] = flipped_diagonally;
                        
                        config["name"] = object.name;
                        config["type"] = object.type;

                        for (auto it = object.properties.begin(); it != object.properties.end(); ++it) {
                            TMXProperty prop = it->second;
                            config[prop.name] = nlohmann::json::parse(prop.value);
                        }

                        objectLayer[objectLayer.size() + 1] = json_to_lua(gameProps->lua, config);
                    }
                }
            }

            update_size();
        }

        virtual void update(double deltaTime) override {
            Amara::Group::update(deltaTime);
            update_size();
        }

        Quad getFillTileQuad(int gx, int gy) {
            int tileID = fillMap[gy*mapWidth + gx];
            if (tileID == 0) return Quad();
            
            Rectangle rect = { 
                pos.x + gx * tileWidth - widthInPixels*scale.x*origin.x,
                pos.y + gy * tileHeight - heightInPixels*scale.x*origin.y,
                (float)tileWidth*scale.x,
                (float)tileHeight*scale.y
            };
            return rotateQuad(Quad(rect), pos, rotation);
        }

        Quad getPartitionQuad(int j, int k) {
            Rectangle rect = { 
                pos.x + j * partitionWidth*tileWidth - widthInPixels*scale.x*origin.x,
                pos.y + k * partitionHeight*tileHeight - heightInPixels*scale.y*origin.y,
                (float)partitionWidth*tileWidth*scale.x,
                (float)partitionHeight*tileHeight*scale.y
            };
            return rotateQuad(Quad(rect), pos, rotation);
        }

        virtual bool collidesWithShape(const Shape& other) override {
            if (partitionWidth <= 0 || partitionHeight <= 0) return false;

            int numPartitionsX = ceil((float)mapWidth / partitionWidth);
            int numPartitionsY = ceil((float)mapHeight / partitionHeight);

            for (int j = 0; j < numPartitionsX; ++j) {
                for (int k = 0; k < numPartitionsY; ++k) {
                    Quad partition = getPartitionQuad(j, k);
                    if (other.collidesWith(partition)) {
                        int startX = j * partitionWidth;
                        int startY = k * partitionHeight;
                        int endX = startX + partitionWidth;
                        int endY = startY + partitionHeight;

                        if (endX > mapWidth) endX = mapWidth;
                        if (endY > mapHeight) endY = mapHeight;

                        for (int x = startX; x < endX; ++x) {
                            for (int y = startY; y < endY; ++y) {
                                int tileID = fillMap[y*mapWidth + x];
                                if (tileID == 0) continue;
                                
                                Quad tile = getFillTileQuad(x, y);
                                if (other.collidesWith(tile)) {
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
            return false;
        }

        virtual Shape getCollisionShape() override {
            return CustomShape(this);
        }

        Rectangle getRectangle() {
            return Rectangle(
                pos.x - widthInPixels*scale.x*origin.x,
                pos.y - heightInPixels*scale.y*origin.y,
                widthInPixels*scale.x, 
                heightInPixels*scale.y
            );
        }

        Vector2 getCenter() {
            return getRectangle().getCenter();
        }

        static void bind_lua(sol::state& lua) {
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
                "createObjects", sol::resolve<void(sol::protected_function)>(&Tilemap::createObjects),
                "objects", &Tilemap::objects,
                "origin", sol::property([](Amara::Tilemap& t) -> Vector2& { return t.origin; }, [](Amara::Tilemap& t, sol::object v) { t.origin = v; }),
                "originX", sol::property([](Amara::Tilemap& t) -> float { return t.origin.x; }, [](Amara::Tilemap& t, float v) { t.origin.x = v; }),
                "originY", sol::property([](Amara::Tilemap& t) -> float { return t.origin.y; }, [](Amara::Tilemap& t, float v) { t.origin.y = v; }),
                "width", sol::readonly(&Tilemap::mapWidth),
                "height", sol::readonly(&Tilemap::mapHeight),
                "tileWidth", sol::readonly(&Tilemap::tileWidth),
                "tileHeight", sol::readonly(&Tilemap::tileHeight),
                "widthInPixels", sol::readonly(&Tilemap::widthInPixels),
                "heightInPixels", sol::readonly(&Tilemap::heightInPixels),
                "left", sol::readonly(&Tilemap::left),
                "right", sol::readonly(&Tilemap::right),
                "top", sol::readonly(&Tilemap::top),
                "bottom", sol::readonly(&Tilemap::bottom),
                "partitionWidth", sol::property([](Amara::Tilemap& t) -> int { return t.partitionWidth; }, [](Amara::Tilemap& t, double value) { t.partitionWidth = floor(value); } ),
                "partitionHeight", sol::property([](Amara::Tilemap& t) -> int { return t.partitionHeight; }, [](Amara::Tilemap& t, double value) { t.partitionHeight = floor(value); } ),
                "rect", sol::property(&Tilemap::getRectangle),
                "center", sol::property(&Tilemap::getCenter)
            );
        }
    };
}