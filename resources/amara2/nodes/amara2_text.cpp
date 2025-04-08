namespace Amara {
    class Text: public Amara::Node {
    public:
        std::string text;
        std::u32string converted_text;

        Amara::FontAsset* font = nullptr;
        
        int progress = 0;

        Amara::BlendMode blendMode = Amara::BlendMode::Alpha;

        Amara::AlignmentEnum alignment = Amara::AlignmentEnum::Left;

        Vector2 origin = { 0.5, 0.5 };

        int textwidth = 0;
        int textheight = 0;

        int wrapWidth = -1;
        Amara::WrapModeEnum wrapMode = Amara::WrapModeEnum::ByCharacter;

        TextLayout layout;

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

        virtual nlohmann::json toJSON() override {
            nlohmann::json config = Amara::Node::toJSON();
            
            config["wrapWidth"] = wrapWidth;
            config["alignment"] = static_cast<int>(alignment);
            
            if (font) config["font"] = font->key;
            if (!text.empty()) config["text"] = text;

            config["originX"] = origin.x;
            config["originY"] = origin.y;

            return config;
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            Amara::Node::configure(config);
            
            if (json_has(config, "wrapWidth")) {
                setWrapWidth(config["wrapWidth"]);
            }
            if (json_has(config, "wrapMode")) {
                setWrapMode(config["wrapMode"]);
            }

            if (json_has(config, "alignment")) align(static_cast<Amara::AlignmentEnum>(config["alignment"]));
            
            if (json_has(config, "font")) setFont(config["font"]);
            if (json_has(config, "text")) setText(config["text"]);

            if (json_has(config, "originX")) origin.x = config["originX"];
            if (json_has(config, "originY")) origin.y = config["originY"];

            return this;
        }

        void updateText() {
            if (font) {
                font->packGlyphsFromString(converted_text);
                layout = font->generateLayout(converted_text, wrapWidth, wrapMode, alignment);
                textwidth = layout.width;
                textheight = layout.height;
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

        sol::object setWrapWidth(int width) {
            wrapWidth = width;
            updateText();
            return get_lua_object();
        }
        sol::object setWrapMode(Amara::WrapModeEnum _mode) {
            wrapMode = _mode;
            updateText();
            return get_lua_object();
        }
        sol::object setWrap(int width, Amara::WrapModeEnum _mode) {
            wrapMode = _mode;
            return setWrapWidth(width);
        }

        sol::object setOrigin(float _x, float _y) {
            origin = { _x, _y };
            return get_lua_object();
        }
        sol::object setOrigin(float _o) {
            return setOrigin(_o, _o);
        }

        sol::object align(AlignmentEnum _alignment) {
            alignment = _alignment;
            updateText();
            return get_lua_object();
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

            int count = 0;

            for (const TextLine& line : layout.lines) {
                for (const Glyph& glyph : line.glyphs) {
                    Vector3 glyphPos = Vector3(
                        rotateAroundAnchor(
                            anchoredPos, 
                            Vector2( 
                                anchoredPos.x + (line.x + glyph.x - layout.width*origin.x)*passOn.scale.x*scale.x,
                                anchoredPos.y + (line.y + glyph.y - layout.height*origin.y)*passOn.scale.y*scale.y
                            ),
                            passOn.rotation + rotation
                        ),
                        anchoredPos.z
                    );

                    srcRect.x = glyph.src.x;
                    srcRect.y = glyph.src.y;
                    srcRect.w = glyph.src.w;
                    srcRect.h = glyph.src.h;
    
                    dim = {
                        glyphPos.x,
                        glyphPos.y - glyphPos.z,
                        glyph.src.w*scale.x*passOn.scale.x,
                        glyph.src.h*scale.y*passOn.scale.y
                    };
    
                    destRect.x = vcenter.x + dim.x*passOn.zoom.x;
                    destRect.y = vcenter.y + dim.y*passOn.zoom.y;
                    destRect.w = dim.w * passOn.zoom.x;
                    destRect.h = dim.h * passOn.zoom.y;

                    if (font->glTextureID != 0 && Props::glContext != NULL) {
                        Quad srcQuad = Quad(
                            { srcRect.x/font->atlasWidth, srcRect.y/font->atlasHeight },
                            { (srcRect.x+srcRect.w)/font->atlasWidth, srcRect.y/font->atlasHeight },
                            { (srcRect.x+srcRect.w)/font->atlasWidth, (srcRect.y+srcRect.h)/font->atlasHeight },
                            { srcRect.x/font->atlasWidth, (srcRect.y+srcRect.h)/font->atlasHeight }
                        );
                        Quad destQuad = glTranslateQuad(v, rotateQuad(
                            Quad(destRect),
                            Vector2(
                                destRect.x + dorigin.x,
                                destRect.y + dorigin.y
                            ),
                            passOn.rotation + rotation
                        ));

                        vertices = {
                            destQuad.p1.x, destQuad.p1.y, srcQuad.p1.x, srcQuad.p1.y,
                            destQuad.p2.x, destQuad.p2.y, srcQuad.p2.x, srcQuad.p2.y,
                            destQuad.p3.x, destQuad.p3.y, srcQuad.p3.x, srcQuad.p3.y,
                            destQuad.p4.x, destQuad.p4.y, srcQuad.p4.x, srcQuad.p4.y
                        };

                        Props::renderBatch->pushQuad(
                            Props::currentShaderProgram,
                            font->glTextureID,
                            vertices,
                            v, 
                            blendMode
                        );
                    }
                    // else if (font->texture && Props::renderer) {
                    //     SDL_SetRenderViewport(Props::renderer, &v);
                    //     SDL_SetTextureScaleMode(font->texture, SDL_SCALEMODE_NEAREST);
    
                    //     SDL_RenderTextureRotated(
                    //         Props::renderer, 
                    //         font->texture,
                    //         &srcRect,
                    //         &destRect,
                    //         getDegrees(passOn.rotation + rotation),
                    //         &dorigin,
                    //         SDL_FLIP_NONE
                    //     );
                    // }

                    count += 1;
                    if (count >= progress) {
                        return;
                    }
                }
            }
        }

        int length() {
            return converted_text.size();
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Text>("Text",
                sol::base_classes, sol::bases<Node>(),
                "w", sol::readonly(&Text::textwidth),
                "h", sol::readonly(&Text::textheight),
                "text", sol::readonly(&Text::text),
                "setText", &Text::setText,
                "setFont", &Text::setFont,
                "progress", &Text::progress,
                "length", sol::property(&Text::length),
                "origin", &Text::origin,
                "setOrigin", sol::overload(
                    sol::resolve<sol::object(float, float)>(&Text::setOrigin),
                    sol::resolve<sol::object(float)>(&Text::setOrigin)
                ),
                "align", &Text::align,
                "alignment", sol::readonly(&Text::alignment),
                "setWrapWidth", &Text::setWrapWidth,
                "setWrapMode", &Text::setWrapMode,
                "setWrap", &Text::setWrap
            );
        }
    };
}