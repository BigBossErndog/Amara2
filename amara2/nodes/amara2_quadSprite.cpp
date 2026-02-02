namespace Amara {
    class QuadSprite: public Amara::Sprite {
    public:
        std::array<Amara::Vector2, 4> points;
        SDL_Vertex draw_vertices[8];
        bool points_initialized = false;
        
        sol::table points_tbl;
        
        QuadSprite(): Amara::Sprite() {
            set_base_node_id("QuadSprite");
        }
        
        virtual void init() override {
            Amara::Sprite::init();
            
            if (!points_tbl.valid()) {
                points_tbl = gameProps->lua.create_table();
                sol::table points_meta = gameProps->lua.create_table();
                points_meta["__newindex"] = [this](sol::table tbl, sol::object key, sol::object val) {
                    if (key.is<int>()) {
                        int index = key.as<int>();
                        if (index < 1 || index > 4) {
                            fatal_error("Error: QuadSprite.points can only be indexed from 1 - 4. Received ", index, ".");
                        }
                        points[index] = Amara::Vector2(val);
                    }
                };
                points_meta["__index"] = [this](sol::table tbl, sol::object key) -> sol::object {
                    if (key.is<int>()) {
                        int index = key.as<int>();
                        if (index < 1 || index > 4) {
                            fatal_error("Error: QuadSprite.points can only be indexed from 1 - 4. Received ", index, ".");
                        }
                        return sol::make_object(gameProps->lua, &(points[index]));
                    }
                    return sol::nil;
                };
            }
        }
        
        virtual Amara::Node* configure(nlohmann::json config) override {
            Amara::Sprite::configure(config);
            
            if (json_has(config, "points")) {
                nlohmann::json points_data = config["points"];
                
                if (points_data.is_array()) {
                    if (points_data.size() != 4) {
                        fatal_error("Error: QuadSprite points requires 4 points.");
                    }
                    for (int i = 0; i < 4; i++) {
                        points[i] = Amara::Vector2(points_data[i]);
                    }
                    points_initialized = true;
                }
                else if (points_data.is_object()) {
                    Amara::Quad quad = Amara::Quad(points_data);
                    points[0] = quad.p1;
                    points[1] = quad.p2;
                    points[2] = quad.p3;
                    points[3] = quad.p4;
                    points_initialized = true;
                }
            }
            
            return this;
        }
        
        virtual bool setTexture(std::string key) override {
            if (Amara::Sprite::setTexture(key)) {
                if (!points_initialized) {
                    points[0] = Amara::Vector2(-frameWidth/2.0, -frameHeight/2.0);
                    points[1] = Amara::Vector2(frameWidth/2.0, -frameHeight/2.0);
                    points[2] = Amara::Vector2(frameWidth/2.0, frameHeight/2.0);
                    points[3] = Amara::Vector2(-frameWidth/2.0, frameHeight/2.0);
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
                vertex.position.x = vcenter.x + (anchoredPos.x + (points[i].x * scale.x * passOn.scale.x)) * totalZoom.x;
                vertex.position.y = vcenter.y + (anchoredPos.y - anchoredPos.z + (points[i].y * scale.y * passOn.scale.y)) * totalZoom.y;
                
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
                Quad inputQuad = rotateQuad(
                    Quad(destRect),
                    anchoredPos,
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
                Quad destQuad = glTranslateQuad(v, rotateQuad(
                    Quad(points[0], points[1], points[2], points[3]),
                    anchoredPos,
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
        
        static void bind_lua(sol::state& lua) {
            lua.new_usertype<QuadSprite>("QuadSprite",
                sol::base_classes, sol::bases<Amara::Sprite, Amara::Node>(),
                "points", &Amara::QuadSprite::points
            );
        }
    };
}