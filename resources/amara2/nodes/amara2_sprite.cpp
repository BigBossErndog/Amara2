namespace Amara {
    class Animation;
    /**
     * This is used for 2D sprites.
     */
    class Sprite: public Amara::Node {
    public:
        ImageAsset* image = nullptr;
        SpritesheetAsset* spritesheet = nullptr;

        Amara::BlendMode blendMode = Amara::BlendMode::Alpha;
        Amara::Color tint = Amara::Color::White;

        Vector2 origin = { 0.5, 0.5 };
        
        int imageWidth = 0;
        int imageHeight = 0;
        
        int frameWidth = 0;
        int frameHeight = 0;

        int cropLeft = 0;
        int cropRight = 0;
        int cropTop = 0;
        int cropBottom = 0;

        int frame = 0;

        Animation* animation = nullptr;

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

        bool setTexture(std::string key) {
            image = nullptr;
            spritesheet = nullptr;

            if (!Props::assets->has(key)) {
                debug_log("Error: Asset \"", key, "\" was not found.");
                return false;
            }

            image = Props::assets->get(key)->as<ImageAsset*>();
            
            if (image == nullptr) {
                debug_log("Error: Asset \"", key, "\" is not a valid texture asset.");
                return false;
            }
            imageWidth = image->width;
            imageHeight = image->height;

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

            return data;
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            Amara::Node::configure(config);

            if (json_has(config, "tint")) tint = config["tint"];
            if (json_has(config, "blendMode")) blendMode = static_cast<Amara::BlendMode>(config["blendMode"].get<int>());

            if (json_has(config, "texture")) setTexture(config["texture"]);
            if (json_has(config, "frame")) frame = config["frame"];
            if (json_has(config, "animation")) animate(config["animation"]);

            if (json_has(config, "originX")) origin.x = config["originX"];
            if (json_has(config, "originY")) origin.y = config["originY"];
            if (json_has(config, "origin")) origin = config["origin"];

            if (json_has(config, "cropLeft")) cropLeft = config["cropLeft"];
            if (json_has(config, "cropRight")) cropRight = config["cropRight"];
            if (json_has(config, "cropTop")) cropTop = config["cropTop"];
            if (json_has(config, "cropBottom")) cropBottom = config["cropBottom"];

            return this;
        }

        Amara::Action* animate(nlohmann::json config);

        sol::object animate(sol::object config) {
            Amara::Action* anim = animate(lua_to_json(config));
            return anim->get_lua_object();
        }
        sol::object stopAnimating() {
            for (Amara::Node* node: children) {
                if (node->is_animation && !node->destroyed) {
                    node->destroy();
                }
            }
            return get_lua_object();
        }
        
        sol::object setOrigin(float _x, float _y) {
            origin = Vector2( _x, _y );
            return get_lua_object();
        }
        sol::object setOrigin(float _o) {
            return setOrigin(_o, _o);
        }

        virtual void drawSelf(const Rectangle& v) override {
            if (image == nullptr) return;

            if (cropLeft < 0) cropLeft = 0;
            if (cropRight < 0) cropRight = 0;
            if (cropTop < 0) cropTop = 0;
            if (cropBottom < 0) cropBottom = 0;

            Vector2 vcenter = { v.w/2.0f, v.h/2.0f };
            Vector2 totalZoom = { passOn.zoom.x*passOn.window_zoom.x, passOn.zoom.y*passOn.window_zoom.y };

            float imgw = (spritesheet ? frameWidth : imageWidth);
            float imgh = (spritesheet ? frameHeight : imageHeight);

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

            SDL_FRect srcRect;
            SDL_FRect destRect;

            if (spritesheet) {
                int fixedFrame = frame % (int)floor(((float)image->width / (float)spritesheet->frameWidth) * ((float)image->height / (float)spritesheet->frameHeight));
                srcRect.x = static_cast<float>((fixedFrame % (imageWidth / frameWidth)) * frameWidth + cropLeft);
                srcRect.y = static_cast<float>(floor(fixedFrame / (imageWidth / frameWidth)) * frameHeight + cropTop);
                srcRect.w = static_cast<float>(frameWidth - cropLeft - cropRight);
                srcRect.h = static_cast<float>(frameHeight - cropTop - cropBottom);
            }
            else {
                srcRect = {
                    static_cast<float>(cropLeft),
                    static_cast<float>(cropTop),
                    static_cast<float>(imageWidth - cropLeft - cropRight),
                    static_cast<float>(imageHeight - cropTop - cropBottom)
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
                    v.x - diag_distance, v.y - diag_distance,
                    v.w + diag_distance*2, v.w + diag_distance*2
                )
            )) return;

            if (image->texture && Props::renderer) {
                // 2D Rendering
                SDL_SetTextureScaleMode(image->texture, SDL_SCALEMODE_NEAREST);
                SDL_SetTextureColorMod(image->texture, tint.r, tint.g, tint.b);
                SDL_SetTextureAlphaMod(image->texture, alpha * passOn.alpha * 255);
                setSDLBlendMode(image->texture, blendMode);

                SDL_RenderTextureRotated(
                    Props::renderer, 
                    image->texture,
                    &srcRect,
                    &destRect,
                    getDegrees(passOn.rotation + rotation),
                    &dorigin,
                    SDL_FLIP_NONE
                );
            }
            else if (image->gpuTexture && Props::gpuDevice) {
                // GPU Rendering
            }
            #ifdef AMARA_OPENGL
            else if (image->glTextureID != 0 && Props::glContext != NULL) {
                Quad srcQuad = Quad(
                    { srcRect.x/imageWidth, srcRect.y/imageHeight },
                    { (srcRect.x+srcRect.w)/imageWidth, srcRect.y/imageHeight },
                    { (srcRect.x+srcRect.w)/imageWidth, (srcRect.y+srcRect.h)/imageHeight },
                    { srcRect.x/imageWidth, (srcRect.y+srcRect.h)/imageHeight }
                );
                Quad destQuad = glTranslateQuad(v, rotateQuad(
                    Quad(destRect),
                    Vector2(
                        destRect.x + dorigin.x,
                        destRect.y + dorigin.y
                    ),
                    passOn.rotation + rotation
                ), passOn.insideFrameBuffer);

                vertices = {
                    destQuad.p1.x, destQuad.p1.y, srcQuad.p1.x, srcQuad.p1.y,
                    destQuad.p2.x, destQuad.p2.y, srcQuad.p2.x, srcQuad.p2.y,
                    destQuad.p3.x, destQuad.p3.y, srcQuad.p3.x, srcQuad.p3.y,
                    destQuad.p4.x, destQuad.p4.y, srcQuad.p4.x, srcQuad.p4.y
                };

                Props::renderBatch->pushQuad(
                    Props::currentShaderProgram,
                    image->glTextureID,
                    vertices, passOn.alpha * alpha, tint,
                    v, passOn.insideFrameBuffer,
                    blendMode
                );
            }
            #endif
        }

        float getWidth() {
            if (spritesheet) return frameWidth*scale.x;
            if (image) return imageWidth*scale.x;
            return 0;
        }
        float getHeight() {
            if (spritesheet) return frameHeight*scale.y;
            if (image) return imageHeight*scale.y;
            return 0;
        }

        Rectangle getRectangle() {
            return Rectangle(
                pos.x - (getWidth()*scale.x)*origin.x,
                pos.y - (getHeight()*scale.y)*origin.y,
                getWidth()*scale.x,
                getHeight()*scale.y
            );
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Sprite>("Sprite",
                sol::base_classes, sol::bases<Node>(),
                "setTexture", &Sprite::setTexture,
                "tint", &Sprite::tint,
                "blendMode", &Sprite::blendMode,
                "frame", &Sprite::frame,
                "animate",  sol::resolve<sol::object(sol::object)>(&Sprite::animate),
                "stopAnimating", &Sprite::stopAnimating,
                "imagew", sol::readonly(&Sprite::imageWidth),
                "imageh", sol::readonly(&Sprite::imageHeight),
                "framew", sol::readonly(&Sprite::frameWidth),
                "frameh", sol::readonly(&Sprite::frameHeight),
                "cropLeft", &Sprite::cropLeft,
                "cropRight", &Sprite::cropRight,
                "cropTop", &Sprite::cropTop,
                "cropBottom", &Sprite::cropBottom,
                "origin", &Sprite::origin,
                "setOrigin", sol::overload(
                    sol::resolve<sol::object(float, float)>(&Sprite::setOrigin),
                    sol::resolve<sol::object(float)>(&Sprite::setOrigin)
                ),
                "w", sol::property(&Sprite::getWidth),
                "h", sol::property(&Sprite::getHeight),
                "width", sol::property(&Sprite::getWidth),
                "height", sol::property(&Sprite::getHeight),
                "rect", sol::property(&Sprite::getRectangle)
            );
        }
    };
}