namespace Amara {
    class GameProps;
    class Asset;

    struct TMXProperty {
        std::string name;
        std::string type = "string";
        std::string value;
    };
    using TMXProperties = std::map<std::string, TMXProperty>;

    enum class TMXObjectType {
        Rectangle, Ellipse, Point, Polygon, Polyline, Text
    };

    struct TMXObject {
        unsigned int id = 0;
        std::string name;
        std::string type;
        float x = 0.0f, y = 0.0f, width = 0.0f, height = 0.0f, rotation = 0.0f;
        unsigned int gid = 0;
        bool visible = true;
        TMXObjectType objectType = TMXObjectType::Rectangle;
        std::vector<std::pair<float, float>> points;
        std::string text_content, text_fontfamily, text_color, text_halign, text_valign;
        int text_pixelsize = 16;
        bool text_wrap = false, text_bold = false, text_italic = false, text_underline = false, text_strikeout = false, text_kerning = true;
        TMXProperties properties;
    };

    struct TMXObjectGroup {
        std::string name;
        std::string color;
        float opacity = 1.0f;
        bool visible = true;
        float offsetX = 0.0f, offsetY = 0.0f;
        std::string drawOrder = "topdown";
        std::vector<TMXObject> objects;
        TMXProperties properties;
    };

    struct TMXImage {
        std::string source;
        std::string transparentColor;
        int width = 0, height = 0;
    };

    struct TMXImageLayer {
        std::string name;
        float offsetX = 0.0f, offsetY = 0.0f;
        float opacity = 1.0f;
        bool visible = true;
        TMXImage image;
        TMXProperties properties;
        std::string mapDirectory;
    };

    struct TMXAnimationFrame {
        unsigned int tileId;
        unsigned int duration; // In milliseconds
    };

    struct TMXAnimation {
        std::vector<TMXAnimationFrame> frames;
        int totalDuration = 0; // In milliseconds
        double progress = 0; // In milliseconds
        unsigned int currentTileId = 0;
        int currentIndex = 0;
    };

    struct TMXTileData {
        unsigned int localId = 0;
        TMXProperties properties;
        std::optional<TMXAnimation> animation;
    };

    struct TMXTileset {
        unsigned int firstGid = 0;
        std::string name;
        unsigned int tileWidth = 0;
        unsigned int tileHeight = 0;
        unsigned int tileCount = 0;
        unsigned int columns = 0;
        unsigned int spacing = 0;
        unsigned int margin = 0;
        std::string imageSource;
        int imageWidth = 0;
        int imageHeight = 0;
        std::string transparentColor;

        std::map<unsigned int, TMXTileData> tileData;
    };

    struct TMXTileLayer {
        std::string name;
        unsigned int width = 0;
        unsigned int height = 0;
        float opacity = 1.0f;
        bool visible = true;
        std::vector<unsigned int> data;
        TMXProperties properties;
    };

    class TMXTilemapAsset : public Amara::Asset {
    public:
        unsigned int width = 0;
        unsigned int height = 0;
        unsigned int tileWidth = 0;
        unsigned int tileHeight = 0;

        std::string orientation;
        std::string renderOrder;

        std::vector<TMXTileset> tilesets;
        std::vector<TMXTileLayer> layers;
        std::vector<TMXObjectGroup> objectGroups;
        std::vector<TMXImageLayer> imageLayers;
        TMXProperties mapProperties;

        TMXTilemapAsset(Amara::GameProps* _gameProps): Amara::Asset(_gameProps) {
            type = AssetEnum::TMXTilemap;
            typeKey = "TMXTilemapAsset";
        }

        bool loadTmx(const std::string& tmxPath);

        const TMXTileset* findTilesetForGid(unsigned int gid) const {
            if (gid == 0) return nullptr;
            for (auto it = tilesets.rbegin(); it != tilesets.rend(); ++it) {
                if (gid >= it->firstGid) {
                    return &(*it);
                }
            }
            return nullptr;
        }

        const TMXTileData* getTileData(unsigned int gid) const {
            const TMXTileset* tileset = findTilesetForGid(gid);
            if (!tileset) {
                return nullptr;
            }
            unsigned int localId = gid - tileset->firstGid;
            auto it = tileset->tileData.find(localId);
            if (it != tileset->tileData.end()) {
                return &(it->second);
            }
            return nullptr;
        }


        static TMXProperties parseProperties(tinyxml2::XMLElement* parentElement);
        static std::vector<std::pair<float, float>> parsePointsString(const std::string& pointsStr);
    };


    TMXProperties TMXTilemapAsset::parseProperties(tinyxml2::XMLElement* parentElement) {
        TMXProperties props;
        if (!parentElement) return props;

        tinyxml2::XMLElement* propertiesElement = parentElement->FirstChildElement("properties");
        if (!propertiesElement) return props;

        for (tinyxml2::XMLElement* propElement = propertiesElement->FirstChildElement("property"); propElement != nullptr; propElement = propElement->NextSiblingElement("property")) {
            const char* name = propElement->Attribute("name");
            const char* type = propElement->Attribute("type");
            const char* valueAttr = propElement->Attribute("value");

            if (name) {
                TMXProperty prop;
                prop.name = name;
                prop.type = type ? type : "string";

                if (valueAttr) {
                    prop.value = valueAttr;
                } else {
                    const char* textValue = propElement->GetText();
                    prop.value = textValue ? textValue : "";
                }
                props[prop.name] = prop;
            } else {
                debug_log("Warning: Property element missing 'name' attribute. Skipping.");
            }
        }
        return props;
    }

    std::vector<std::pair<float, float>> TMXTilemapAsset::parsePointsString(const std::string& pointsStr) {
        std::vector<std::pair<float, float>> points;
        std::stringstream ss(pointsStr);
        std::string pairStr;

        while (std::getline(ss, pairStr, ' ')) {
            std::stringstream ssPair(pairStr);
            std::string coordStr;
            float x = 0.0f, y = 0.0f;
            int coordIndex = 0;

            while (std::getline(ssPair, coordStr, ',')) {
                try {
                    if (coordIndex == 0) x = std::stof(coordStr);
                    else if (coordIndex == 1) y = std::stof(coordStr);
                } catch (const std::invalid_argument& e) {
                    debug_log("Warning: Invalid coordinate '", coordStr, "' in points string '", pointsStr, "'. Using 0.");
                } catch (const std::out_of_range& e) {
                     debug_log("Warning: Coordinate '", coordStr, "' out of range in points string '", pointsStr, "'. Using 0.");
                }
                coordIndex++;
            }
            if (coordIndex >= 2) points.push_back({x, y});
            else if (!pairStr.empty()) debug_log("Warning: Incomplete coordinate pair '", pairStr, "' in points string '", pointsStr, "'. Skipping.");
        }
        return points;
    }

    bool TMXTilemapAsset::loadTmx(const std::string& tmxPath) {
        path = gameProps->system->getAssetPath(tmxPath);
        
        if (!gameProps->system->fileExists(path)) {
            debug_log("Error: Tilemap file not found at \"", path, "\".");
            gameProps->breakWorld();
            return false;
        }
        
        std::string mapDir = "";
        size_t lastSlash = path.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            mapDir = path.substr(0, lastSlash + 1);
        }


        SDL_IOStream* rw = SDL_IOFromFile(path.c_str(), "rb");
        if (!rw) {
            debug_log("Error: Failed to open TMX file: ", path, " - ", SDL_GetError());
            return false;
        }

        Sint64 fileSize_s64 = SDL_GetIOSize(rw);
        if (fileSize_s64 <= 0) {
             SDL_CloseIO(rw);
             debug_log("Error: Invalid or empty TMX file: ", path);
             return false;
        }

        size_t fileSize = static_cast<size_t>(fileSize_s64);
        std::unique_ptr<unsigned char[]> buffer(new (std::nothrow) unsigned char[fileSize + 1]);
        if (!buffer) {
            SDL_CloseIO(rw);
            debug_log("Error: Failed to allocate memory for TMX file buffer: ", path);
            return false;
        }

        size_t bytesRead = SDL_ReadIO(rw, buffer.get(), fileSize);
        SDL_CloseIO(rw);

        if (bytesRead != fileSize) {
            debug_log("Error: Failed to read entire TMX file: ", path);
            return false;
        }
        buffer[fileSize] = '\0';

        unsigned char* raw_buffer_ptr = buffer.get();
        if (Amara::Encryption::is_buffer_encrypted(raw_buffer_ptr, fileSize)) {
            #if defined(AMARA_ENCRYPTION_KEY)
                Amara::Encryption::decryptBuffer(raw_buffer_ptr, fileSize, AMARA_ENCRYPTION_KEY);
                raw_buffer_ptr[fileSize] = '\0';
            #else
                debug_log("Error: Attempted to load encrypted TMX data without encryption key: \"", path, "\".");
                gameProps->breakWorld();
                return false;
            #endif
        }

        tinyxml2::XMLDocument doc;
        tinyxml2::XMLError parseResult = doc.Parse(reinterpret_cast<const char*>(raw_buffer_ptr), fileSize);

        if (parseResult != tinyxml2::XML_SUCCESS) {
            debug_log("Error: Failed to parse TMX file XML: ", path, " - Error Code: ", parseResult, " - ", doc.ErrorStr());
            return false;
        }

        tinyxml2::XMLElement* mapRoot = doc.RootElement();
        if (!mapRoot || !String::equal(mapRoot->Name(), "map")) {
             debug_log("Error: Invalid TMX file format. Missing <map> root element in: ", path);
             return false;
        }

        mapRoot->QueryUnsignedAttribute("width", &width);
        mapRoot->QueryUnsignedAttribute("height", &height);
        mapRoot->QueryUnsignedAttribute("tilewidth", &tileWidth);
        mapRoot->QueryUnsignedAttribute("tileheight", &tileHeight);
        orientation = mapRoot->Attribute("orientation") ? mapRoot->Attribute("orientation") : "orthogonal";
        renderOrder = mapRoot->Attribute("renderorder") ? mapRoot->Attribute("renderorder") : "right-down";
        mapProperties = parseProperties(mapRoot);

        tilesets.clear();
        for (tinyxml2::XMLElement* tsElement = mapRoot->FirstChildElement("tileset"); tsElement != nullptr; tsElement = tsElement->NextSiblingElement("tileset")) {
            TMXTileset currentTileset;
            tsElement->QueryUnsignedAttribute("firstgid", &currentTileset.firstGid);
            if (currentTileset.firstGid == 0) {
                debug_log("Warning: Tileset found with firstgid=0 in map '", path, "'. Skipping this tileset entry.");
                continue;
            }

            const char* source = tsElement->Attribute("source");
            if (source) {
                debug_log("Error: External tilesets (TSX system) are not supported.");
                debug_log("       Found reference to '", source, "' in map '", path, "'.");
                debug_log("       Please embed the tileset directly into the TMX file.");
                debug_log("       ( In Tiled: Edit -> Preferences -> Plugins -> Tiled -> Embed tilesets, then File -> Export As... -> Select .tmx )");
                tilesets.clear(); layers.clear(); objectGroups.clear(); imageLayers.clear(); mapProperties.clear();
                width = height = tileWidth = tileHeight = 0;
                return false;
            }
            else {
                const char* tsName = tsElement->Attribute("name");
                if (tsName) currentTileset.name = tsName;

                tsElement->QueryUnsignedAttribute("tilewidth", &currentTileset.tileWidth);
                tsElement->QueryUnsignedAttribute("tileheight", &currentTileset.tileHeight);
                tsElement->QueryUnsignedAttribute("tilecount", &currentTileset.tileCount);
                tsElement->QueryUnsignedAttribute("columns", &currentTileset.columns);
                tsElement->QueryUnsignedAttribute("spacing", &currentTileset.spacing);
                tsElement->QueryUnsignedAttribute("margin", &currentTileset.margin);

                tinyxml2::XMLElement* imgElement = tsElement->FirstChildElement("image");
                if (imgElement) {
                     const char* imgSrc = imgElement->Attribute("source");
                     if (imgSrc) currentTileset.imageSource = imgSrc;
                     imgElement->QueryIntAttribute("width", &currentTileset.imageWidth);
                     imgElement->QueryIntAttribute("height", &currentTileset.imageHeight);
                     const char* trans = imgElement->Attribute("trans");
                     if (trans) currentTileset.transparentColor = trans;
                } else {
                    debug_log("Warning: Embedded tileset '", currentTileset.name, "' is missing <image> element in map '", path, "'.");
                }

                for (tinyxml2::XMLElement* tileElement = tsElement->FirstChildElement("tile"); tileElement != nullptr; tileElement = tileElement->NextSiblingElement("tile")) {
                    unsigned int localId = 0;
                    if (tileElement->QueryUnsignedAttribute("id", &localId) == tinyxml2::XML_SUCCESS) {
                        TMXTileData tileData;
                        tileData.localId = localId;
                        tileData.properties = parseProperties(tileElement);

                        tinyxml2::XMLElement* animElement = tileElement->FirstChildElement("animation");
                        if (animElement) {
                            TMXAnimation animation;
                            for (tinyxml2::XMLElement* frameElement = animElement->FirstChildElement("frame"); frameElement != nullptr; frameElement = frameElement->NextSiblingElement("frame")) {
                                TMXAnimationFrame frame;
                                if (frameElement->QueryUnsignedAttribute("tileid", &frame.tileId) != tinyxml2::XML_SUCCESS) {
                                     debug_log("Warning: Animation frame missing 'tileid' in tileset '", currentTileset.name, "', tile '", localId, "' in map '", path, "'. Skipping frame.");
                                     continue;
                                }
                                if (frameElement->QueryUnsignedAttribute("duration", &frame.duration) != tinyxml2::XML_SUCCESS) {
                                     debug_log("Warning: Animation frame missing 'duration' in tileset '", currentTileset.name, "', tile '", localId, "' in map '", path, "'. Skipping frame.");
                                     continue;
                                }
                                animation.frames.push_back(frame);
                                animation.totalDuration += frame.duration;
                            }
                            if (!animation.frames.empty()) {
                                tileData.animation = animation;
                            }
                        }
                        currentTileset.tileData[localId] = tileData;
                    } else {
                         debug_log("Warning: Tile element missing 'id' attribute within tileset '", currentTileset.name, "' in map '", path, "'. Skipping tile data.");
                    }
                }
                tilesets.push_back(currentTileset);
            }
        }

        std::sort(tilesets.begin(), tilesets.end(), [](const TMXTileset& a, const TMXTileset& b) {
            return a.firstGid < b.firstGid;
        });

        layers.clear();
        objectGroups.clear();
        imageLayers.clear();

        for (tinyxml2::XMLElement* element = mapRoot->FirstChildElement(); element != nullptr; element = element->NextSiblingElement()) {
            const char* elementName = element->Name();
            if (!elementName) continue;

            if (String::equal(elementName, "layer")) {
                TMXTileLayer currentLayer;
                currentLayer.name = element->Attribute("name") ? element->Attribute("name") : "";
                currentLayer.width = width;
                currentLayer.height = height;
                element->QueryFloatAttribute("opacity", &currentLayer.opacity);
                element->QueryBoolAttribute("visible", &currentLayer.visible);
                currentLayer.properties = parseProperties(element);

                tinyxml2::XMLElement* dataElement = element->FirstChildElement("data");
                if (!dataElement) {
                    debug_log("Warning: Tile layer '", currentLayer.name, "' in map '", path, "' is missing <data> element. Skipping layer.");
                    continue;
                }

                const char* encoding = dataElement->Attribute("encoding");
                const char* compression = dataElement->Attribute("compression");
                const char* dataText = dataElement->GetText();
                std::string dataString = dataText ? dataText : "";

                if (encoding) {
                    dataString.erase(std::remove_if(dataString.begin(), dataString.end(), ::isspace), dataString.end());
                }

                if (dataString.empty() && !encoding) {
                } else if (dataString.empty() && encoding) {
                     debug_log("Warning: Tile layer '", currentLayer.name, "' in map '", path, "' has empty <data> content after whitespace removal. Skipping layer.");
                     continue;
                }

                currentLayer.data.reserve(static_cast<size_t>(width) * height);

                if (!encoding) {
                    unsigned int tileCount = 0;
                    for (tinyxml2::XMLElement* tileElement = dataElement->FirstChildElement("tile"); tileElement != nullptr; tileElement = tileElement->NextSiblingElement("tile")) {
                        unsigned int gid = 0;
                        tileElement->QueryUnsignedAttribute("gid", &gid);
                        currentLayer.data.push_back(gid);
                        tileCount++;
                    }
                    if (tileCount != static_cast<size_t>(width) * height) {
                        debug_log("Warning: XML tile layer '", currentLayer.name, "' in map '", path, "' has ", tileCount, " tiles, expected ", static_cast<size_t>(width) * height, ".");
                    }
                }
                else if (String::equal(encoding, "csv")) {
                    std::stringstream ss(dataString);
                    std::string value;
                    unsigned int tileCount = 0;
                    while (std::getline(ss, value, ',')) {
                        try {
                            currentLayer.data.push_back(std::stoul(value));
                        } catch (...) {
                            debug_log("Warning: Invalid/Out-of-range GID '", value, "' in CSV data for layer '", currentLayer.name, "'. Using 0.");
                            currentLayer.data.push_back(0);
                        }
                        tileCount++;
                    }
                     if (tileCount != static_cast<size_t>(width) * height) {
                         debug_log("Warning: CSV tile layer '", currentLayer.name, "' in map '", path, "' has ", tileCount, " tiles, expected ", static_cast<size_t>(width) * height, ".");
                    }
                }
                else if (String::equal(encoding, "base64")) {
                    std::vector<unsigned char> decoded_data = base64_decode(dataString);
                    if (decoded_data.empty() && !dataString.empty()) {
                        debug_log("Error: Base64 decoding failed for layer '", currentLayer.name, "' in map '", path, "'. Skipping layer.");
                        continue;
                    }

                    std::vector<unsigned char> decompressed_data = decoded_data;
                    if (compression) {
                        decompressed_data = decompress_data(decoded_data, std::string(compression));
                        if (decompressed_data.empty() && !decoded_data.empty()) {
                            debug_log("Error: Decompression ('", compression, "') failed for layer '", currentLayer.name, "' in map '", path, "'. Skipping layer.");
                            continue;
                        }
                    }

                    size_t expected_bytes = static_cast<size_t>(width) * height * 4;
                    if (decompressed_data.size() != expected_bytes) {
                        debug_log("Warning: Decompressed data size mismatch for layer '", currentLayer.name, "' in map '", path, "'. Expected ", expected_bytes, " bytes, got ", decompressed_data.size(), ".");
                    }

                    unsigned int tileCount = 0;
                    for (size_t i = 0; (i + 3) < decompressed_data.size() && tileCount < (static_cast<size_t>(width) * height); i += 4) {
                        unsigned int gid = static_cast<unsigned int>(decompressed_data[i]) |
                                           (static_cast<unsigned int>(decompressed_data[i + 1]) << 8) |
                                           (static_cast<unsigned int>(decompressed_data[i + 2]) << 16) |
                                           (static_cast<unsigned int>(decompressed_data[i + 3]) << 24);
                        currentLayer.data.push_back(gid);
                        tileCount++;
                    }
                     if (tileCount != static_cast<size_t>(width) * height) {
                         debug_log("Warning: Base64 tile layer '", currentLayer.name, "' in map '", path, "' processed ", tileCount, " tiles, expected ", static_cast<size_t>(width) * height, ".");
                    }
                }
                else {
                    debug_log("Error: Unsupported layer data encoding '", encoding, "' for layer '", currentLayer.name, "' in map '", path, "'. Skipping layer.");
                    continue;
                }
                layers.push_back(currentLayer);
            }
            else if (String::equal(elementName, "objectgroup")) {
                TMXObjectGroup currentGroup;
                currentGroup.name = element->Attribute("name") ? element->Attribute("name") : "";
                currentGroup.color = element->Attribute("color") ? element->Attribute("color") : "";
                element->QueryFloatAttribute("opacity", &currentGroup.opacity);
                element->QueryBoolAttribute("visible", &currentGroup.visible);
                element->QueryFloatAttribute("offsetx", &currentGroup.offsetX);
                element->QueryFloatAttribute("offsety", &currentGroup.offsetY);
                currentGroup.drawOrder = element->Attribute("draworder") ? element->Attribute("draworder") : "topdown";
                currentGroup.properties = parseProperties(element);

                for (tinyxml2::XMLElement* objElement = element->FirstChildElement("object"); objElement != nullptr; objElement = objElement->NextSiblingElement("object")) {
                    TMXObject currentObject;
                    objElement->QueryUnsignedAttribute("id", &currentObject.id);
                    currentObject.name = objElement->Attribute("name") ? objElement->Attribute("name") : "";
                    currentObject.type = objElement->Attribute("type") ? objElement->Attribute("type") : "";
                    objElement->QueryFloatAttribute("x", &currentObject.x);
                    objElement->QueryFloatAttribute("y", &currentObject.y);
                    objElement->QueryFloatAttribute("width", &currentObject.width);
                    objElement->QueryFloatAttribute("height", &currentObject.height);
                    objElement->QueryFloatAttribute("rotation", &currentObject.rotation);
                    objElement->QueryUnsignedAttribute("gid", &currentObject.gid);
                    objElement->QueryBoolAttribute("visible", &currentObject.visible);
                    currentObject.properties = parseProperties(objElement);

                    tinyxml2::XMLElement* shapeElement = objElement->FirstChildElement();
                    if (shapeElement) {
                        const char* shapeName = shapeElement->Name();
                        if (String::equal(shapeName, "ellipse")) currentObject.objectType = TMXObjectType::Ellipse;
                        else if (String::equal(shapeName, "point")) currentObject.objectType = TMXObjectType::Point;
                        else if (String::equal(shapeName, "polygon")) {
                            currentObject.objectType = TMXObjectType::Polygon;
                            const char* points = shapeElement->Attribute("points");
                            if (points) currentObject.points = parsePointsString(points);
                        } else if (String::equal(shapeName, "polyline")) {
                            currentObject.objectType = TMXObjectType::Polyline;
                            const char* points = shapeElement->Attribute("points");
                            if (points) currentObject.points = parsePointsString(points);
                        } else if (String::equal(shapeName, "text")) {
                            currentObject.objectType = TMXObjectType::Text;
                            currentObject.text_fontfamily = shapeElement->Attribute("fontfamily") ? shapeElement->Attribute("fontfamily") : "sans-serif";
                            shapeElement->QueryIntAttribute("pixelsize", &currentObject.text_pixelsize);
                            shapeElement->QueryBoolAttribute("wrap", &currentObject.text_wrap);
                            currentObject.text_color = shapeElement->Attribute("color") ? shapeElement->Attribute("color") : "#000000";
                            currentObject.text_halign = shapeElement->Attribute("halign") ? shapeElement->Attribute("halign") : "left";
                            currentObject.text_valign = shapeElement->Attribute("valign") ? shapeElement->Attribute("valign") : "top";
                            shapeElement->QueryBoolAttribute("bold", &currentObject.text_bold);
                            shapeElement->QueryBoolAttribute("italic", &currentObject.text_italic);
                            shapeElement->QueryBoolAttribute("underline", &currentObject.text_underline);
                            shapeElement->QueryBoolAttribute("strikeout", &currentObject.text_strikeout);
                            shapeElement->QueryBoolAttribute("kerning", &currentObject.text_kerning);
                            const char* text = shapeElement->GetText();
                            if (text) currentObject.text_content = text;
                        }
                    } else {
                        currentObject.objectType = TMXObjectType::Rectangle;
                    }
                    currentGroup.objects.push_back(currentObject);
                }
                objectGroups.push_back(currentGroup);
            }
            else if (String::equal(elementName, "imagelayer")) {
                TMXImageLayer currentImageLayer;
                currentImageLayer.mapDirectory = mapDir;
                currentImageLayer.name = element->Attribute("name") ? element->Attribute("name") : "";
                element->QueryFloatAttribute("offsetx", &currentImageLayer.offsetX);
                element->QueryFloatAttribute("offsety", &currentImageLayer.offsetY);
                element->QueryFloatAttribute("opacity", &currentImageLayer.opacity);
                element->QueryBoolAttribute("visible", &currentImageLayer.visible);
                currentImageLayer.properties = parseProperties(element);

                tinyxml2::XMLElement* imgElement = element->FirstChildElement("image");
                if (imgElement) {
                    currentImageLayer.image.source = imgElement->Attribute("source") ? imgElement->Attribute("source") : "";
                    currentImageLayer.image.transparentColor = imgElement->Attribute("trans") ? imgElement->Attribute("trans") : "";
                    imgElement->QueryIntAttribute("width", &currentImageLayer.image.width);
                    imgElement->QueryIntAttribute("height", &currentImageLayer.image.height);
                } else {
                    debug_log("Warning: Image layer '", currentImageLayer.name, "' in map '", path, "' is missing the <image> element. Skipping image info.");
                }
                imageLayers.push_back(currentImageLayer);
            }
        }

        if (tilesets.empty()) {
            debug_log("Warning: No embedded tilesets were successfully loaded for map: ", path);
        }

        return true;
    }

}