namespace Amara {
    struct Tiled_Property {
        std::string name;
        std::string type = "string"; // Tiled type: string, int, float, bool, color, file, object
        std::string value;

        // TODO: std::variant<std::string, int, float, bool, Color, std::string, int> parsedValue;
    };
    using Tiled_Properties = std::map<std::string, Tiled_Property>;

    enum class Tiled_ObjectType {
        Rectangle, // Default if no shape element
        Ellipse,
        Point,
        Polygon,
        Polyline,
        Text
    };

    struct Tiled_Object {
        unsigned int id = 0;

        std::string name;
        std::string type; // User-defined type string

        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;  // For Rectangle, Ellipse, Text
        float height = 0.0f; // For Rectangle, Ellipse, Text
        float rotation = 0.0f;
        unsigned int gid = 0; // Tile GID if object is represented by a tile
        bool visible = true;

        Tiled_ObjectType objectType = Tiled_ObjectType::Rectangle;

        // Shape data for Polygon and Polyline
        std::vector<std::pair<float, float>> points;

        std::string text_content;
        std::string text_fontfamily;
        int text_pixelsize;
        bool text_wrap;
        std::string text_color;
        std::string text_halign; // left, center, right, justify
        std::string text_valign; // top, center, bottom
        bool text_bold = false;
        bool text_italic = false;
        bool text_underline = false;
        bool text_strikeout = false;
        bool text_kerning = true;

        Tiled_Properties properties;
    };

    struct Tiled_ObjectGroup {
        std::string name;
        std::string color; // Stored as string (e.g., "#FF00FF")

        float opacity = 1.0f;
        bool visible = true;
        float offsetX = 0.0f;
        float offsetY = 0.0f;
        std::string drawOrder = "topdown"; // "topdown" or "index"
        std::vector<Tiled_Object> objects;
        Tiled_Properties properties;
    };

    struct Tiled_Image {
        std::string source;           // Relative path from TMX file
        std::string transparentColor; // Stored as string (e.g., "#FF00FF")
        int width = 0;                // Image width in pixels (from <image> tag)
        int height = 0;               // Image height in pixels (from <image> tag)
    };

    struct Tiled_ImageLayer {
        std::string name;
        float offsetX = 0.0f;
        float offsetY = 0.0f;
        float opacity = 1.0f;
        bool visible = true;
        Tiled_Image image;
        Tiled_Properties properties;
        std::string mapDirectory; // Store directory of the TMX for resolving relative paths
    };

    struct Tiled_Tileset {
        unsigned int firstGid = 0;
        std::string name;
        unsigned int tileWidth = 0;
        unsigned int tileHeight = 0;
        unsigned int tileCount = 0;
        unsigned int columns = 0;
        unsigned int spacing = 0;
        unsigned int margin = 0;
        // Consider adding Tiled_Properties here too if tilesets can have them
        // Tiled_Properties properties;
        // Consider adding image source if tileset image is embedded/referenced
        // Tiled_Image image;
    };

    struct Tiled_TileLayer {
        std::string name;
        unsigned int width = 0;
        unsigned int height = 0;
        float opacity = 1.0f;
        bool visible = true;
        std::vector<unsigned int> data; // Tile GIDs (Global IDs)
        Tiled_Properties properties; // Add properties to layers too
    };

    class Tiled_TilemapAsset : public Amara::Asset {
    public:
        unsigned int width = 0;         // Map width in tiles
        unsigned int height = 0;        // Map height in tiles
        unsigned int tileWidth = 0;     // Tile width in pixels
        unsigned int tileHeight = 0;    // Tile height in pixels

        std::string orientation;        // e.g., "orthogonal", "isometric"
        std::string renderOrder;        // e.g., "right-down"

        std::vector<Tiled_Tileset> tilesets;
        std::vector<Tiled_TileLayer> layers;
        std::vector<Tiled_ObjectGroup> objectGroups; // Added
        std::vector<Tiled_ImageLayer> imageLayers;   // Added
        Tiled_Properties mapProperties;              // Added for map-level properties

        Tiled_TilemapAsset() {
            type = AssetEnum::TiledTilemap;
            typeKey = "Tiled_TilemapAsset";
        }

        bool loadTmx(const std::string& tmxPath);

        const Tiled_Tileset* findTilesetForGid(unsigned int gid) const {
            for (auto it = tilesets.rbegin(); it != tilesets.rend(); ++it) {
                if (gid >= it->firstGid) {
                    return &(*it);
                }
            }
            return nullptr;
        }

        static Tiled_Properties parseProperties(tinyxml2::XMLElement* parentElement);
        static std::vector<std::pair<float, float>> parsePointsString(const std::string& pointsStr);
    };

    Tiled_Properties Tiled_TilemapAsset::parseProperties(tinyxml2::XMLElement* parentElement) {
        Tiled_Properties props;
        if (!parentElement) return props;

        tinyxml2::XMLElement* propertiesElement = parentElement->FirstChildElement("properties");
        if (!propertiesElement) return props;

        for (tinyxml2::XMLElement* propElement = propertiesElement->FirstChildElement("property"); propElement != nullptr; propElement = propElement->NextSiblingElement("property")) {
            const char* name = propElement->Attribute("name");
            const char* type = propElement->Attribute("type");
            const char* value = propElement->Attribute("value");

            if (name) {
                Tiled_Property prop;
                prop.name = name;
                if (type) prop.type = type; else prop.type = "string";

                if (value) {
                    prop.value = value;
                } else {
                    const char* textValue = propElement->GetText();
                    prop.value = textValue ? textValue : "";
                    if (!type) prop.type = "string";
                }
                props[prop.name] = prop;
            } else {
                debug_log("Warning: Property element missing 'name' attribute. Skipping.");
            }
        }
        return props;
    }

    std::vector<std::pair<float, float>> Tiled_TilemapAsset::parsePointsString(const std::string& pointsStr) {
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
                    if (coordIndex == 0) {
                        x = std::stof(coordStr);
                    } else if (coordIndex == 1) {
                        y = std::stof(coordStr);
                    }
                } catch (const std::invalid_argument& e) {
                    debug_log("Warning: Invalid coordinate '", coordStr, "' in points string '", pointsStr, "'. Using 0.");
                } catch (const std::out_of_range& e) {
                     debug_log("Warning: Coordinate '", coordStr, "' out of range in points string '", pointsStr, "'. Using 0.");
                }
                coordIndex++;
            }
            if (coordIndex >= 2) { // Ensure we have at least x and y
                 points.push_back({x, y});
            } else if (!pairStr.empty()) {
                 debug_log("Warning: Incomplete coordinate pair '", pairStr, "' in points string '", pointsStr, "'. Skipping.");
            }
        }
        return points;
    }

    bool Tiled_TilemapAsset::loadTmx(const std::string& tmxPath) {
        path = Props::system->getAssetPath(tmxPath);

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
        char* buffer = new (std::nothrow) char[fileSize + 1];
        if (!buffer) {
            SDL_CloseIO(rw);
            debug_log("Error: Failed to allocate memory for TMX file buffer: ", path);
            return false;
        }

        size_t bytesRead = SDL_ReadIO(rw, buffer, fileSize);
        SDL_CloseIO(rw);

        if (bytesRead != fileSize) {
            delete[] buffer;
            debug_log("Error: Failed to read entire TMX file: ", path);
            return false;
        }
        buffer[fileSize] = '\0';

        tinyxml2::XMLDocument doc;
        tinyxml2::XMLError parseResult = doc.Parse(buffer, fileSize);
        delete[] buffer;
        buffer = nullptr;

        if (parseResult != tinyxml2::XML_SUCCESS) {
            debug_log("Error: Failed to parse TMX file XML: ", path, " - Error Code: ", parseResult);
            return false;
        }

        tinyxml2::XMLElement* mapRoot = doc.RootElement();
        if (!mapRoot || !string_equal(mapRoot->Name(), "map")) {
             debug_log("Error: Invalid TMX file format. Missing <map> root element in: ", path);
             return false;
        }

        mapRoot->QueryUnsignedAttribute("width", &width);
        mapRoot->QueryUnsignedAttribute("height", &height);
        mapRoot->QueryUnsignedAttribute("tilewidth", &tileWidth);
        mapRoot->QueryUnsignedAttribute("tileheight", &tileHeight);

        const char* orient = mapRoot->Attribute("orientation");
        if (orient) orientation = orient; else orientation = "orthogonal";

        const char* render = mapRoot->Attribute("renderorder");
        if (render) renderOrder = render; else renderOrder = "right-down";

        mapProperties = parseProperties(mapRoot);

        tilesets.clear();
        for (tinyxml2::XMLElement* tsElement = mapRoot->FirstChildElement("tileset"); tsElement != nullptr; tsElement = tsElement->NextSiblingElement("tileset")) {
            Tiled_Tileset currentTileset;
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
                debug_log("       ( Edit -> Preferences -> Embed tilesets, then Export As .tmx )");
                tilesets.clear();
                layers.clear();
                objectGroups.clear();
                imageLayers.clear();
                mapProperties.clear();
                width = height = tileWidth = tileHeight = 0;

                return false;
            }

            const char* tsName = tsElement->Attribute("name");
            if (tsName) currentTileset.name = tsName;

            tsElement->QueryUnsignedAttribute("tilewidth", &currentTileset.tileWidth);
            tsElement->QueryUnsignedAttribute("tileheight", &currentTileset.tileHeight);
            tsElement->QueryUnsignedAttribute("tilecount", &currentTileset.tileCount);
            tsElement->QueryUnsignedAttribute("columns", &currentTileset.columns);
            tsElement->QueryUnsignedAttribute("spacing", &currentTileset.spacing);
            tsElement->QueryUnsignedAttribute("margin", &currentTileset.margin);

            // TODO: Parse <image> tag within embedded <tileset> if needed later
            // tinyxml2::XMLElement* imgElement = tsElement->FirstChildElement("image");
            // if (imgElement) { ... }

            // TODO: Parse <tile> specific properties/animations within <tileset> if needed later

            tilesets.push_back(currentTileset);
        }

        std::sort(tilesets.begin(), tilesets.end(), [](const Tiled_Tileset& a, const Tiled_Tileset& b) {
            return a.firstGid < b.firstGid;
        });

        layers.clear();
        objectGroups.clear();
        imageLayers.clear();

        for (tinyxml2::XMLElement* element = mapRoot->FirstChildElement(); element != nullptr; element = element->NextSiblingElement()) {
            const char* elementName = element->Name();
            if (!elementName) continue;

            if (string_equal(elementName, "layer")) {
                Tiled_TileLayer currentLayer;
                const char* layerName = element->Attribute("name");
                if (layerName) currentLayer.name = layerName;

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

                if (!dataText && !encoding) {
                    // Handle XML case where dataText might be null
                }
                else if (!dataText || std::string(dataText).empty()) {
                     debug_log("Warning: Tile layer '", currentLayer.name, "' in map '", path, "' has empty <data> content. Skipping layer.");
                     continue;
                }

                std::string dataString;
                if (dataText) {
                    dataString = dataText;
                    if (encoding) {
                         dataString.erase(std::remove_if(dataString.begin(), dataString.end(), ::isspace), dataString.end());
                    }
                }

                currentLayer.data.reserve(width * height);

                if (!encoding) {
                    // XML encoding (<tile gid="..."/> elements)
                    unsigned int tileCount = 0;
                    for (tinyxml2::XMLElement* tileElement = dataElement->FirstChildElement("tile"); tileElement != nullptr; tileElement = tileElement->NextSiblingElement("tile")) {
                        unsigned int gid = 0;
                        tileElement->QueryUnsignedAttribute("gid", &gid);
                        currentLayer.data.push_back(gid);
                        tileCount++;
                    }
                    if (tileCount != width * height) {
                        debug_log("Warning: XML tile layer '", currentLayer.name, "' in map '", path, "' has ", tileCount, " tiles, expected ", width * height, ".");
                    }
                }
                else if (string_equal(encoding, "csv")) {
                    std::stringstream ss(dataString);
                    std::string value;
                    unsigned int tileCount = 0;
                    while (std::getline(ss, value, ',')) {
                        try {
                            currentLayer.data.push_back(std::stoul(value));
                        } catch (const std::invalid_argument& e) {
                            debug_log("Warning: Invalid GID '", value, "' in CSV data for layer '", currentLayer.name, "'. Using 0.");
                            currentLayer.data.push_back(0);
                        } catch (const std::out_of_range& e) {
                            debug_log("Warning: GID '", value, "' out of range in CSV data for layer '", currentLayer.name, "'. Using 0.");
                            currentLayer.data.push_back(0);
                        }
                        tileCount++;
                    }
                    if (tileCount != width * height) {
                         debug_log("Warning: CSV tile layer '", currentLayer.name, "' in map '", path, "' has ", tileCount, " tiles, expected ", width * height, ".");
                    }
                }
                else if (string_equal(encoding, "base64")) {
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
                    for (size_t i = 0; (i + 3) < decompressed_data.size() && tileCount < (width * height); i += 4) {
                        unsigned int gid = static_cast<unsigned int>(decompressed_data[i]) |
                                           (static_cast<unsigned int>(decompressed_data[i + 1]) << 8) |
                                           (static_cast<unsigned int>(decompressed_data[i + 2]) << 16) |
                                           (static_cast<unsigned int>(decompressed_data[i + 3]) << 24);
                        currentLayer.data.push_back(gid);
                        tileCount++;
                    }
                     if (tileCount != width * height) {
                         debug_log("Warning: Base64 tile layer '", currentLayer.name, "' in map '", path, "' processed ", tileCount, " tiles, expected ", width * height, ".");
                    }
                }
                else {
                    debug_log("Error: Unsupported layer data encoding '", encoding, "' for layer '", currentLayer.name, "' in map '", path, "'. Skipping layer.");
                    continue;
                }

                layers.push_back(currentLayer);
            }
            else if (string_equal(elementName, "objectgroup")) {
                Tiled_ObjectGroup currentGroup;
                const char* groupName = element->Attribute("name");
                if (groupName) currentGroup.name = groupName;

                const char* colorStr = element->Attribute("color");
                if (colorStr) currentGroup.color = colorStr;

                element->QueryFloatAttribute("opacity", &currentGroup.opacity);
                element->QueryBoolAttribute("visible", &currentGroup.visible);
                element->QueryFloatAttribute("offsetx", &currentGroup.offsetX);
                element->QueryFloatAttribute("offsety", &currentGroup.offsetY); 

                const char* drawOrderStr = element->Attribute("draworder");
                if (drawOrderStr) currentGroup.drawOrder = drawOrderStr;
                else currentGroup.drawOrder = "topdown";

                currentGroup.properties = parseProperties(element);

                for (tinyxml2::XMLElement* objElement = element->FirstChildElement("object"); objElement != nullptr; objElement = objElement->NextSiblingElement("object")) {
                    Tiled_Object currentObject;
                    objElement->QueryUnsignedAttribute("id", &currentObject.id);
                    const char* objName = objElement->Attribute("name");
                    if (objName) currentObject.name = objName;
                    const char* objType = objElement->Attribute("type");
                    if (objType) currentObject.type = objType;

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
                        if (string_equal(shapeName, "ellipse")) {
                            currentObject.objectType = Tiled_ObjectType::Ellipse;
                        } else if (string_equal(shapeName, "point")) {
                            currentObject.objectType = Tiled_ObjectType::Point;
                        } else if (string_equal(shapeName, "polygon")) {
                            currentObject.objectType = Tiled_ObjectType::Polygon;
                            const char* points = shapeElement->Attribute("points");
                            if (points) currentObject.points = parsePointsString(points);
                        } else if (string_equal(shapeName, "polyline")) {
                            currentObject.objectType = Tiled_ObjectType::Polyline;
                            const char* points = shapeElement->Attribute("points");
                            if (points) currentObject.points = parsePointsString(points);
                        } else if (string_equal(shapeName, "text")) {
                            currentObject.objectType = Tiled_ObjectType::Text;
                            const char* fontFamily = shapeElement->Attribute("fontfamily");
                            if (fontFamily) currentObject.text_fontfamily = fontFamily;
                            shapeElement->QueryIntAttribute("pixelsize", &currentObject.text_pixelsize);
                            shapeElement->QueryBoolAttribute("wrap", &currentObject.text_wrap);
                            const char* color = shapeElement->Attribute("color");
                            if (color) currentObject.text_color = color;
                            const char* halign = shapeElement->Attribute("halign");
                            if (halign) currentObject.text_halign = halign; else currentObject.text_halign = "left";
                            const char* valign = shapeElement->Attribute("valign");
                            if (valign) currentObject.text_valign = valign; else currentObject.text_valign = "top";
                            shapeElement->QueryBoolAttribute("bold", &currentObject.text_bold);
                            shapeElement->QueryBoolAttribute("italic", &currentObject.text_italic);
                            shapeElement->QueryBoolAttribute("underline", &currentObject.text_underline);
                            shapeElement->QueryBoolAttribute("strikeout", &currentObject.text_strikeout);
                            shapeElement->QueryBoolAttribute("kerning", &currentObject.text_kerning);
                            const char* text = shapeElement->GetText();
                            if (text) currentObject.text_content = text;
                        }
                    }
                    else {
                        // No shape element: Default to Rectangle if no GID, otherwise it's a Tile object (which we treat as Rectangle visually)
                        currentObject.objectType = Tiled_ObjectType::Rectangle;
                    }

                    currentGroup.objects.push_back(currentObject);
                }
                objectGroups.push_back(currentGroup);
            }
            else if (string_equal(elementName, "imagelayer")) {
                Tiled_ImageLayer currentImageLayer;
                // Store the directory for resolving relative image paths later
                size_t lastSlash = path.find_last_of("/\\");
                currentImageLayer.mapDirectory = (lastSlash == std::string::npos) ? "" : path.substr(0, lastSlash + 1);

                const char* imgLayerName = element->Attribute("name");
                if (imgLayerName) currentImageLayer.name = imgLayerName;

                element->QueryFloatAttribute("offsetx", &currentImageLayer.offsetX); // Defaults to 0.0
                element->QueryFloatAttribute("offsety", &currentImageLayer.offsetY); // Defaults to 0.0
                element->QueryFloatAttribute("opacity", &currentImageLayer.opacity); // Defaults to 1.0
                element->QueryBoolAttribute("visible", &currentImageLayer.visible); // Defaults to true

                currentImageLayer.properties = parseProperties(element);

                tinyxml2::XMLElement* imgElement = element->FirstChildElement("image");
                if (imgElement) {
                    const char* imgSrc = imgElement->Attribute("source");
                    if (imgSrc) currentImageLayer.image.source = imgSrc;

                    const char* transColor = imgElement->Attribute("trans");
                    if (transColor) currentImageLayer.image.transparentColor = transColor;

                    imgElement->QueryIntAttribute("width", &currentImageLayer.image.width);
                    imgElement->QueryIntAttribute("height", &currentImageLayer.image.height);
                } else {
                    debug_log("Warning: Image layer '", currentImageLayer.name, "' in map '", path, "' is missing the <image> element. Skipping image info.");
                }

                imageLayers.push_back(currentImageLayer);
            }
            // --- Group Layer Parsing (Optional/Future) ---
            // else if (string_equal(elementName, "group")) {
            //     // TODO: Handle nested groups recursively if needed
            //     debug_log("Info: Skipping nested layer group '", element->Attribute("name"), "' (not fully supported).");
            // }
            // --- Ignore other known elements ---
            // else if (string_equal(elementName, "tileset") || string_equal(elementName, "properties")) {
            //     // Already handled or part of other elements
            // }
            // --- Warn about unknown elements ---
            // else {
            //     debug_log("Info: Skipping unknown top-level map element: <", elementName, "> in map '", path, "'.");
            // }
        }

        if (tilesets.empty()) {
            debug_log("Warning: No embedded tilesets were successfully loaded for map: ", path);
        }

        return true;
    }
}