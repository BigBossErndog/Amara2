namespace Amara {
    /**
     * This is used for 2D sprites.
     */
    class Sprite: public Amara::Node {
    public:
        ImageAsset* image = nullptr;
        SpritesheetAsset* spritesheet = nullptr;

        int imageWidth = 0;
        int imageHeight = 0;

        int frameWidth = 0;
        int frameHeight = 0;

        int cropLeft = 0;
        int cropRight = 0;
        int cropTop = 0;
        int cropBottom = 0;

        int frame = 0;

        Vector2 origin = { 0.5, 0.5 };

        Sprite(): Amara::Node() {
            set_base_node_id("Sprite");
        }

        bool loadTexture(std::string key) {
            image = nullptr;
            spritesheet = nullptr;

            if (!Props::assets->has(key)) {
                debug_log("Error: Asset\"", key, "\" was not found.");
                return false;
            }

            image = Props::assets->get(key)->as<ImageAsset*>();
            
            if (image == nullptr) {
                debug_log("Error: Asset\"", key, "\" is not a valid texture asset.");
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

        sol::object setOrigin(float _x, float _y) {
            origin = { _x, _y };
            return get_lua_object();
        }

        virtual void drawSelf(const Rectangle& v) override {
            if (image == nullptr) return;

            SDL_Rect setv = Rectangle::makeSDLRect(v);
            SDL_SetRenderViewport(Props::renderer, &setv);

            if (cropLeft < 0) cropLeft = 0;
            if (cropRight < 0) cropRight = 0;
            if (cropTop < 0) cropTop = 0;
            if (cropBottom < 0) cropBottom = 0;

            if (image->texture && Props::renderer) {
                // 2D Rendering
                SDL_FRect srcRect;
                SDL_FRect destRect;

                Vector2 vcenter = centerOf(v);

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

                Rectangle dim = {
                    anchoredPos.x + (cropLeft - imgw*origin.x)*scale.x*passOn.scale.x, 
                    anchoredPos.y - anchoredPos.z + (cropTop - imgh*origin.y)*scale.y*passOn.scale.y,
                    (imgw - cropLeft - cropRight)*scale.x*passOn.scale.x,
                    (imgh - cropTop - cropBottom)*scale.y*passOn.scale.y
                };
 
                destRect.x = vcenter.x + (dim.x + passOn.scroll.x)*passOn.zoom.x; 
                destRect.y = vcenter.y + (dim.y + passOn.scroll.y)*passOn.zoom.y;
                destRect.w = dim.w * passOn.zoom.x;
                destRect.h = dim.h * passOn.zoom.y;

                SDL_FPoint dorigin = {
                    (imgw*origin.x - cropLeft)*scale.x*passOn.scale.x*passOn.zoom.x,
                    (imgh*origin.y - cropTop)*scale.y*passOn.scale.y*passOn.zoom.y
                };

                if (spritesheet) {
                    srcRect.x = static_cast<float>((frame % (imageWidth / frameWidth)) * frameWidth + cropLeft);
                    srcRect.y = static_cast<float>(floor(frame / (imageWidth / frameWidth)) * frameHeight + cropTop);
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
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Sprite>("Sprite",
                sol::base_classes, sol::bases<Node>(),
                "loadTexture", &Sprite::loadTexture,
                "imagew", sol::readonly(&Sprite::imageWidth),
                "imageh", sol::readonly(&Sprite::imageHeight),
                "framew", sol::readonly(&Sprite::frameWidth),
                "frameh", sol::readonly(&Sprite::frameHeight),
                "cropLeft", &Sprite::cropLeft,
                "cropRight", &Sprite::cropRight,
                "cropTop", &Sprite::cropTop,
                "cropBottom", &Sprite::cropBottom,
                "setOrigin", &Sprite::setOrigin
            );
        }
    };
}