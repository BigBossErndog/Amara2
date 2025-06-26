namespace Amara {
    class Text;

    struct Text_TempProps {
        float offsetX = 0;
        float offsetY = 0;

        float alpha = 1;

        Amara::Color color = Amara::Color::White;
        Amara::BlendMode blendMode = Amara::BlendMode::Alpha;

        sol::protected_function manipulator = sol::nil;

        void set(Amara::Text* text);
    };

    class Text: public Amara::Node {
    public:
        std::string text;
        std::u32string converted_text;
        
        Amara::FontAsset* font = nullptr;
        
        int progress = 0;
        
        Amara::BlendMode blendMode = Amara::BlendMode::Alpha;
        Amara::Color tint = Amara::Color::White;

        Amara::AlignmentEnum alignment = Amara::AlignmentEnum::Left;
        
        Vector2 origin = { 0.5, 0.5 };

        int textWidth = 0;
        int textHeight = 0;

        double wrapWidth = -1;
        Amara::WrapModeEnum wrapMode = Amara::WrapModeEnum::ByCharacter;

        int lineSpacing = 0;

        TextLayout layout;

        Text_TempProps temp_props;
        bool in_drawing = false;

        std::unordered_map<std::string, sol::protected_function> manipulators;
        sol::protected_function currentManipulator;

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

            config["tint"] = tint.toJSON();
            config["blendMode"] = static_cast<int>(blendMode);

            config["tint"] = tint.toJSON();
            config["blendMode"] = static_cast<int>(blendMode);
            
            config["wrapWidth"] = wrapWidth;
            config["wrapMode"] = wrapMode;

            config["lineSpacing"] = lineSpacing;

            config["alignment"] = static_cast<int>(alignment);
            
            if (font) config["font"] = font->key;
            if (!text.empty()) config["text"] = text;

            config["originX"] = origin.x;
            config["originY"] = origin.y;

            return config;
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            Amara::Node::configure(config);

            if (in_drawing) {
                if (config.is_string()) {
                    std::string manipulator_name = config;
                    
                    if (String::equal(manipulator_name, "end")) {
                        temp_props.set(this);
                    }
                    else if (manipulators.find(manipulator_name) != manipulators.end()) {
                        temp_props.manipulator = manipulators[manipulator_name];
                    }
                    else if (Amara::Color::isColor(manipulator_name)) {
                        temp_props.color = nlohmann::json(manipulator_name);
                    }
                    else {
                        debug_log("Error: Text Manipulator \"", manipulator_name, "\" was not found.");
                        gameProps->breakWorld();
                    }
                }
                else if (json_is(config, "end")) {
                    temp_props.set(this);
                }
                else {
                    if (json_has(config, "offsetX")) temp_props.offsetX = config["offsetX"];
                    if (json_has(config, "offsetY")) temp_props.offsetY = config["offsetY"];
                    if (json_has(config, "alpha")) temp_props.alpha = config["alpha"];
                    if (json_has(config, "tint")) temp_props.color = config["tint"];
                    if (json_has(config, "color")) temp_props.color = config["color"];
                    if (json_has(config, "blendMode")) temp_props.blendMode = static_cast<Amara::BlendMode>(config["blendMode"].get<int>());
                    if (json_has(config, "manipulator")) {
                        std::string manipulator_name = config["manipulator"];
                        if (manipulators.find(manipulator_name) != manipulators.end()) {
                            temp_props.manipulator = manipulators[manipulator_name];
                        }
                        else {
                            debug_log("Error: Text Manipulator \"", manipulator_name, "\" was not found.");
                            gameProps->breakWorld();
                        }
                    }
                }
                return this;
            }

            if (config.is_string()) setManipulator(config);
            
            if (json_has(config, "tint")) tint = config["tint"];
            if (json_has(config, "color")) tint = config["color"];
            if (json_has(config, "blendMode")) blendMode = static_cast<Amara::BlendMode>(config["blendMode"].get<int>());
            
            if (json_has(config, "wrapWidth")) {
                setWrapWidth(config["wrapWidth"]);
            }
            if (json_has(config, "wrapMode")) {
                setWrapMode(config["wrapMode"]);
            }

            if (json_has(config, "lineSpacing")) setLineSpacing(config["lineSpacing"]);

            if (json_has(config, "alignment")) align(static_cast<Amara::AlignmentEnum>(config["alignment"]));
            
            if (json_has(config, "font")) setFont(config["font"]);
            if (json_has(config, "text")) {
                if (config["text"].is_string()) setText(config["text"].get<std::string>());
                else if (config["text"].is_number()) {
                    if (config["text"].is_number_integer()) setText(std::to_string(config["text"].get<int>()));
                    else setText(std::to_string(config["text"].get<double>()));
                }
                else setText(std::string(config["text"].dump()));
            }

            if (json_has(config, "originX")) origin.x = config["originX"];
            if (json_has(config, "originY")) origin.y = config["originY"];
            if (json_has(config, "origin")) origin = config["origin"];

            return this;
        }

        void updateText() {
            if (font) {
                font->packGlyphsFromString(converted_text);
                layout = font->generateLayout(converted_text, wrapWidth, wrapMode, alignment, lineSpacing);
                textWidth = layout.width;
                textHeight = layout.height;
            }
        }

        void setText(std::string str) {
            if (String::equal(text, str)) return;
            
            text = str;

            converted_text = Amara::String::utf8_to_utf32(str);  // Convert UTF-8 string to UTF-32

            updateText();
            progress = layout.display_size();
        }

        sol::object setText(sol::variadic_args input) {
            std::ostringstream ss;
            for (auto arg : input) {
                if (arg.is<std::string>()) ss << arg.get<std::string>();
                else if (arg.is<sol::table>()) {
                    sol::object result = gameProps->lua["table"]["to_string"](arg);
                    ss << "$" << result.as<std::string>();
                }
                else ss << lua_to_string(arg);
            }
            setText(ss.str());
            return get_lua_object();
        }

        bool setFont(std::string key) {
            font = nullptr;

            if (destroyed) return false;
            
            if (!gameProps->assets->has(key)) {
                debug_log("Error: Asset \"", key, "\" was not found.");
                return false;
            }

            Amara::Asset* asset = gameProps->assets->get(key);
            if (asset) font = asset->as<Amara::FontAsset*>();

            if (font == nullptr) {
                debug_log("Error: Asset \"", key, "\" is not a valid font asset.");
                return false;
            }

            if (!converted_text.empty()) {
                updateText();
            }

            return true;
        }

        sol::object setWrapWidth(double width) {
            wrapWidth = width;
            updateText();
            return get_lua_object();
        }
        sol::object setWrapMode(Amara::WrapModeEnum _mode) {
            wrapMode = _mode;
            updateText();
            return get_lua_object();
        }

        int setLineSpacing(int _spacing) {
            lineSpacing = _spacing;
            updateText();
            return _spacing;
        }

        AlignmentEnum align(AlignmentEnum _alignment) {
            alignment = _alignment;
            updateText();
            return alignment;
        }

        void drawSelf(const Rectangle& v) override {
            if (font == nullptr || converted_text.empty()) return;

            temp_props.set(this);
            in_drawing = true;

            Vector2 vcenter = { v.w/2.0f, v.h/2.0f };
            Vector2 totalZoom = { passOn.zoom.x*passOn.window_zoom.x, passOn.zoom.y*passOn.window_zoom.y };

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
            float diag_distance;

            Rectangle dim = {
                anchoredPos.x + (textWidth*origin.x)*scale.x*passOn.scale.x, 
                anchoredPos.y - anchoredPos.z + (textHeight*origin.y)*scale.y*passOn.scale.y,
                textWidth*scale.x*passOn.scale.x,
                textHeight*scale.y*passOn.scale.y
            };
            destRect.x = vcenter.x + dim.x*totalZoom.x;
            destRect.y = vcenter.y + dim.y*totalZoom.y;
            destRect.w = dim.w * totalZoom.x;
            destRect.h = dim.h * totalZoom.y;
            SDL_FPoint dorigin = { 
                (textWidth*origin.x)*scale.x*passOn.scale.x*totalZoom.x,
                (textHeight*origin.y)*scale.y*passOn.scale.y*totalZoom.y
            };

            diag_distance = distanceBetween(0, 0, destRect.w, destRect.h);
            if (!Shape::collision(
                Rectangle(destRect), Rectangle(
                    -diag_distance, -diag_distance,
                    v.w + diag_distance*2, v.h + diag_distance*2
                )
            )) return;

            if (input.active) {
                Quad inputQuad = rotateQuad(
                    Quad(destRect),
                    Vector2(
                        destRect.x + dorigin.x,
                        destRect.y + dorigin.y
                    ),
                    passOn.rotation + rotation
                );
                input.queueInput(inputQuad);
            }

            int count = 0;

            if (font->texture && gameProps->renderer) {
                SDL_SetTextureScaleMode(font->texture, SDL_SCALEMODE_NEAREST);
            }

            for (const TextLine& line : layout.lines) {
                for (const Glyph& glyph : line.glyphs) {
                    if (count >= progress) break;

                    if (!glyph.renderable) {
                        if (glyph.is_config) {
                            configure(glyph.config);
                        }
                        continue;
                    }
                    if (temp_props.manipulator.valid()) {
                        try {
                            sol::protected_function_result result = temp_props.manipulator(count, lifeTime, (char)glyph.codepoint);
                            if (!result.valid()) {
                                sol::error err = result;
                                throw std::runtime_error("Lua Error: " + std::string(err.what()));  
                            }
                            else {
                                configure(lua_to_json(result));
                            }
                        } catch (const std::exception& e) {
                            debug_log(e.what());
                            gameProps->breakWorld();
                        }
                    }

                    Vector3 glyphPos = Vector3(
                        rotateAroundAnchor(
                            anchoredPos, 
                            Vector2( 
                                anchoredPos.x + (line.x + glyph.x + temp_props.offsetX - layout.width*origin.x)*passOn.scale.x*scale.x,
                                anchoredPos.y + (line.y + glyph.y + temp_props.offsetY - layout.height*origin.y)*passOn.scale.y*scale.y
                            ),
                            passOn.rotation + rotation
                        ),
                        anchoredPos.z
                    );

                    srcRect.x = glyph.src.x;
                    srcRect.y = glyph.src.y;
                    srcRect.w = glyph.src.w;
                    srcRect.h = glyph.src.h;
    
                    dim = Rectangle(
                        glyphPos.x,
                        glyphPos.y - glyphPos.z,
                        glyph.src.w*scale.x*passOn.scale.x,
                        glyph.src.h*scale.y*passOn.scale.y
                    );
    
                    destRect.x = vcenter.x + dim.x*totalZoom.x;
                    destRect.y = vcenter.y + dim.y*totalZoom.y;
                    destRect.w = dim.w * totalZoom.x;
                    destRect.h = dim.h * totalZoom.y;

                    diag_distance = distanceBetween(0, 0, destRect.w, destRect.h);
                    if (!Shape::collision(
                        Rectangle(destRect), Rectangle(
                            -diag_distance, -diag_distance,
                            v.w + diag_distance*2, v.h + diag_distance*2
                        )
                    )) continue;

                    if (font->texture && gameProps->renderer) {
                        SDL_SetTextureScaleMode(font->texture, SDL_SCALEMODE_NEAREST);
                        SDL_SetTextureColorMod(font->texture, temp_props.color.r, temp_props.color.g, temp_props.color.b);
                        SDL_SetTextureAlphaMod(font->texture, alpha * passOn.alpha * temp_props.alpha * 255);
                        Apply_SDL_BlendMode(gameProps, font->texture, temp_props.blendMode);

                        SDL_RenderTextureRotated(
                            gameProps->renderer, 
                            font->texture,
                            &srcRect,
                            &destRect,
                            getDegrees(passOn.rotation + rotation),
                            &dorigin,
                            SDL_FLIP_NONE
                        );
                    }
                    #ifdef AMARA_OPENGL
                    else if (font->glTextureID != 0 && gameProps->glContext != NULL) {
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
                        ), passOn.insideTextureContainer);

                        vertices = {
                            destQuad.p1.x, destQuad.p1.y, srcQuad.p1.x, srcQuad.p1.y,
                            destQuad.p2.x, destQuad.p2.y, srcQuad.p2.x, srcQuad.p2.y,
                            destQuad.p3.x, destQuad.p3.y, srcQuad.p3.x, srcQuad.p3.y,
                            destQuad.p4.x, destQuad.p4.y, srcQuad.p4.x, srcQuad.p4.y
                        };

                        gameProps->renderBatch->pushQuad(
                            gameProps->currentShaderProgram,
                            font->glTextureID,
                            vertices, passOn.alpha * alpha * temp_props.alpha,
                            temp_props.color,
                            v, passOn.insideTextureContainer,
                            temp_props.blendMode
                        );
                    }
                    #endif

                    count += 1;
                }
            }

            in_drawing = false;
        }

        int length() {
            return converted_text.size();
        }

        Rectangle getRectangle() {
            return Rectangle(
                pos.x - (textWidth*scale.x)*origin.x,
                pos.y - (textHeight*scale.y)*origin.y,
                textWidth*scale.x,
                textHeight*scale.y
            );
        }
        Rectangle stretchTo(const Rectangle& rect) {
            pos.x = rect.x + rect.w*origin.x;
            pos.y = rect.y + rect.h*origin.y;
            scale.x = rect.w / static_cast<float>(textWidth);
            scale.y = rect.h / static_cast<float>(textHeight);
            return rect;
        }
        
        sol::object fitWithin(const Rectangle& rect) {
            if (rect.w == 0 || rect.h == 0) return get_lua_object();

            rotation = 0;

            float horFactor = rect.w / static_cast<float>(textWidth);
            float verFactor = rect.h / static_cast<float>(textHeight);
            
            if (horFactor < verFactor) {
                scale.x = horFactor;
                scale.y = horFactor;
            }
            else {
                scale.x = verFactor;
                scale.y = verFactor;
            }

            float scaledWidth  = textWidth  * scale.x;
            float scaledHeight = textHeight * scale.y;

            pos.x = rect.x + (rect.w - scaledWidth)/2 + scaledWidth*origin.x;
            pos.y = rect.y + (rect.h - scaledHeight)/2 + scaledHeight*origin.y;
            
            return get_lua_object();
        }

        Vector2 getCenter() {
            return getRectangle().getCenter();
        }

        void resetProgress() {
            progress = 0;
        }
        bool progressText(int speed) {
            progress += speed;
            if (progress >= layout.display_size()) {
                progress = layout.display_size();
                return true;
            }
            return false;
        }
        
        sol::object autoProgress(double speed); // speed = characters per second
        sol::object autoProgress(sol::table sol_config);
        sol::object skipProgress();
        
        float setWidth(float _w) {
            scale.x = _w / static_cast<float>(textWidth);
        }
        float setHeight(float _h) {
            scale.y = _h / static_cast<float>(textHeight);
        }

        sol::object setManipulator(sol::function manipulator) {
            currentManipulator = manipulator;
            return get_lua_object();
        }

        sol::object setManipulator(std::string manipulator_name, sol::function manipulator) {
            manipulators[manipulator_name] = manipulator;
            return get_lua_object();
        }

        sol::object setManipulator(std::string manipulator_name) {
            if (manipulators.find(manipulator_name) != manipulators.end()) {
                currentManipulator = manipulators[manipulator_name];
            }
            else {
                debug_log("Error: Text Manipulator \"", manipulator_name, "\" was not found.");
                gameProps->breakWorld();
            }
            return get_lua_object();
        }

        sol::object removeManipulator() {
            currentManipulator = sol::nil;
            return get_lua_object();
        }

        sol::object removeManipulator(std::string manipulator_name) {
            if (manipulators.find(manipulator_name) != manipulators.end()) {
                manipulators.erase(manipulator_name);
            }
            return get_lua_object();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Text>("Text",
                sol::base_classes, sol::bases<Node>(),
                "tint", sol::property([](Amara::Text& t) -> Amara::Color { return t.tint; }, [](Amara::Text& t, sol::object v) { t.tint = v; }),
                "color", sol::property([](Amara::Text& t) -> Amara::Color { return t.tint; }, [](Amara::Text& t, sol::object v) { t.tint = v; }),
                "blendMode", &Text::blendMode,
                "w", sol::property([](Amara::Text& t) -> float { return t.textWidth; }, &Text::setWidth),
                "h", sol::property([](Amara::Text& t) -> float { return t.textHeight; }, &Text::setHeight),
                "width", sol::property([](Amara::Text& t) -> float { return t.textWidth; }, &Text::setWidth),
                "height", sol::property([](Amara::Text& t) -> float { return t.textHeight; }, &Text::setHeight),
                "rect", sol::property(&Text::getRectangle, &Text::stretchTo),
                "stretchTo", &Text::stretchTo,
                "fitWithin", &Text::fitWithin,
                "center", sol::property(&Text::getCenter),
                "length", sol::property(&Text::length),
                "text", sol::property(
                    [](Amara::Text& t) -> std::string { return t.text; },
                    [](Amara::Text& t, sol::object v) {
                        if (v.is<std::string>()) t.setText(v.as<std::string>());
                        else {
                            t.setText(lua_to_string(v));
                        }
                        return t.text;
                    }
                ),
                "setText", sol::resolve<sol::object(sol::variadic_args)>(&Text::setText),
                "font", sol::property(
                    [](Amara::Text& t) -> std::string { return t.font->key; },
                    [](Amara::Text& t, std::string key) { 
                        if (t.setFont(key)) return t.font->key;
                        else return std::string("");
                    }
                ),
                "length", sol::property(&Text::length),
                "origin", &Text::origin,
                "originX", sol::property([](Amara::Text& t) -> float { return t.origin.x; }, [](Amara::Text& t, float v) { t.origin.x = v; }),
                "originY", sol::property([](Amara::Text& t) -> float { return t.origin.y; }, [](Amara::Text& t, float v) { t.origin.y = v; }),
                "alignment", sol::property([](Amara::Text& t) -> int { return static_cast<int>(t.alignment); }, [](Amara::Text& t, int v) { t.align(static_cast<Amara::AlignmentEnum>(v)); }),
                "lineSpacing", sol::property([](Amara::Text& t) -> int { return t.lineSpacing; }, [](Amara::Text& t, sol::object v) { 
                    if (v.is<int>()) t.setLineSpacing(v.as<int>());
                    else if (v.is<double>()) t.setLineSpacing(floor(v.as<double>())); 
                }),
                "wrapMode", sol::property([](Amara::Text& t) -> int { return static_cast<int>(t.wrapMode); }, [](Amara::Text& t, int v) { t.setWrapMode(static_cast<Amara::WrapModeEnum>(v)); }),
                "wrapWidth", sol::property([](Amara::Text& t) -> int { return t.wrapWidth; }, [](Amara::Text& t, sol::object v) {
                    if (v.is<int>()) t.setWrapWidth(v.as<int>());
                    else if (v.is<double>()) t.setWrapWidth(floor(v.as<double>()));
                }),
                "progress", &Text::progress,
                "progressText", &Text::progressText,
                "resetProgress", &Text::resetProgress,
                "setManipulator", sol::overload(
                    sol::resolve<sol::object(sol::function)>(&Text::setManipulator),
                    sol::resolve<sol::object(std::string, sol::function)>(&Text::setManipulator),
                    sol::resolve<sol::object(std::string)>(&Text::setManipulator)
                ),
                "removeManipulator", sol::overload(
                    sol::resolve<sol::object()>(&Text::removeManipulator),
                    sol::resolve<sol::object(std::string)>(&Text::removeManipulator)
                ),
                "manipulator", &Text::currentManipulator,
                "autoProgress", sol::overload(
                    sol::resolve<sol::object(double)>(&Text::autoProgress),
                    sol::resolve<sol::object(sol::table)>(&Text::autoProgress)
                ),
                "skipProgress", &Text::skipProgress
            );
        }
    };

    void Text_TempProps::set(Amara::Text* text) {
        offsetX = 0;
        offsetY = 0;
        alpha = 1;

        color = text->tint;
        blendMode = text->blendMode;

        manipulator = text->currentManipulator;
    }
}