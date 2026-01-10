namespace Amara {
    class ShaderLayer: public Amara::ShaderContainer {
    public:
        ShaderLayer(): Amara::ShaderContainer() {
            set_base_node_id("ShaderLayer");
        }

        virtual void drawObjects(const Rectangle& v) override {
            if (fixedToCamera && !gameProps->passOn.insideTextureContainer) {
                gameProps->passOn.reset();
            }
            passOn = gameProps->passOn;
            
            width = ceil(v.w);
            height = ceil(v.h);
            rotation = 0;

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
            
            destRect.x = 0;
            destRect.y = 0;
            destRect.w = width;
            destRect.h = height;

            SDL_FPoint dorigin = { 0, 0 };
            
            srcRect = getSrcRect();

            if (input.active && !passOn.insideTextureContainer) {
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
            gameProps->passOn = passOn;

            Amara::Node::drawChildren(v);

            gameProps->passOn = rec_props;
            passOn = rec_props;
        }

        virtual void pass_on_properties() override {
            if (fixedToCamera && !gameProps->passOn.insideTextureContainer) {
                gameProps->passOn.reset();
            }
            passOn = gameProps->passOn;
            
            if (passOnPropsEnabled) {
                gameProps->passOn = passOn;
            }
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<ShaderLayer>("ShaderLayer",
                sol::base_classes, sol::bases<Amara::ShaderContainer, Amara::TextureContainer, Amara::Node>()
            );
        }
    };
}