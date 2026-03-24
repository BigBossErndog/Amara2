namespace Amara {
    class RepeatSprite: public Amara::Sprite {
    public:
        Vector2 offset;
        Vector2 spacing;

        RepeatSprite(): Amara::Sprite() {
            set_base_node_id("RepeatSprite");
        }
        
        Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "offset")) offset = json_get<Vector2>(config, "offset");
            if (json_has(config, "spacing")) spacing = json_get<Vector2>(config, "spacing");
            return Amara::Sprite::configure(config);
        }

        void drawTile(const Rectangle& v, Vector2 gOffset) {
            if (image == nullptr) return;

            if (cropLeft < 0) cropLeft = 0;
            if (cropRight < 0) cropRight = 0;
            if (cropTop < 0) cropTop = 0;
            if (cropBottom < 0) cropBottom = 0;

            Vector2 vcenter = { v.w/2.0f, v.h/2.0f };
            Vector2 totalZoom = { passOn.zoom.x*passOn.window_zoom.x, passOn.zoom.y*passOn.window_zoom.y };

            float imgw = (spritesheet ? frameWidth : textureWidth);
            float imgh = (spritesheet ? frameHeight : textureHeight);

            Vector2 render_pos = pos + gOffset;
            if (renderPixelPerfect) render_pos = render_pos.round();

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

            if (input.active && !passOn.inputDisabled) {
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

        virtual void drawSelf(const Rectangle& v) override {
            if (image == nullptr) return;

            float imgw = (spritesheet ? frameWidth : textureWidth);
            float imgh = (spritesheet ? frameHeight : textureHeight);
            float min = ((imgw < imgh) ? imgw : imgh) + ((spacing.x < spacing.y) ? spacing.x : spacing.y);

            int v_diag = ((v.w > v.h) ? v.w : v.h) * 1.5;

            Vector2 cal_scale = scale * passOn.scale;
            float min_scale = ((cal_scale.x < cal_scale.y) ? cal_scale.x : cal_scale.y);
            
            int num_sprites_across = ceil(v_diag / (min * min_scale)) + 2;
            Vector2 view_length = Vector2(
                (imgw + spacing.x) * num_sprites_across,
                (imgh + spacing.y) * num_sprites_across
            );

            float ox = fmod(offset.x, imgw + spacing.x);
            while (ox < 0) ox += imgw + spacing.x;
            float oy = fmod(offset.y, imgh + spacing.y);
            while (oy < 0) oy += imgh + spacing.y;

            for (int i = 0; i < num_sprites_across; i++) {
                for (int j = 0; j < num_sprites_across; j++) {
                    drawTile(v, Vector2(
                        i * (imgw + spacing.x) - view_length.x / 2.0f + ox,
                        j * (imgh + spacing.y) - view_length.y / 2.0f + oy
                    ));
                }
            }
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<RepeatSprite>("RepeatSprite",
                sol::base_classes, sol::bases<Amara::Sprite, Amara::Node>(),
                "offset", sol::property(
                    [](Amara::RepeatSprite& r) -> Vector2& { return r.offset; },
                    [](Amara::RepeatSprite& r, sol::object v) { r.offset = v; }
                ),
                "spacing", sol::property(
                    [](Amara::RepeatSprite& r) -> Vector2& { return r.spacing; },
                    [](Amara::RepeatSprite& r, sol::object v) { r.spacing = v; }
                )
            );
        }
    };
}