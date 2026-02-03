namespace Amara {
    class QuadSprite: public Amara::Sprite {
    public:
        Amara::Quad def;
        SDL_Vertex draw_vertices[8];
        bool points_initialized = false;
        
        QuadSprite(): Amara::Sprite() {
            set_base_node_id("QuadSprite");
        }
        
        virtual Amara::Node* configure(nlohmann::json config) override {
            Amara::Sprite::configure(config);
            
            if (json_has(config, "points")) {
                def = config["points"];
                points_initialized = true;
            }
            if (json_has(config, "quad")) {
                def = config["quad"];
                points_initialized = true;
            }
            
            return this;
        }
        
        virtual bool setTexture(std::string key) override {
            if (Amara::Sprite::setTexture(key)) {
                if (!points_initialized) {
                    def = Amara::Quad(
                        Amara::Vector2(-frameWidth/2.0, -frameHeight/2.0),
                        Amara::Vector2(frameWidth/2.0, -frameHeight/2.0),
                        Amara::Vector2(frameWidth/2.0, frameHeight/2.0),
                        Amara::Vector2(-frameWidth/2.0, frameHeight/2.0)
                    );
                    points_initialized = true;
                }
                return true;
            }
            return false;
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
            Quad drawQuad = rotateQuad(
                def,
                Vector2::Origin,
                passOn.rotation + rotation
            );
            
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
            
            for (int i = 0; i < 4; ++i) {
                SDL_Vertex& vertex = draw_vertices[i];
                vertex.position.x = vcenter.x + (anchoredPos.x + (drawQuad[i].x * scale.x * passOn.scale.x)) * totalZoom.x;
                vertex.position.y = vcenter.y + (anchoredPos.y - anchoredPos.z + (drawQuad[i].y * scale.y * passOn.scale.y)) * totalZoom.y;
                
                vertex.color = { 1, 1, 1, 1 };
                
                switch (i) {
                    case 0:
                        vertex.tex_coord.x = srcRect.x / (double)textureWidth;
                        vertex.tex_coord.y = srcRect.y / (double)textureHeight;
                        break;
                    case 1:
                        vertex.tex_coord.x = (srcRect.x + srcRect.w) / (double)textureWidth;
                        vertex.tex_coord.y = srcRect.y / (double)textureHeight;
                        break;
                    case 2:
                        vertex.tex_coord.x = (srcRect.x + srcRect.w) / (double)textureWidth;
                        vertex.tex_coord.y = (srcRect.y + srcRect.h) / (double)textureHeight;
                        break;
                    case 3:
                        vertex.tex_coord.x = srcRect.x / (double)textureWidth;
                        vertex.tex_coord.y = (srcRect.y + srcRect.h) / (double)textureHeight;
                        break;
                }
            }
            
            if (input.active && !passOn.inputDisabled) {
                input.queueInput(moveQuad(
                    Quad(Vector2(draw_vertices[0]), Vector2(draw_vertices[1]), Vector2(draw_vertices[2]), Vector2(draw_vertices[3])),
                    v.x, v.y
                ), 
                v, nullptr);
            }

            if (image->texture && gameProps->renderer) {
                // 2D Rendering
                SDL_SetTextureScaleMode(image->texture, SDL_SCALEMODE_NEAREST);
                SDL_SetTextureColorMod(image->texture, tint.r, tint.g, tint.b);
                SDL_SetTextureAlphaMod(image->texture, alpha * passOn.alpha * 255);
                Apply_SDL_BlendMode(gameProps, image->texture, blendMode);
                
                const int indices[] = { 0, 1, 2, 2, 3, 0 };
                
                SDL_RenderGeometry(gameProps->renderer, image->texture, draw_vertices, 4, indices, 6);
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
                Quad destQuad = glTranslateQuad(
                    v,
                    Amara::Quad(draw_vertices[0], draw_vertices[1], draw_vertices[2], draw_vertices[3]), 
                    passOn.insideTextureContainer
                );

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
        
        static void bind_lua(sol::state& lua) {
            lua.new_usertype<QuadSprite>("QuadSprite",
                sol::base_classes, sol::bases<Amara::Sprite, Amara::Node>(),
                "quad", sol::property(
                    [](Amara::QuadSprite& sprite) -> Amara::Quad& {
                        return sprite.def;
                    },
                    [](Amara::QuadSprite& sprite, sol::object val) {
                        sprite.def = val;
                    }
                ),
                "points", sol::property(
                    [](Amara::QuadSprite& sprite) -> Amara::Quad& {
                        return sprite.def;
                    },
                    [](Amara::QuadSprite& sprite, sol::object val) {
                        sprite.def = val;
                    }
                ),
                1, [](Amara::QuadSprite& sprite) {
                    sprite.def = sprite.def[0];
                },
                2, [](Amara::QuadSprite& sprite) {
                    sprite.def = sprite.def[1];
                },
                3, [](Amara::QuadSprite& sprite) {
                    sprite.def = sprite.def[2];
                },
                4, [](Amara::QuadSprite& sprite) {
                    sprite.def = sprite.def[3];
                }
            );
        }
    };
}