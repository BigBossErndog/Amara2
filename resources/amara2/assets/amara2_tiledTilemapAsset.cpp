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
        unsigned int gid = 0; // Optional tile GID (object represented by a tile)
        bool visible = true;

        Tiled_ObjectType objectType = Tiled_ObjectType::Rectangle; // Default

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

    // --- Image Layer Structures ---
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
        Tiled_Properties mapProperties;             // Added for map-level properties

        Tiled_TilemapAsset() {
            type = AssetEnum::TiledTilemap;
            typeKey = "Tiled_TilemapAsset";
        }

        bool loadTmx(const std::string& tmxPath);

        const Tiled_Tileset* findTilesetForGid(unsigned int gid) const {
            // ... (implementation remains the same)
            for (auto it = tilesets.rbegin(); it != tilesets.rend(); ++it) {
                if (gid >= it->firstGid) {
                    return &(*it);
                }
            }
            return nullptr;
        }

        // Helper function to parse properties (can be placed inside the class or outside in the namespace)
        static Tiled_Properties parseProperties(tinyxml2::XMLElement* parentElement);
        // Helper function to parse points string (can be placed inside the class or outside)
        static std::vector<std::pair<float, float>> parsePointsString(const std::string& pointsStr);
    };

    // --- Helper Function Implementations ---

    Tiled_Properties Tiled_TilemapAsset::parseProperties(tinyxml2::XMLElement* parentElement) {
        Tiled_Properties props;
        if (!parentElement) return props;

        tinyxml2::XMLElement* propertiesElement = parentElement->FirstChildElement("properties");
        if (!propertiesElement) return props;

        for (tinyxml2::XMLElement* propElement = propertiesElement->FirstChildElement("property"); propElement != nullptr; propElement = propElement->NextSiblingElement("property")) {
            const char* name = propElement->Attribute("name");
            const char* type = propElement->Attribute("type"); // Optional
            const char* value = propElement->Attribute("value"); // May be null if value is element text

            if (name) {
                Tiled_Property prop;
                prop.name = name;
                if (type) prop.type = type; else prop.type = "string"; // Default type

                if (value) {
                    prop.value = value;
                } else {
                    // Handle multi-line string properties where value is the text content
                    const char* textValue = propElement->GetText();
                    prop.value = textValue ? textValue : ""; // Use empty string if no text
                    // Ensure type is string if it was inferred from text content
                    if (!type) prop.type = "string";
                }
                props[prop.name] = prop; // Add or overwrite property
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

        while (std::getline(ss, pairStr, ' ')) { // Split by space
            std::stringstream ssPair(pairStr);
            std::string coordStr;
            float x = 0.0f, y = 0.0f;
            int coordIndex = 0;

            while (std::getline(ssPair, coordStr, ',')) { // Split by comma
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
            if (coordIndex >= 2) { // Ensure we got at least x and y
                 points.push_back({x, y});
            } else if (!pairStr.empty()) {
                 debug_log("Warning: Incomplete coordinate pair '", pairStr, "' in points string '", pointsStr, "'. Skipping.");
            }
        }
        return points;
    }


    // --- loadTmx Implementation ---
    bool Tiled_TilemapAsset::loadTmx(const std::string& tmxPath) {
        path = Props::files->getAssetPath(tmxPath);

        SDL_IOStream* rw = SDL_IOFromFile(path.c_str(), "rb");
        Sint64 fileSize_s64 = SDL_GetIOSize(rw);
        // ... (Error checking for fileSize_s64, size limits) ...
        size_t fileSize = static_cast<size_t>(fileSize_s64);
        char* buffer = new (std::nothrow) char[fileSize + 1];
        // ... (Error checking for buffer allocation) ...
        size_t bytesRead = SDL_ReadIO(rw, buffer, fileSize);
        SDL_CloseIO(rw);
        // ... (Error checking for bytesRead) ...
        buffer[fileSize] = '\0';
        tinyxml2::XMLDocument doc;
        tinyxml2::XMLError parseResult = doc.Parse(buffer, fileSize);
        delete[] buffer;
        buffer = nullptr;
        // ... (Error checking for parseResult) ...
        tinyxml2::XMLElement* mapRoot = doc.RootElement();
        // ... (Error checking for mapRoot) ...

        // --- Parse Map Attributes ---
        mapRoot->QueryUnsignedAttribute("width", &width);
        mapRoot->QueryUnsignedAttribute("height", &height);
        mapRoot->QueryUnsignedAttribute("tilewidth", &tileWidth);
        mapRoot->QueryUnsignedAttribute("tileheight", &tileHeight);
        // ... (Parse orientation, renderOrder) ...
        const char* orient = nullptr;
        mapRoot->QueryStringAttribute("orientation", &orient);
        if (orient) orientation = orient;
        const char* render = nullptr;
        mapRoot->QueryStringAttribute("renderorder", &render);
        if (render) renderOrder = render;

        debug_log("Loading TMX: ", path, " (", width, "x", height, " tiles, ", tileWidth, "x", tileHeight, " pixels)");

        // --- Parse Map Properties ---
        mapProperties = parseProperties(mapRoot);
        if (!mapProperties.empty()) {
             debug_log("Parsed ", mapProperties.size(), " map properties.");
        }

        // --- Parse Tilesets ---
        tilesets.clear(); // Clear any previous data
        for (tinyxml2::XMLElement* tsElement = mapRoot->FirstChildElement("tileset"); tsElement != nullptr; tsElement = tsElement->NextSiblingElement("tileset")) {
            // ... (Existing tileset parsing logic remains largely the same) ...
            // You might want to add parsing for tileset properties or embedded images here if needed
            // currentTileset.properties = parseProperties(tsElement);
            // tinyxml2::XMLElement* imgElement = tsElement->FirstChildElement("image");
            // if (imgElement) { /* parse image attributes */ }
            Tiled_Tileset currentTileset;
            tsElement->QueryUnsignedAttribute("firstgid", &currentTileset.firstGid);
            if (currentTileset.firstGid == 0) { /* Warning */ continue; }
            const char* source = tsElement->Attribute("source");
            if (source) { /* Warning, skip external */ continue; }
            const char* tsName = nullptr;
            tsElement->QueryStringAttribute("name", &tsName);
            if (tsName) currentTileset.name = tsName;
            tsElement->QueryUnsignedAttribute("tilewidth", &currentTileset.tileWidth);
            tsElement->QueryUnsignedAttribute("tileheight", &currentTileset.tileHeight);
            tsElement->QueryUnsignedAttribute("tilecount", &currentTileset.tileCount);
            tsElement->QueryUnsignedAttribute("columns", &currentTileset.columns);
            tsElement->QueryUnsignedAttribute("spacing", &currentTileset.spacing);
            tsElement->QueryUnsignedAttribute("margin", &currentTileset.margin);
            tilesets.push_back(currentTileset);
            debug_log("Added tileset: ", currentTileset.name, " (", currentTileset.tileCount, " tiles)");
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
                const char* layerName = nullptr;
                element->QueryStringAttribute("name", &layerName);
                if (layerName) currentLayer.name = layerName;
                
                unsigned int layerWidth = 0, layerHeight = 0;
                element->QueryUnsignedAttribute("width", &layerWidth);
                element->QueryUnsignedAttribute("height", &layerHeight);
                if (layerWidth != width || layerHeight != height) { /* Warning */ }
                currentLayer.width = width;
                currentLayer.height = height;
                element->QueryFloatAttribute("opacity", &currentLayer.opacity);
                element->QueryBoolAttribute("visible", &currentLayer.visible);

                currentLayer.properties = parseProperties(element);

                tinyxml2::XMLElement* dataElement = element->FirstChildElement("data");
                if (!dataElement) { /* Warning, skip */ continue; }

                // ... (Existing data parsing logic: encoding, compression, text processing) ...
                const char* encoding = dataElement->Attribute("encoding");
                const char* compression = dataElement->Attribute("compression");
                const char* dataText = dataElement->GetText();
                if (!dataText) { /* Warning, skip */ continue; }

                std::string dataString = dataText;
                dataString.erase(std::remove_if(dataString.begin(), dataString.end(), ::isspace), dataString.end());
                currentLayer.data.reserve(width * height);

                if (!encoding) {
                    // XML encoding
                     for (tinyxml2::XMLElement* tileElement = dataElement->FirstChildElement("tile"); tileElement != nullptr; tileElement = tileElement->NextSiblingElement("tile")) {
                        unsigned int gid = 0;
                        tileElement->QueryUnsignedAttribute("gid", &gid);
                        currentLayer.data.push_back(gid);
                    }
                    if (currentLayer.data.size() != width * height) { /* Warning */ }
                }
                else if (string_equal(std::string(encoding), "csv")) {
                    std::stringstream ss(dataString);
                    std::string value;
                    while (std::getline(ss, value, ',')) {
                        try { currentLayer.data.push_back(std::stoul(value)); }
                        catch (...) { /* Error handling */ currentLayer.data.push_back(0); }
                    }
                     if (currentLayer.data.size() != width * height) { /* Warning */ }
                }
                else if (string_equal(std::string(encoding), "base64")) {
                    // ... Base64 decoding and decompression ...
                    std::vector<unsigned char> decoded_data = base64_decode(dataString);
                    if (decoded_data.empty() && !dataString.empty()) { /* Error, skip */ continue; }
                    std::vector<unsigned char> decompressed_data = decoded_data;
                    if (compression) {
                        decompressed_data = decompress_data(decoded_data, std::string(compression));
                        if (decompressed_data.empty() && !decoded_data.empty()) { /* Error, skip */ continue; }
                    }
                    size_t expected_bytes = width * height * 4;
                    if (decompressed_data.size() != expected_bytes) { /* Warning */ }
                    for (size_t i = 0; (i + 3) < decompressed_data.size(); i += 4) {
                        unsigned int gid = decompressed_data[i] | (decompressed_data[i + 1] << 8) | (decompressed_data[i + 2] << 16) | (decompressed_data[i + 3] << 24);
                        currentLayer.data.push_back(gid);
                    }
                    if (currentLayer.data.size() != width * height) { /* Warning */ }
                }
                else {
                    debug_log("Error: Unsupported layer data encoding '", encoding, "' for layer '", currentLayer.name, "'. Skipping layer.");
                    continue;
                }

                layers.push_back(currentLayer);
            }
            else if (string_equal(elementName, "objectgroup")) {
                Tiled_ObjectGroup currentGroup;
                const char* groupName = nullptr;
                element->QueryStringAttribute("name", &groupName);
                if (groupName) currentGroup.name = groupName;

                const char* colorStr = nullptr;
                element->QueryStringAttribute("color", &colorStr);
                if (colorStr) currentGroup.color = colorStr;

                element->QueryFloatAttribute("opacity", &currentGroup.opacity);
                element->QueryBoolAttribute("visible", &currentGroup.visible);
                element->QueryFloatAttribute("offsetx", &currentGroup.offsetX);
                element->QueryFloatAttribute("offsety", &currentGroup.offsetY);

                const char* drawOrderStr = nullptr;
                element->QueryStringAttribute("draworder", &drawOrderStr);
                if (drawOrderStr) currentGroup.drawOrder = drawOrderStr;

                // Parse Group Properties
                currentGroup.properties = parseProperties(element);

                // Parse Objects within the group
                for (tinyxml2::XMLElement* objElement = element->FirstChildElement("object"); objElement != nullptr; objElement = objElement->NextSiblingElement("object")) {
                    Tiled_Object currentObject;
                    objElement->QueryUnsignedAttribute("id", &currentObject.id); // ID is unique within the map
                    const char* objName = nullptr;
                    objElement->QueryStringAttribute("name", &objName);
                    if (objName) currentObject.name = objName;
                    const char* objType = nullptr;
                    objElement->QueryStringAttribute("type", &objType);
                    if (objType) currentObject.type = objType;

                    objElement->QueryFloatAttribute("x", &currentObject.x);
                    objElement->QueryFloatAttribute("y", &currentObject.y);
                    objElement->QueryFloatAttribute("width", &currentObject.width);   // May be 0
                    objElement->QueryFloatAttribute("height", &currentObject.height); // May be 0
                    objElement->QueryFloatAttribute("rotation", &currentObject.rotation);
                    objElement->QueryUnsignedAttribute("gid", &currentObject.gid); // May be 0
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
                            
                            const char* fontFamily = nullptr;
                            shapeElement->QueryStringAttribute("fontfamily", &fontFamily);
                            if (fontFamily) currentObject.text_fontfamily = fontFamily;

                            shapeElement->QueryIntAttribute("pixelsize", &currentObject.text_pixelsize);
                            shapeElement->QueryBoolAttribute("wrap", &currentObject.text_wrap);

                            const char* color = nullptr;
                            shapeElement->QueryStringAttribute("color", &color);
                            if (color) currentObject.text_color = color; // Stored as string (e.g., #AARRGGBB or #RRGGBB)

                            const char* halign = nullptr;
                            shapeElement->QueryStringAttribute("halign", &halign);
                            if (halign) currentObject.text_halign = halign;

                            const char* valign = nullptr;
                            shapeElement->QueryStringAttribute("valign", &valign);
                            if (valign) currentObject.text_valign = valign;

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
                        // No shape element, defaults to Rectangle unless it's a Tile object
                        if (currentObject.gid == 0) {
                             currentObject.objectType = Tiled_ObjectType::Rectangle;
                        } else {
                             // If GID is set, it is considered a Tile object,
                             // but keep Rectangle type unless specific handling is needed.
                             // Width/Height might come from the tile itself later.
                             currentObject.objectType = Tiled_ObjectType::Rectangle;
                        }
                    }

                    currentGroup.objects.push_back(currentObject);
                }
                objectGroups.push_back(currentGroup);
            }
            else if (string_equal(elementName, "imagelayer")) {
                Tiled_ImageLayer currentImageLayer;
                currentImageLayer.mapDirectory = path;

                const char* imgLayerName = nullptr;
                element->QueryStringAttribute("name", &imgLayerName);
                if (imgLayerName) currentImageLayer.name = imgLayerName;

                element->QueryFloatAttribute("offsetx", &currentImageLayer.offsetX);
                element->QueryFloatAttribute("offsety", &currentImageLayer.offsetY);
                element->QueryFloatAttribute("opacity", &currentImageLayer.opacity);
                element->QueryBoolAttribute("visible", &currentImageLayer.visible);

                currentImageLayer.properties = parseProperties(element);

                tinyxml2::XMLElement* imgElement = element->FirstChildElement("image");
                if (imgElement) {
                    const char* imgSrc = nullptr;
                    imgElement->QueryStringAttribute("source", &imgSrc);
                    if (imgSrc) currentImageLayer.image.source = imgSrc;

                    const char* transColor = nullptr;
                    imgElement->QueryStringAttribute("trans", &transColor);
                    if (transColor) currentImageLayer.image.transparentColor = transColor;

                    imgElement->QueryIntAttribute("width", &currentImageLayer.image.width);
                    imgElement->QueryIntAttribute("height", &currentImageLayer.image.height);
                }
                else {
                    debug_log("Warning: Image layer '", currentImageLayer.name, "' is missing the <image> element. Skipping image info.");
                }

                imageLayers.push_back(currentImageLayer);
            }
            // --- Handle other potential top-level elements ---
            // else if (string_equal(elementName, "group")) {
            //     // TODO: Handle nested groups if needed (recursive parsing)
            //     debug_log("Warning: Nested layer groups are not fully supported yet.");
            // }
            // else if (!string_equal(elementName, "tileset") && !string_equal(elementName, "properties")) {
            //     // Ignore tilesets (parsed earlier) and properties (parsed earlier)
            //     debug_log("Info: Skipping unsupported top-level map element: ", elementName);
            // }
        }

        if (tilesets.empty()) {
            debug_log("Warning: No tilesets found or loaded for map: ", path);
        }
        return true;
    }
}