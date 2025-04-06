namespace Amara {
    struct Glyph {
        float u0, v0, u1, v1; // UV texture coordinates
        int width, height;
        int xoffset, yoffset;
        int xadvance;
    };
    
    class FontAsset: public Asset {
    public:
        static const int atlasWidth = 2048;
        static const int atlasHeight = 2048;

        std::map<int, Glyph> glyphCache;
        
        int currentX = 0, currentY = 0;
        int rowHeight = 0;

        unsigned char *fontBuffer;
        stbtt_fontinfo font;

        int fontSize = 0;

        SDL_Texture* texture = nullptr;
        #ifdef AMARA_OPENGL
        GLuint glTextureID = 0;
        #endif

        bool loadFont(std::string _p, int _size) {
            path = Props::files->getAssetPath(_p);
            fontSize = _size;

            if (!Props::files->fileExists(path)) {
                debug_log("Error: File not found at ", path);
                return false;
            }

            clearTexture();

            SDL_IOStream *rw = SDL_IOFromFile(path.c_str(), "rb");
            if (!rw) {
                debug_log("Failed to open file: ", SDL_GetError());
                return false;
            }

            Sint64 fileSize = SDL_GetIOSize(rw);
            fontBuffer = (unsigned char*)SDL_malloc(fileSize);
            SDL_ReadIO(rw, fontBuffer, fileSize);
            SDL_CloseIO(rw);

            stbtt_InitFont(&font, fontBuffer, stbtt_GetFontOffsetForIndex(fontBuffer, 0));

            #ifdef AMARA_OPENGL
            if (Props::graphics == GraphicsEnum::OpenGL && Props::glContext != NULL) {
                GLuint glTextureID;
                glGenTextures(1, &glTextureID);

                if (glTextureID == 0) {
                    debug_log("Error: Texture generation failed. ", path);
                    return false;
                }

                glBindTexture(GL_TEXTURE_2D, glTextureID);

                unsigned char* emptyData = (unsigned char*)calloc(atlasWidth * atlasHeight, 1);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, emptyData);
                free(emptyData);
                glGenerateMipmap(GL_TEXTURE_2D);
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
            #endif

            return false;
        }

        void addGlyphToTexture(int codepoint) {
            if (glyphCache.find(codepoint) != glyphCache.end()) return;
        
            int width, height, xoff, yoff;
            unsigned char* bitmap = stbtt_GetCodepointBitmap(&font, 0, stbtt_ScaleForPixelHeight(&font, fontSize), codepoint, &width, &height, &xoff, &yoff);
        
            // Check if we need to start a new row
            if (currentX + width >= atlasWidth) {
                currentX = 0;
                currentY += rowHeight;
                rowHeight = 0;
            }
        
            if (currentY + height >= atlasHeight) {
                debug_log("Error: Texture atlas for font \"", key ,"\" is full!");
                stbtt_FreeBitmap(bitmap, nullptr);
                return;
            }
            
            #ifdef AMARA_OPENGL
            if (Props::graphics == GraphicsEnum::OpenGL && Props::glContext != NULL) {
                glBindTexture(GL_TEXTURE_2D, glTextureID);
                glTexSubImage2D(GL_TEXTURE_2D, 0, currentX, currentY, width, height, GL_RED, GL_UNSIGNED_BYTE, bitmap);
            }
            #endif
            
            // Store glyph metadata
            Glyph glyph;
            glyph.u0 = (float)currentX / atlasWidth;
            glyph.v0 = (float)currentY / atlasHeight;
            glyph.u1 = (float)(currentX + width) / atlasWidth;
            glyph.v1 = (float)(currentY + height) / atlasHeight;
            glyph.width = width;
            glyph.height = height;
            glyph.xoffset = xoff;
            glyph.yoffset = yoff;
            
            int advance;
            stbtt_GetCodepointHMetrics(&font, codepoint, &advance, nullptr);
            glyph.xadvance = advance * stbtt_ScaleForPixelHeight(&font, 32);
        
            glyphCache[codepoint] = glyph;
        
            // Move packing cursor
            currentX += width + 2; // Add padding
            rowHeight = std::max(rowHeight, height);
        
            stbtt_FreeBitmap(bitmap, nullptr);
        }

        void packGlyphsFromString(std::u32string str) {
            for (char32_t codepoint : str) {
                int glyphID = static_cast<int>(codepoint);
                addGlyphToTexture(glyphID);
            }
        }

        void packGlyphsFromString(std::string str) {
            packGlyphsFromString(Amara::String::utf8_to_utf32(str));
        }

        Rectangle getSize(std::u32string str, int wordWrapWidth) {
            float width = 0, height = 0;
            float maxHeight = 0;
            float lineWidth = 0;
        
            for (char32_t codepoint : str) {
                if (codepoint == U' ') {
                    lineWidth += glyphCache[U' '].xadvance;
                    continue;
                }
                if (codepoint == U'\n') {
                    height += maxHeight;
                    maxHeight = 0;
                    lineWidth = 0;
                    continue;
                }
        
                if (glyphCache.find(codepoint) == glyphCache.end()) {
                    continue;
                }
        
                Glyph &glyph = glyphCache[codepoint];
                lineWidth += glyph.xadvance;
                maxHeight = std::max(maxHeight, static_cast<float>(glyph.height));
                
                if (wordWrapWidth > 0 && lineWidth > wordWrapWidth) {
                    height += maxHeight;
                    maxHeight = glyph.height;
                    lineWidth = glyph.xadvance;
                }

                width = std::max(width, lineWidth);
            }
        
            height += maxHeight;
            return { 0, 0, width, height };
        }
        Rectangle getSize(std::string str, int wordWrapWidth) {
            return getSize(Amara::String::utf8_to_utf32(str), wordWrapWidth);
        }

        virtual void clearTexture() {
            if (texture) {
                SDL_DestroyTexture(texture);
                texture = nullptr;
            }
            #ifdef AMARA_OPENGL
            if (glTextureID != 0) {
                Props::queue_texture_garbage(glTextureID);
                glTextureID = 0;
            }
            #endif
        }

        void destroy() {
            clearTexture();
            Amara::Asset::destroy();
        }
    };
}