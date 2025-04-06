namespace Amara {
    class Text: public Amara::Node {
    public:
        std::string text;
        std::u32string converted_text;

        Amara::FontAsset* font = nullptr;
        
        int progress = 0;

        int textwidth = 0;
        int textheight = 0;
        int wordWrapWidth = -1;

        #ifdef AMARA_OPENGL
        std::array<float, 16> vertices = {
            -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
             0.5f, -0.5f,  1.0f, 0.0f, // Bottom-right
             0.5f,  0.5f,  1.0f, 1.0f, // Top-right
            -0.5f,  0.5f,  0.0f, 1.0f  // Top-left
        };
        #endif

        Text(): Amara::Node() {
            set_base_node_id("Text");
        }

        void updateText() {
            if (font) {
                font->packGlyphsFromString(converted_text);
                Rectangle dim = font->getSize(converted_text, wordWrapWidth);
                textwidth = dim.w;
                textheight = dim.h;
            }
        }

        void setText(std::string str) {
            text = str;

            std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
            converted_text = converter.from_bytes(str);  // Convert UTF-8 string to UTF-32

            progress = converted_text.size();
            updateText();
        }

        bool setFont(std::string key) {
            font = nullptr;
            if (!Props::assets->has(key)) {
                debug_log("Error: Asset \"", key, "\" was not found.");
                return false;
            }

            font = Props::assets->get(key)->as<Amara::FontAsset*>();

            if (font == nullptr) {
                debug_log("Error: Asset \"", key, "\" is not a valid font asset.");
                return false;
            }

            if (!converted_text.empty()) {
                updateText();
            }

            return true;
        }

        void drawSelf(const Rectangle& v) override {
            if (font == nullptr || converted_text.empty()) return;

            Vector2 vcenter = { v.w/2.0f, v.h/2.0f };

            float cursorX = 0, cursorY = 0;

            if (progress > converted_text.size()) {
                progress = converted_text.size();
            }
            
            Vector3 anchoredPos = Vector3(
                rotateAroundAnchor(
                    passOn.anchor, 
                    Vector2( 
                        (passOn.anchor.x + pos.x*passOn.scale.x), 
                        (passOn.anchor.y + pos.y*passOn.scale.y)
                    ),
                    passOn.rotation
                ),
                passOn.anchor.z + pos.z
            );

            SDL_FRect srcRect, destRect;
            Rectangle dim;

            SDL_FPoint dorigin = { 0, 0 };

            for (int i = 0; i < progress && i < converted_text.size(); i++) {
                char32_t c = converted_text[i];
                if (c == U' ') {
                    cursorX += font->glyphCache[U' '].xadvance;  // Space handling
                    continue;
                }
                if (c == U'\n') {
                    cursorX = 0;  // Reset cursorX for new line
                    cursorY += font->fontSize;  // Move down for new line
                    continue;
                }

                if (font->glyphCache.find(c) == font->glyphCache.end()) {
                    continue;  // Skip if glyph not found
                }
                Glyph &glyph = font->glyphCache[c];

                Vector3 glyphPos = Vector3(
                    rotateAroundAnchor(
                        anchoredPos, 
                        Vector2( 
                            anchoredPos.x + cursorX + glyph.xoffset,
                            anchoredPos.y + cursorY - glyph.yoffset
                        ),
                        rotation
                    ),
                    anchoredPos.z
                );
                float glyphw = glyph.width;
                float glyphh = glyph.height;

                srcRect = {
                    glyph.u0 * font->atlasWidth,
                    glyph.v0 * font->atlasHeight,
                    glyph.u1 * font->atlasWidth - glyph.u0 * font->atlasWidth,
                    glyph.v1 * font->atlasHeight - glyph.v0 * font->atlasHeight
                };

                dim = {
                    glyphPos.x,
                    glyphPos.y - glyphPos.z,
                    glyphw*scale.x*passOn.scale.x,
                    glyphh*scale.y*passOn.scale.y
                };

                destRect.x = vcenter.x + dim.x*passOn.zoom.x;
                destRect.y = vcenter.y + dim.y*passOn.zoom.y;
                destRect.w = dim.w * passOn.zoom.x;
                destRect.h = dim.h * passOn.zoom.y;

                cursorX += glyph.xadvance;
            }
        }

        int length() {
            return converted_text.size();
        }

        void bindLua(sol::state& lua) {
            lua.new_usertype<Text>("Text",
                sol::base_classes, sol::bases<Node>(),
                "setText", &Text::setText,
                "setFont", &Text::setFont,
                "progress", &Text::progress,
                "length", sol::property(&Text::length)
            );
        }
    };
}