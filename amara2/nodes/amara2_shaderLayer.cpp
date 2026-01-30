namespace Amara {
    class ShaderLayer: public Amara::ShaderContainer {
    public:
        bool resolutionLocked = true;
        Vector2 render_scale = Vector2(1, 1);
        
        ShaderLayer(): Amara::ShaderContainer() {
            set_base_node_id("ShaderLayer");
        }
        
        virtual Amara::Node* configure(nlohmann::json config) override {
            if (config.contains("resolution")) {
                resolutionLocked = false;
                nlohmann::json& res_data = config["resolution"];
                if (res_data.is_array()) {
                    if (res_data.size() == 2) {
                        width = floor((float)res_data[0]);
                        height = floor((float)res_data[1]);
                    }
                    else if (res_data.size() == 4) {
                        width = floor((float)res_data[2]);
                        height = floor((float)res_data[3]);
                    }
                }
                else if (res_data.is_object()) {
                    if (json_has_any(res_data, "width", "height", "w", "h")) {
                        Amara::Rectangle rect = Amara::Rectangle(res_data);
                        width = floor(rect.w);
                        height = floor(rect.h);
                    }
                    else if (json_has_any(res_data, "x", "y")) {
                        Amara::Vector2 vec = Amara::Vector2(res_data);
                        width = floor(vec.x);
                        height = floor(vec.y);
                    }
                }
            }
            return Amara::ShaderContainer::configure(config);
        }

        virtual void drawObjects(const Rectangle& v) override {
            if (fixedToCamera && !gameProps->passOn.texturePropsLock) {
                gameProps->passOn.reset(true);
            }
            passOn = gameProps->passOn;
            
            if (resolutionLocked) {
                width = ceil(v.w);
                height = ceil(v.h);
                render_scale = Vector2(1, 1);
            }
            else {
                render_scale = Vector2(
                    v.w / width,
                    v.h / height
                );
            }
            
            if (rec_width != width || rec_height != height) {
                createCanvas(width, height);
            }

            #ifdef AMARA_OPENGL
            currentShaderProgram = gameProps->defaultShaderProgram;
            #endif
            
            if (update_canvas || !canvasLocked) {
                drawCanvas(v);
                update_canvas = false;
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

            SDL_FRect srcRect;
            SDL_FRect destRect;
            
            destRect.x = (v.w - (width * render_scale.x))/2.0;
            destRect.y = (v.h - (height * render_scale.y))/2.0;
            destRect.w = width * render_scale.x;
            destRect.h = height * render_scale.y;

            SDL_FPoint dorigin = { 0, 0 };
            
            srcRect = getSrcRect();

            if (input.active && !passOn.inputDisabled) {
                Quad inputQuad = Quad(destRect);
                input.queueInput(moveQuad(inputQuad, v.x, v.y), v, nullptr);
            }
            
            if (canvasTexture && gameProps->renderer) {
                SDL_SetTextureScaleMode(canvasTexture, SDL_SCALEMODE_NEAREST);
                SDL_SetTextureColorMod(canvasTexture, tint.r, tint.g, tint.b);
                SDL_SetTextureAlphaMod(canvasTexture, alpha * passOn.alpha * 255);
                Apply_SDL_BlendMode(gameProps, canvasTexture, blendMode);

                SDL_RenderTextureRotated(
                    gameProps->renderer, 
                    canvasTexture,
                    &srcRect,
                    &destRect,
                    0,
                    &dorigin,
                    SDL_FLIP_NONE
                );
            }
            #ifdef AMARA_OPENGL
            else if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                Quad srcQuad = Quad(
                    { srcRect.x/width, srcRect.y/height },
                    { (srcRect.x+srcRect.w)/width, srcRect.y/height },
                    { (srcRect.x+srcRect.w)/width, (srcRect.y+srcRect.h)/height },
                    { srcRect.x/width, (srcRect.y+srcRect.h)/height }
                );
                Quad destQuad = glTranslateQuad(v, Quad(destRect), passOn.insideTextureContainer);

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

        virtual void drawChildren(const Rectangle& v) override {
            PassOnProps rec_props = gameProps->passOn;
            
            passOn.insideTextureContainer = true;
            passOn.texturePropsLock = false;
            if (!resolutionLocked) {
                passOn.window_zoom /= render_scale;
                passOn.input_scale *= render_scale;
            }
            gameProps->passOn = passOn;

            Amara::Node::drawChildren(v);
            
            gameProps->passOn = rec_props;
            passOn = rec_props;
        }

        virtual void pass_on_properties() override {
            Amara::Node::pass_on_properties();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<ShaderLayer>("ShaderLayer",
                sol::base_classes, sol::bases<Amara::ShaderContainer, Amara::TextureContainer, Amara::Node>(),
                "resolution", sol::property(
                    [] (ShaderLayer& self) {
                        return Vector2(self.width, self.height);
                    },
                    [] (ShaderLayer& self, sol::object val) {
                        if (val.valid()) {
                            nlohmann::json j = lua_to_json(val);
                            self.configure(nlohmann::json::object({
                                { "resolution", j }
                            }));
                        }
                        else {
                            self.resolutionLocked = true;
                        }
                    }
                )
            );
        }
    };
}