namespace Amara {
    struct Glyph {
        float x = 0, y = 0; // Position to render
        float u0 = 0, v0 = 0, u1 = 0, v1 = 0; // Texture coordinates
        int width = 0, height = 0;
        Rectangle src;
        int xoffset, yoffset;
        int xadvance;

        bool renderable = true;
    };

    class TextLine {
    public:
        TextLine() {
            reset();
        }

        std::u32string text;
        std::vector<Glyph> glyphs;

        int x = 0, y = 0; // Position to render
        int width = 0, height = 0;

        float cursorX = 0;

        void reset() {
            text.clear();
            glyphs.clear();

            x = 0;
            y = 0;

            width = 0;
            height = 0;
        }

        void merge(const TextLine& other) {
            text += other.text;
            
            for (Glyph glyph: other.glyphs) {
                glyph.x += other.x;
                glyphs.push_back(glyph);
            }

            width += other.width;
            height = std::max(height, other.height);
        }

        int size() {
            return text.size();
        }
    };

    class TextLayout {
    public:
        TextLayout() {
            text.clear();
            lines.clear();
            width = 0;
            height = 0;
        }

        std::u32string text;
        std::deque<TextLine> lines;
        int width, height;

        TextLine& newLine() {
            lines.emplace_back();
            return lines.back();
        }

        TextLine& getLine(int index) {
            return lines[index];
        }
        TextLine& getLastLine() {
            return lines.back();
        }
    };
    
    class FontAsset: public Asset {
    public:
        static const int atlasWidth = 2048;
        static const int atlasHeight = 2048;

        std::map<int, Glyph> glyphCache;
        
        int currentX = 0, currentY = 0;
        int rowHeight = 0;

        float scale = 1;
        int ascent = 0;
        int descent = 0;
        int lineGap = 0;
        int baseline = 0;
        float lineHeight = 0;

        unsigned char *fontBuffer;
        stbtt_fontinfo font;

        int fontSize = 0;

        SDL_Texture* texture = nullptr;
        #ifdef AMARA_OPENGL
        GLuint glTextureID = 0;
        #endif

        bool loadFont(std::string _p, int _size) {
            path = Props::system->getAssetPath(_p);
            fontSize = _size;

            if (!Props::system->fileExists(path)) {
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

            scale = stbtt_ScaleForMappingEmToPixels(&font, fontSize);

            stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
            baseline = (int)(ascent * scale);

            lineHeight = (ascent - descent + lineGap) * scale;

            if (Props::graphics == GraphicsEnum::Render2D && Props::renderer) {
                texture = SDL_CreateTexture(Props::renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, atlasWidth, atlasHeight);
                return true;
            }
            #ifdef AMARA_OPENGL
            else if (Props::graphics == GraphicsEnum::OpenGL && Props::glContext != NULL) {
                glGenTextures(1, &glTextureID);

                if (glTextureID == 0) {
                    debug_log("Error: Texture generation failed. ", path);
                    return false;
                }

                glBindTexture(GL_TEXTURE_2D, glTextureID);
                
                unsigned char* emptyData = (unsigned char*)calloc(atlasWidth * atlasHeight, 1);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, atlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, emptyData);
                free(emptyData);
                glGenerateMipmap(GL_TEXTURE_2D);
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glBindTexture(GL_TEXTURE_2D, 0);
                return true;
            }
            #endif

            return false;
        }

        void addGlyphToTexture(int codepoint) {
            if (glyphCache.find(codepoint) != glyphCache.end()) return;
        
            int width, height, xoff, yoff;
            unsigned char* monobitmap = stbtt_GetCodepointBitmap(&font, 0, scale, codepoint, &width, &height, &xoff, &yoff);

            // Check if we need to start a new row
            if (currentX + width >= atlasWidth) {
                currentX = 0;
                currentY += rowHeight;
                rowHeight = 0;
            }
        
            if (currentY + height >= atlasHeight) {
                debug_log("Error: Texture atlas for font \"", key ,"\" is full!");
                stbtt_FreeBitmap(monobitmap, nullptr);
                return;
            }

            unsigned char* rgbaBitmap = new unsigned char[width * height * 4];
            
            for (int i = 0; i < width * height; ++i) {
                unsigned char alpha = monobitmap[i];
                rgbaBitmap[i * 4 + 0] = 255;    // R
                rgbaBitmap[i * 4 + 1] = 255;    // G
                rgbaBitmap[i * 4 + 2] = 255;    // B
                rgbaBitmap[i * 4 + 3] = alpha;  // A
            }
            
            if (Props::graphics == GraphicsEnum::Render2D && Props::renderer) {
                SDL_Rect destRect = { currentX, currentY, width, height };
                SDL_UpdateTexture(texture, &destRect, rgbaBitmap, width * 4);
            }
            #ifdef AMARA_OPENGL
            else if (Props::graphics == GraphicsEnum::OpenGL && Props::glContext != NULL) {
                glBindTexture(GL_TEXTURE_2D, glTextureID);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glTexSubImage2D(GL_TEXTURE_2D, 0, currentX, currentY, width, height, GL_RGBA, GL_UNSIGNED_BYTE, rgbaBitmap);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            #endif
            
            // Store glyph metadata
            Glyph glyph;
            glyph.src = Rectangle(
                (float)currentX,
                (float)currentY,
                (float)width,
                (float)height
            );
            glyph.u0 = (float)currentX / atlasWidth;
            glyph.v0 = (float)currentY / atlasHeight;
            glyph.u1 = (float)(currentX + width) / atlasWidth;
            glyph.v1 = (float)(currentY + height) / atlasHeight;
            glyph.xoffset = xoff;
            glyph.yoffset = yoff;
            glyph.width = width;
            glyph.height = height;
            
            int advance;
            stbtt_GetCodepointHMetrics(&font, codepoint, &advance, nullptr);
            glyph.xadvance = advance * scale;
        
            glyphCache[codepoint] = glyph;
        
            // Move packing cursor
            currentX += width + 2; // Add padding
            rowHeight = std::max(rowHeight, height);
        
            stbtt_FreeBitmap(monobitmap, nullptr);
            delete[] rgbaBitmap;
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

        TextLayout generateLayout(std::u32string str, int wrapWidth, WrapModeEnum wrapMode, AlignmentEnum alignment, int lineSpacing) {
            TextLayout layout = TextLayout();
            layout.text = str;

            float cursorX = 0, cursorY = 0;

            TextLine* line = &layout.newLine();
            line->height = lineHeight;
            
            if (wrapMode == WrapModeEnum::ByCharacter) {
                for (int i = 0; i < str.size(); ++i) {
                    char32_t codepoint = str[i];
                    if (glyphCache.find(codepoint) == glyphCache.end()) {
                        Glyph glyph;
                        glyph.renderable = false;
                        line->glyphs.push_back(glyph);
                        continue;
                    }
                    Glyph glyph = glyphCache[codepoint];

                    if (codepoint == U'\t' || codepoint == U' ') {
                        cursorX += glyph.xadvance;
                        if (wrapWidth <= 0 || ((line->width + glyph.xadvance) <= wrapWidth)) {
                            line->width += glyph.xadvance;
                        }
                        glyph.renderable = false;
                        line->glyphs.push_back(glyph);
                        continue;
                    }
                    if (codepoint == U'\r') { // Carriage return handling
                        glyph.renderable = false;
                        line->glyphs.push_back(glyph);
                        continue;
                    }

                    if (codepoint == U'\n' || (wrapWidth > 0 && (line->width + glyph.xadvance) > wrapWidth)) {
                        layout.height += line->height + lineSpacing;

                        cursorX = 0;
                        cursorY += lineHeight + lineSpacing;
                        line = &layout.newLine();
                        line->height = lineHeight;
                        line->y = cursorY;

                        if (codepoint == U'\n') {
                            glyph.renderable = false;
                            line->glyphs.push_back(glyph);
                            continue;
                        }
                        else glyph.x = 0;
                    }
                    
                    glyph.x = cursorX + glyph.xoffset;
                    glyph.y = fontSize + glyph.yoffset;
                    cursorX += glyph.xadvance;
                    line->width += glyph.xadvance;

                    line->text += codepoint;
                    line->glyphs.push_back(glyph);

                    layout.width = std::max(layout.width, line->width);
                }
                layout.height += line->height;
            }
            else if (wrapMode == WrapModeEnum::ByWord) {
                TextLine word = TextLine();

                for (int i = 0; i < str.size(); ++i) {
                    char32_t codepoint = str[i];
                    if (glyphCache.find(codepoint) == glyphCache.end()) {
                        Glyph glyph;
                        glyph.renderable = false;
                        word.glyphs.push_back(glyph);
                        continue;
                    }
                    
                    Glyph glyph = glyphCache[codepoint];

                    if (codepoint == U'\t' || codepoint == U' ') {
                        if (wrapWidth > 0 && (line->width + word.width) > wrapWidth) {
                            layout.height += line->height + lineSpacing;
                            
                            cursorY += lineHeight + lineSpacing;
                            line = &layout.newLine();
                            line->height = lineHeight;
                            line->y = cursorY;
                            
                            word.x = 0;
                        }
                        cursorX += glyph.xadvance;
                        word.width += glyph.xadvance;

                        line->merge(word);
                        cursorX += word.x;

                        word = TextLine();
                        word.x += cursorX;
                        cursorX = 0;

                        glyph.renderable = false;
                        word.glyphs.push_back(glyph);
                        continue;
                    }
                    if (codepoint == U'\r') { // Carriage return handling-
                        glyph.renderable = false;
                        word.glyphs.push_back(glyph);
                        continue;  
                    }

                    if (codepoint == U'\n') {
                        layout.height += line->height + lineSpacing;

                        glyph.renderable = false;
                        word.glyphs.push_back(glyph);

                        line->merge(word);

                        word = TextLine();

                        cursorX = 0;
                        cursorY += fontSize + lineSpacing;
                        line = &layout.newLine();
                        line->height = lineHeight;
                        line->y = cursorY;
                        continue;
                    }

                    glyph.x = cursorX + glyph.xoffset;
                    glyph.y = fontSize + glyph.yoffset;
                    cursorX += glyph.xadvance;

                    word.text += codepoint;
                    word.width += glyph.xadvance;
                    word.glyphs.push_back(glyph);

                    layout.width = std::max(layout.width, line->width + word.width);
                }
                line->merge(word);
                layout.height += line->height;
            }

            float alignmentOffset = 0;
            switch (alignment) {
                case Amara::AlignmentEnum::Left:
                    alignmentOffset = 0;
                    break;
                case Amara::AlignmentEnum::Center:
                    alignmentOffset = 0.5f;
                    break;
                case Amara::AlignmentEnum::Right:
                    alignmentOffset = 1.0f;
                    break;
            }
            for (TextLine& line : layout.lines) {
                line.x = alignmentOffset * (layout.width - line.width);
            }

            return layout;
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

            if (fontBuffer) {
                SDL_free(fontBuffer);
                fontBuffer = nullptr;
            }
        }

        void destroy() {
            clearTexture();
            Amara::Asset::destroy();
        }
    };
}