namespace Amara {
    // Assuming you have loaded the asset:
    // Amara::Tiled_TilemapAsset* mapAsset = Props::assets->get("your_map_key")->as<Amara::Tiled_TilemapAsset*>();
    // Amara::Node* parentNode; // The node where you'll add the tile sprites

    void renderTileLayer(Amara::Tiled_TilemapAsset* mapAsset, int layerIndex, Amara::Node* parentNode) {
        if (!mapAsset || layerIndex < 0 || layerIndex >= mapAsset->layers.size()) {
            Amara::debug_log("Error: Invalid map asset or layer index.");
            return;
        }

        const Amara::Tiled_TileLayer& layer = mapAsset->layers[layerIndex];

        // Define flip flags (matching Tiled's specification)
        const unsigned int FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
        const unsigned int FLIPPED_VERTICALLY_FLAG   = 0x40000000;
        const unsigned int FLIPPED_DIAGONALLY_FLAG   = 0x20000000;
        const unsigned int ALL_FLIP_FLAGS = FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG | FLIPPED_DIAGONALLY_FLAG;

        // Loop through each tile position (y rows, x columns)
        for (unsigned int y = 0; y < layer.height; ++y) {
            for (unsigned int x = 0; x < layer.width; ++x) {
                // Calculate the index in the 1D data vector
                unsigned int index = y * layer.width + x;
                if (index >= layer.data.size()) {
                    Amara::debug_log("Warning: Index out of bounds for layer data.");
                    continue; // Should not happen if loaded correctly
                }

                // Get the Global Tile ID (GID)
                unsigned int gid = layer.data[index];

                // --- Step 1: Check for empty tile ---
                if (gid == 0) {
                    continue; // Skip empty tiles
                }

                // --- Step 2: Extract flip flags ---
                bool flipped_horizontally = (gid & FLIPPED_HORIZONTALLY_FLAG);
                bool flipped_vertically   = (gid & FLIPPED_VERTICALLY_FLAG);
                bool flipped_diagonally   = (gid & FLIPPED_DIAGONALLY_FLAG); // Less common, might need special rotation/texture coord handling

                // --- Step 3: Get the clean GID (without flags) ---
                unsigned int cleanGid = gid & ~ALL_FLIP_FLAGS;

                // --- Step 4: Find the tileset this GID belongs to ---
                const Amara::Tiled_Tileset* tileset = mapAsset->findTilesetForGid(cleanGid);
                if (!tileset) {
                    Amara::debug_log("Warning: Could not find tileset for GID: ", cleanGid);
                    continue; // Skip if tileset not found
                }

                // --- Step 5: Calculate the Local Tile ID (within the tileset) ---
                unsigned int localTileId = cleanGid - tileset->firstGid;

                // --- Step 6: Determine the source rectangle in the tileset image ---
                // This requires knowing the tileset image asset key. Tiled stores this
                // externally or sometimes embedded. The current parser doesn't seem
                // to store the image path directly *in* Tiled_Tileset, which is a limitation.
                // You'll likely need a convention (e.g., tileset name matches image asset key)
                // or enhance the parser to store the image source from the TMX/TSX.

                // Assuming you have the tileset image asset key (e.g., from tileset->name)
                std::string tilesetImageKey = tileset->name; // Example convention
                Amara::ImageAsset* tilesetImage = Amara::Props::assets->get(tilesetImageKey)->as<Amara::ImageAsset*>();

                if (!tilesetImage) {
                    Amara::debug_log("Warning: Tileset image asset not found: ", tilesetImageKey);
                    continue;
                }

                unsigned int tileX = localTileId % tileset->columns;
                unsigned int tileY = localTileId / tileset->columns;

                // Calculate pixel coordinates within the tileset image
                int srcX = tileset->margin + tileX * (tileset->tileWidth + tileset->spacing);
                int srcY = tileset->margin + tileY * (tileset->tileHeight + tileset->spacing);
                int srcW = tileset->tileWidth;
                int srcH = tileset->tileHeight;

                // --- Step 7: Create and configure a Sprite ---
                Amara::Sprite* tileSprite = parentNode->createChild("Sprite")->as<Amara::Sprite*>();
                tileSprite->setTexture(tilesetImageKey);

                // Set position (adjust for layer offsets if needed)
                tileSprite->pos.x = x * mapAsset->tileWidth;
                tileSprite->pos.y = y * mapAsset->tileHeight;
                // You might want to adjust origin, e.g., tileSprite->setOrigin(0, 0);

                // Set the source rectangle using cropping
                tileSprite->cropLeft = srcX;
                tileSprite->cropTop = srcY;
                // Calculate right/bottom crop based on image dimensions and source rect
                tileSprite->cropRight = tilesetImage->width - (srcX + srcW);
                tileSprite->cropBottom = tilesetImage->height - (srcY + srcH);

                // Apply flips/rotation (Amara::Sprite doesn't directly support diagonal flip)
                // You might need custom shader logic or pre-rotated assets for diagonal flips.
                // Horizontal/Vertical flips can often be done by swapping/negating scale or texture coords.
                // For simplicity, let's just log them for now. Amara::Sprite doesn't have direct flip properties.
                // You might need to adjust texture coordinates manually or use scale.x = -1 / scale.y = -1
                // combined with origin adjustments.
                if (flipped_horizontally) {
                    // tileSprite->scale.x *= -1; // Needs origin adjustment
                    Amara::debug_log("Tile at (", x, ",", y, ") is flipped horizontally.");
                }
                if (flipped_vertically) {
                    // tileSprite->scale.y *= -1; // Needs origin adjustment
                    Amara::debug_log("Tile at (", x, ",", y, ") is flipped vertically.");
                }
                if (flipped_diagonally) {
                    Amara::debug_log("Tile at (", x, ",", y, ") is flipped diagonally (requires special handling).");
                }

                // Set other properties if needed (opacity, tint, depth)
                tileSprite->alpha = layer.opacity;
                // tileSprite->depth = ... // Set depth based on layer order or map render order
            }
        }
    }
}