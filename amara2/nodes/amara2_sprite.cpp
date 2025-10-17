namespace Amara {
    class Animation;

    /**
     * This Sprite class is used for 2D sprites.
     */
    class Sprite: public Amara::Node {
    public:
        ImageAsset* image = nullptr;
        SpritesheetAsset* spritesheet = nullptr;

        Amara::BlendMode blendMode = Amara::BlendMode::Alpha;
        Amara::Color tint = Amara::Color::White;

        Vector2 origin = { 0.5, 0.5 };
        
        int textureWidth = 0;
        int textureHeight = 0;
        
        int frameWidth = 0;
        int frameHeight = 0;

        int cropLeft = 0;
        int cropRight = 0;
        int cropTop = 0;
        int cropBottom = 0;

        int frame = 1;

        bool renderPixelPerfect = false;

        #ifdef AMARA_OPENGL
        std::array<float, 16> vertices = {
            -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
             0.5f, -0.5f,  1.0f, 0.0f, // Bottom-right
             0.5f,  0.5f,  1.0f, 1.0f, // Top-right
            -0.5f,  0.5f,  0.0f, 1.0f  // Top-left
        };
        #endif

        Sprite(): Amara::Node() {
            set_base_node_id("Sprite");
        }
        
        virtual bool setTexture(std::string key) {
            if (image && image->temp) {
                delete image;
            }

            image = nullptr;
            spritesheet = nullptr;

            textureWidth = 0;
            textureHeight = 0;

            frameWidth = 0;
            frameHeight = 0;

            if (destroyed || key.empty()) return false;

            if (!gameProps->assets->has(key)) {
                fatal_error("Error: Asset \"", key, "\" was not found.");
                return false;
            }

            Amara::Asset* asset = gameProps->assets->get(key);
            if (asset) image = asset->as<ImageAsset*>();
            
            if (image == nullptr) {
                fatal_error("Error: Asset \"", key, "\" is not a valid texture asset.");
                gameProps->breakWorld();
                return false;
            }
            textureWidth = image->width;
            textureHeight = image->height;

            spritesheet = image->as<SpritesheetAsset*>();
            if (spritesheet) {
                frameWidth = spritesheet->frameWidth;
                frameHeight = spritesheet->frameHeight;
            }
            else {
                frameWidth = 0;
                frameHeight = 0;
            }
            return true;
        }

        virtual bool setTempTexture(std::string path, bool isSpritesheet, int frameWidth, int frameHeight) {
            if (image && image->temp) {
                delete image;
            }

            image = nullptr;
            spritesheet = nullptr;

            textureWidth = 0;
            textureHeight = 0;

            frameWidth = 0;
            frameHeight = 0;
            
            if (destroyed || path.empty()) return false;
            
            if (isSpritesheet) {
                spritesheet = new SpritesheetAsset(gameProps);
                spritesheet->loadSpritesheet(path, frameWidth, frameHeight);
                image = spritesheet;
            }
            else {
                image = new ImageAsset(gameProps);
                image->loadImage(path);
            }

            textureWidth = image->width;
            textureHeight = image->height;
            
            spritesheet = image->as<SpritesheetAsset*>();
            if (spritesheet) {
                frameWidth = spritesheet->frameWidth;
                frameHeight = spritesheet->frameHeight;
            }
            else {
                frameWidth = 0;
                frameHeight = 0;
            }

            image->key = "temp";
            image->temp = true;

            return true;
        }
        bool setTempTexture(nlohmann::json temp_data) {
            if (temp_data.is_string()) {
                return setTempTexture(temp_data.get<std::string>(), false, 0, 0);
            }
            else if (temp_data.is_object()) {
                if (json_has(temp_data, "path")) {
                    if (json_has_any(temp_data, "width", "height", "w", "h")) {
                        int width = 0;
                        int height = 0;
                        if (json_has(temp_data, "width")) width = temp_data["width"].get<int>();
                        if (json_has(temp_data, "height")) height = temp_data["height"].get<int>();
                        if (json_has(temp_data, "w")) width = temp_data["w"].get<int>();
                        if (json_has(temp_data, "h")) height = temp_data["h"].get<int>();
                        return setTempTexture(temp_data["path"].get<std::string>(), true, width, height);
                    }
                    else {
                        return setTempTexture(temp_data["path"].get<std::string>(), false, 0, 0);
                    }
                }
            }
            fatal_error("Error: Invalid temp image data.");
            return false;
        }

        bool lua_setTempTexture(sol::object v) {
            return setTempTexture(lua_to_json(v));
        }

        virtual nlohmann::json toJSON() override {
            nlohmann::json data = Amara::Node::toJSON();
            
            if (image) {
                data["texture"] = image->key;
            }
            data["frame"] = frame;
            
            data["tint"] = tint.toJSON();
            data["blendMode"] = static_cast<int>(blendMode);
            
            data["originX"] = origin.x;
            data["originY"] = origin.y;

            data["cropLeft"] = cropLeft;
            data["cropRight"] = cropRight;
            data["cropTop"] = cropTop;
            data["cropBottom"] = cropBottom;

            data["renderPixelPerfect"] = renderPixelPerfect;

            return data;
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "tint")) tint = config["tint"];
            if (json_has(config, "blendMode")) blendMode = static_cast<Amara::BlendMode>(config["blendMode"].get<int>());

            if (json_has(config, "texture")) setTexture(config["texture"]);
            if (json_has(config, "tempTexture")) setTempTexture(config["tempTexture"]);
            
            if (json_has(config, "frame")) frame = config["frame"];
            if (json_has(config, "animation")) animate(config["animation"]);

            if (json_has(config, "originX")) origin.x = config["originX"];
            if (json_has(config, "originY")) origin.y = config["originY"];
            if (json_has(config, "origin")) origin = config["origin"];

            if (json_has(config, "originPosition")) {
                origin = Vector2(config["originPosition"]) / Vector2(
                    (spritesheet ? frameWidth : textureWidth), 
                    (spritesheet ? frameHeight : textureHeight)
                );
            }
            if (json_has(config, "originPositionX")) {
                origin.x = config["originPositionX"].get<float>() / (spritesheet ? frameWidth : textureWidth);
            }
            if (json_has(config, "originPositionY")) {
                origin.y = config["originPositionY"].get<float>() / (spritesheet ? frameHeight : textureHeight);
            }
            
            if (json_has(config, "cropLeft")) cropLeft = config["cropLeft"];
            if (json_has(config, "cropRight")) cropRight = config["cropRight"];
            if (json_has(config, "cropTop")) cropTop = config["cropTop"];
            if (json_has(config, "cropBottom")) cropBottom = config["cropBottom"];

            if (json_has(config, "width")) setWidth(config["width"]);
            if (json_has(config, "height")) setHeight(config["height"]);

            if (json_has(config, "rect")) stretchTo(config["rect"]);
            if (json_has(config, "size")) stretchTo(config["size"]);
            
            if (json_has(config, "renderPixelPerfect")) renderPixelPerfect = config["renderPixelPerfect"];

            return Amara::Node::configure(config);
        }

        Amara::Action* animate(nlohmann::json config);

        sol::object animate(sol::object config) {
            Amara::Action* anim = animate(lua_to_json(config));
            if (anim) return anim->get_lua_object();
            return sol::nil;
        }
        sol::object stopAnimating() {
            for (Amara::Node* node: children) {
                if (node->is_animation && !node->destroyed) {
                    node->destroy();
                }
            }
            return get_lua_object();
        }

        virtual void drawSelf(const Rectangle& v) override {
            if (image == nullptr) return;

            if (cropLeft < 0) cropLeft = 0;
            if (cropRight < 0) cropRight = 0;
            if (cropTop < 0) cropTop = 0;
            if (cropBottom < 0) cropBottom = 0;

            Vector2 vcenter = { v.w/2.0f, v.h/2.0f };
            Vector2 totalZoom = { passOn.zoom.x*passOn.window_zoom.x, passOn.zoom.y*passOn.window_zoom.y };

            float imgw = (spritesheet ? frameWidth : textureWidth);
            float imgh = (spritesheet ? frameHeight : textureHeight);

            Vector2 render_pos = (renderPixelPerfect) ? pos.round() : pos;

            Vector3 anchoredPos = Vector3(
                rotateAroundAnchor(
                    passOn.anchor, 
                    Vector2( 
                        (passOn.anchor.x + render_pos.x*passOn.scale.x), 
                        (passOn.anchor.y + render_pos.y*passOn.scale.y)
                    ),
                    passOn.rotation
                ),
                passOn.anchor.z + pos.z
            );

            SDL_FRect srcRect, destRect;

            if (spritesheet) {
                int maxFrames = (int)floor(((float)image->width / (float)spritesheet->frameWidth) * ((float)image->height / (float)spritesheet->frameHeight));
                int fixedFrame = ((frame - 1) % maxFrames);
                while (fixedFrame < 0) fixedFrame += maxFrames;
                srcRect.x = static_cast<float>((fixedFrame % (textureWidth / frameWidth)) * frameWidth + cropLeft);
                srcRect.y = static_cast<float>(floor(fixedFrame / (textureWidth / frameWidth)) * frameHeight + cropTop);
                srcRect.w = static_cast<float>(frameWidth - cropLeft - cropRight);
                srcRect.h = static_cast<float>(frameHeight - cropTop - cropBottom);
            }
            else {
                srcRect = {
                    static_cast<float>(cropLeft),
                    static_cast<float>(cropTop),
                    static_cast<float>(textureWidth - cropLeft - cropRight),
                    static_cast<float>(textureHeight - cropTop - cropBottom)
                };
            }

            Rectangle dim = {
                anchoredPos.x + (cropLeft - imgw*origin.x)*scale.x*passOn.scale.x, 
                anchoredPos.y - anchoredPos.z + (cropTop - imgh*origin.y)*scale.y*passOn.scale.y,
                (imgw - cropLeft - cropRight)*scale.x*passOn.scale.x,
                (imgh - cropTop - cropBottom)*scale.y*passOn.scale.y
            };

            destRect.x = vcenter.x + dim.x*totalZoom.x;
            destRect.y = vcenter.y + dim.y*totalZoom.y;
            destRect.w = dim.w * totalZoom.x;
            destRect.h = dim.h * totalZoom.y;

            SDL_FPoint dorigin = {
                (imgw*origin.x - cropLeft)*scale.x*passOn.scale.x*totalZoom.x,
                (imgh*origin.y - cropTop)*scale.y*passOn.scale.y*totalZoom.y
            };

            float diag_distance = distanceBetween(0, 0, destRect.w, destRect.h);
            if (!Shape::checkCollision(
                Rectangle(destRect), Rectangle(
                    -diag_distance, -diag_distance,
                    v.w + diag_distance*2, v.h + diag_distance*2
                )
            )) return;

            if (input.active && !passOn.insideTextureContainer) {
                Quad inputQuad = rotateQuad(
                    Quad(destRect),
                    Vector2(
                        destRect.x + dorigin.x,
                        destRect.y + dorigin.y
                    ),
                    passOn.rotation + rotation
                );
                input.queueInput(moveQuad(inputQuad, v.x, v.y), v, nullptr);
            }

            if (image->texture && gameProps->renderer) {
                // 2D Rendering
                SDL_SetTextureScaleMode(image->texture, SDL_SCALEMODE_NEAREST);
                SDL_SetTextureColorMod(image->texture, tint.r, tint.g, tint.b);
                SDL_SetTextureAlphaMod(image->texture, alpha * passOn.alpha * 255);
                Apply_SDL_BlendMode(gameProps, image->texture, blendMode);

                SDL_RenderTextureRotated(
                    gameProps->renderer, 
                    image->texture,
                    &srcRect,
                    &destRect,
                    getDegrees(passOn.rotation + rotation),
                    &dorigin,
                    SDL_FLIP_NONE
                );
            }
            else if (image->gpuTexture && gameProps->gpuDevice) {
                // GPU Rendering
            }
            #ifdef AMARA_OPENGL
            else if (image->glTextureID != 0 && gameProps->glContext != NULL) {
                Quad srcQuad = Quad(
                    { srcRect.x/textureWidth, srcRect.y/textureHeight },
                    { (srcRect.x+srcRect.w)/textureWidth, srcRect.y/textureHeight },
                    { (srcRect.x+srcRect.w)/textureWidth, (srcRect.y+srcRect.h)/textureHeight },
                    { srcRect.x/textureWidth, (srcRect.y+srcRect.h)/textureHeight }
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
                    image->glTextureID,
                    vertices, passOn.alpha * alpha, tint,
                    v, passOn.insideTextureContainer,
                    blendMode
                );
            }
            #endif
        }

        float getWidth() {
            if (spritesheet) return frameWidth*scale.x;
            if (image) return textureWidth*scale.x;
            return 0;
        }
        float getHeight() {
            if (spritesheet) return frameHeight*scale.y;
            if (image) return textureHeight*scale.y;
            return 0;
        }
        virtual float setWidth(float _w) {
            float cw = (spritesheet ? frameWidth : textureWidth);
            if (cw == 0) return scale.x = 0;
            return scale.x = _w / cw;
        }
        virtual float setHeight(float _h) {
            float ch = (spritesheet ? frameHeight : textureHeight);
            if (ch == 0) return scale.y = 0;
            return scale.y = _h / ch;
        }

        Rectangle getRectangle() {
            return Rectangle(
                pos.x - getWidth()*origin.x,
                pos.y - getHeight()*origin.y,
                getWidth(),
                getHeight()
            );
        }

        virtual Shape getShape() override {
            return Quad(rotateQuad(
                Quad(Rectangle(
                    -getWidth()*origin.x,
                    -getHeight()*origin.y,
                    getWidth(),
                    getHeight()
                )),
                Vector2(0, 0),
                rotation
            ));
        }

        virtual Shape getCollisionShape() override {
            return Quad(rotateQuad(
                Quad(getRectangle()),
                Vector2(pos.x, pos.y),
                rotation
            ));
        }

        Rectangle stretchTo(const Rectangle& rect) {
            pos.x = rect.x + rect.w*origin.x;
            pos.y = rect.y + rect.h*origin.y;
            setWidth(rect.w);
            setHeight(rect.h);
            return rect;
        }

        sol::object fitWithin(const Rectangle& rect) {
            if (rect.w == 0 || rect.h == 0) return get_lua_object();

            rotation = 0;

            float width = getWidth()/scale.x;
            float height = getHeight()/scale.y;

            float horFactor = rect.w / width;
            float verFactor = rect.h / height;
            
            if (horFactor < verFactor) {
                scale.x = horFactor;
                scale.y = horFactor;
            }
            else {
                scale.x = verFactor;
                scale.y = verFactor;
            }

            float scaledWidth  = width  * scale.x;
            float scaledHeight = height * scale.y;

            pos.x = rect.x + (rect.w - scaledWidth)/2 + scaledWidth*origin.x;
            pos.y = rect.y + (rect.h - scaledHeight)/2 + scaledHeight*origin.y;
            
            return get_lua_object();
        }

        Vector2 getCenter() {
            return getRectangle().getCenter();
        }

        virtual void destroy() override {
            if (image && image->temp) {
                delete image;
                image = nullptr;
            }
            Amara::Node::destroy();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Sprite>("Sprite",
                sol::base_classes, sol::bases<Node>(),
                "setTexture", &Sprite::setTexture,
                "texture", sol::property(
                    [](Amara::Sprite& t) -> sol::object {
                        return t.image ?
                            sol::make_object(t.gameProps->lua, t.image->key) : 
                            sol::make_object(t.gameProps->lua, sol::nil); 
                    },
                    [](Amara::Sprite& t, std::string v) { t.setTexture(v); }
                ),
                "tempTexture", sol::property(
                    [](Amara::Sprite& t) -> sol::object {
                        return t.image ?
                            sol::make_object(t.gameProps->lua, t.image->key) : 
                            sol::make_object(t.gameProps->lua, sol::nil); 
                    },
                    [](Amara::Sprite& t, sol::object v) { t.lua_setTempTexture(v); }
                ),
                "tint", sol::property([](Amara::Sprite& t) -> Amara::Color { return t.tint; }, [](Amara::Sprite& t, sol::object v) { t.tint = v; }),
                "blendMode", &Sprite::blendMode,
                "frame", &Sprite::frame,
                "animate",  sol::resolve<sol::object(sol::object)>(&Sprite::animate),
                "stopAnimating", &Sprite::stopAnimating,
                "textureWidth", sol::readonly(&Sprite::textureWidth),
                "textureHeight", sol::readonly(&Sprite::textureHeight),
                "frameWidth", sol::readonly(&Sprite::frameWidth),
                "frameHeight", sol::readonly(&Sprite::frameHeight),
                "cropLeft", sol::property([](Amara::Sprite& t) -> int { return t.cropLeft; }, [](Amara::Sprite& t, sol::object v) {
                    if (v.is<int>()) t.cropLeft = v.as<int>();
                    else if (v.is<double>()) t.cropLeft = static_cast<int>(round(v.as<double>()));
                }),
                "cropRight", sol::property([](Amara::Sprite& t) -> int { return t.cropRight; }, [](Amara::Sprite& t, sol::object v) {
                    if (v.is<int>()) t.cropRight = v.as<int>();
                    else if (v.is<double>()) t.cropRight = static_cast<int>(round(v.as<double>()));
                }),
                "cropTop", sol::property([](Amara::Sprite& t) -> int { return t.cropTop; }, [](Amara::Sprite& t, sol::object v) {
                    if (v.is<int>()) t.cropTop = v.as<int>();
                    else if (v.is<double>()) t.cropTop = static_cast<int>(round(v.as<double>()));
                }),
                "cropBottom", sol::property([](Amara::Sprite& t) -> int { return t.cropBottom; }, [](Amara::Sprite& t, sol::object v) {
                    if (v.is<int>()) t.cropBottom = v.as<int>();
                    else if (v.is<double>()) t.cropBottom = static_cast<int>(round(v.as<double>()));
                }),
                "origin", sol::property([](Amara::Sprite& t) -> Vector2& { return t.origin; }, [](Amara::Sprite& t, sol::object v) { t.origin = v; }),
                "originX", sol::property([](Amara::Sprite& t) -> float { return t.origin.x; }, [](Amara::Sprite& t, float v) { t.origin.x = v; }),
                "originY", sol::property([](Amara::Sprite& t) -> float { return t.origin.y; }, [](Amara::Sprite& t, float v) { t.origin.y = v; }),
                "originPosition", sol::property(
                    [](Amara::Sprite& t) -> Vector2 { return Vector2(
                        (t.spritesheet ? t.frameWidth : t.textureWidth) * t.origin.x,
                        (t.spritesheet ? t.frameHeight : t.textureHeight) * t.origin.y
                    ); },
                    [](Amara::Sprite& t, sol::object v) {
                        Vector2 vec = v;
                        t.origin = vec / Vector2(
                            (t.spritesheet ? t.frameWidth : t.textureWidth),
                            (t.spritesheet ? t.frameHeight : t.textureHeight)
                        );
                    }
                ),
                "originPositionX", sol::property(
                    [](Amara::Sprite& t) -> float { return (t.spritesheet ? t.frameWidth : t.textureWidth) * t.origin.x; },
                    [](Amara::Sprite& t, float v) {
                        float w = (t.spritesheet ? t.frameWidth : t.textureWidth);
                        if (w != 0) t.origin.x = v / w;
                    }
                ),
                "originPositionY", sol::property(
                    [](Amara::Sprite& t) -> float { return (t.spritesheet ? t.frameHeight : t.textureHeight) * t.origin.y; },
                    [](Amara::Sprite& t, float v) {
                        float h = (t.spritesheet ? t.frameHeight : t.textureHeight);
                        if (h != 0) t.origin.y = v / h;
                    }
                ),
                "w", sol::property(&Sprite::getWidth),
                "h", sol::property(&Sprite::getHeight),
                "width", sol::property(&Sprite::getWidth, &Sprite::setWidth),
                "height", sol::property(&Sprite::getHeight, &Sprite::setHeight),
                "rect", sol::property(&Sprite::getRectangle, [](Amara::Sprite& t, sol::object v) {
                    Rectangle r = v;
                    t.stretchTo(r);
                }),
                "size", sol::property(&Sprite::getRectangle, [](Amara::Sprite& t, sol::object v) {
                    Rectangle r = v;
                    t.stretchTo(r);
                }),
                "stretchTo", [](Amara::Sprite& t, sol::object v) {
                    Rectangle r = v;
                    t.stretchTo(r);
                },
                "fitWithin", [](Amara::Sprite& t, sol::object v) {
                    Rectangle r = v;
                    t.fitWithin(r);
                },
                "center", sol::property(&Sprite::getCenter),
                "renderPixelPerfect", &Sprite::renderPixelPerfect
            );
        }
    };
}