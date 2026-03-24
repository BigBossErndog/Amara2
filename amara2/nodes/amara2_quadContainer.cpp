namespace Amara {
    class QuadContainer: public Amara::TextureContainer {
    public:
        Amara::Quad def;
        SDL_Vertex draw_vertices[8];
        bool points_initialized = false;
        
        QuadContainer(): Amara::TextureContainer() {
            set_base_node_id("QuadContainer");
        }
        
        virtual void create() override {
            Amara::TextureContainer::create();
            if (!points_initialized) {
                def = Quad(
                    Vector2(-width/2, -height/2),
                    Vector2(width/2, -height/2),
                    Vector2(width/2, height/2),
                    Vector2(-width/2, height/2)
                );
                points_initialized = true;
            }
        }
        
        virtual void drawObjects(const Rectangle& v) override {
            if (fixedToCamera && !gameProps->passOn.texturePropsLock) {
                gameProps->passOn.reset(true);
            }
            passOn = gameProps->passOn;

            if (rec_width != width || rec_height != height) {
                createCanvas(width, height);
            }
            
            #ifdef AMARA_OPENGL
            currentShaderProgram = gameProps->defaultShaderProgram;
            #endif
            
            if (!drawn_this_frame) {
                if (update_canvas || !canvasLocked) {
                    drawCanvas(v);
                    update_canvas = false;
                }
                drawn_this_frame = true;
            }

            #ifdef AMARA_OPENGL
            ShaderProgram* rec_shader = gameProps->currentShaderProgram;
            if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                if (shaderProgram && shaderProgram != gameProps->currentShaderProgram) {
                    gameProps->currentShaderProgram = shaderProgram;
                }
            }
            #endif

            if (cropLeft < 0) cropLeft = 0;
            if (cropRight < 0) cropRight = 0;
            if (cropTop < 0) cropTop = 0;
            if (cropBottom < 0) cropBottom = 0;

            Vector2 vcenter = { v.w/2.0f, v.h/2.0f };
            Vector2 totalZoom = { passOn.zoom.x*passOn.window_zoom.x, passOn.zoom.y*passOn.window_zoom.y };

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

            SDL_FRect srcRect;
            SDL_FRect destRect;
            Quad drawQuad = rotateQuad(
                def,
                Vector2::Origin,
                passOn.rotation + rotation
            );
            
            for (int i = 0; i < 4; i++) {
                SDL_Vertex& vertex = draw_vertices[i];
                vertex.position.x = vcenter.x + (anchoredPos.x + (drawQuad[i].x * scale.x * passOn.scale.x)) * totalZoom.x;
                vertex.position.y = vcenter.y + (anchoredPos.y - anchoredPos.z + (drawQuad[i].y * scale.y * passOn.scale.y)) * totalZoom.y;
                
                vertex.color = { 1, 1, 1, 1 };
                
                switch (i) {
                    case 0:
                        vertex.tex_coord.x = srcRect.x / (double)width;
                        vertex.tex_coord.y = srcRect.y / (double)height;
                        break;
                    case 1:
                        vertex.tex_coord.x = (srcRect.x + srcRect.w) / (double)width;
                        vertex.tex_coord.y = srcRect.y / (double)height;
                        break;
                    case 2:
                        vertex.tex_coord.x = (srcRect.x + srcRect.w) / (double)width;
                        vertex.tex_coord.y = (srcRect.y + srcRect.h) / (double)height;
                        break;
                    case 3:
                        vertex.tex_coord.x = srcRect.x / (double)width;
                        vertex.tex_coord.y = (srcRect.y + srcRect.h) / (double)height;
                        break;
                }
            }
            
            srcRect = getSrcRect();

            if (input.active && !passOn.inputDisabled) {
                input.queueInput(moveQuad(
                    Quad(Vector2(draw_vertices[0]), Vector2(draw_vertices[1]), Vector2(draw_vertices[2]), Vector2(draw_vertices[3])),
                    v.x, v.y
                ), 
                v, nullptr);
            }
            
            if (canvasTexture && gameProps->renderer) {
                SDL_SetTextureScaleMode(canvasTexture, SDL_SCALEMODE_NEAREST);
                SDL_SetTextureColorMod(canvasTexture, tint.r, tint.g, tint.b);
                SDL_SetTextureAlphaMod(canvasTexture, alpha * passOn.alpha * 255);
                Apply_SDL_BlendMode(gameProps, canvasTexture, blendMode);
                
                const int indices[] = { 0, 1, 2, 2, 3, 0 };
                
                SDL_RenderGeometry(gameProps->renderer, canvasTexture, draw_vertices, 4, indices, 6);
            }
            #ifdef AMARA_OPENGL
            else if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                Quad srcQuad = Quad(
                    { srcRect.x/width, srcRect.y/height },
                    { (srcRect.x+srcRect.w)/width, srcRect.y/height },
                    { (srcRect.x+srcRect.w)/width, (srcRect.y+srcRect.h)/height },
                    { srcRect.x/width, (srcRect.y+srcRect.h)/height }
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
                    currentShaderProgram,
                    glCanvasID,
                    vertices, passOn.alpha * alpha, tint,
                    v, passOn.insideTextureContainer,
                    blendMode
                );
            }
                
            if (rec_shader && shaderProgram && shaderProgram != rec_shader) {
                gameProps->currentShaderProgram = rec_shader;
            }
            #endif
        }
        
        virtual Amara::Node* configure(nlohmann::json config) override {
            Amara::TextureContainer::configure(config);
            
            if (json_has(config, "points")) {
                def = json_get<Amara::Quad>(config, "points");
                points_initialized = true;
            }
            if (json_has(config, "quad")) {
                def = json_get<Amara::Quad>(config, "quad");
                points_initialized = true;
            }
            
            return this;
        }
        
        static void bind_lua(sol::state& lua) {
            lua.new_usertype<QuadContainer>("QuadContainer",
                sol::base_classes, sol::bases<Amara::TextureContainer, Amara::Node>(),
                "quad", sol::property(
                    [](Amara::QuadContainer& sprite) -> Amara::Quad& {
                        return sprite.def;
                    },
                    [](Amara::QuadContainer& sprite, sol::object val) {
                        sprite.def = val;
                    }
                ),
                "points", sol::property(
                    [](Amara::QuadContainer& sprite) -> Amara::Quad& {
                        return sprite.def;
                    },
                    [](Amara::QuadContainer& sprite, sol::object val) {
                        sprite.def = val;
                    }
                ),
                1, [](Amara::QuadContainer& sprite) {
                    sprite.def = sprite.def[0];
                },
                2, [](Amara::QuadContainer& sprite) {
                    sprite.def = sprite.def[1];
                },
                3, [](Amara::QuadContainer& sprite) {
                    sprite.def = sprite.def[2];
                },
                4, [](Amara::QuadContainer& sprite) {
                    sprite.def = sprite.def[3];
                }
            );
        }
    };
}