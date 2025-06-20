namespace Amara {
    class ShaderContainer: public Amara::TextureContainer {
    public:
        #ifdef AMARA_OPENGL
        GLuint glCanvas1ID = 0;
        GLuint glBuffer1ID = 0;

        GLuint glCanvas2ID = 0;
        GLuint glBuffer2ID = 0;
        #endif

        SDL_Texture* canvas1Texture = nullptr;
        SDL_Texture* canvas2Texture = nullptr;

        bool canvas_flip = false;

        std::vector<Amara::ShaderProgram*> shader_passes;

        ShaderContainer(): Amara::TextureContainer() {
            set_base_node_id("ShaderContainer");
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "shaderPasses")) {
                nlohmann::json config = config["shaderPasses"];
                if (config.is_array()) {
                    for (auto it = config.begin(); it != config.end(); ++it) {
                        addShaderPass(it.value());
                    }
                }
            }
            return Amara::TextureContainer::configure(config);
        }

        virtual void deletePipeline() override {
            canvasTexture = canvas1Texture;

            #ifdef AMARA_OPENGL
            glCanvasID = glCanvas1ID;
            glBufferID = glBuffer1ID;
            #endif

            Amara::TextureContainer::deletePipeline();

            #ifdef AMARA_OPENGL
            glCanvas1ID = glCanvasID;
            glBuffer1ID = glBufferID;
            #endif

            if (canvas2Texture) {
                SDL_DestroyTexture(canvas2Texture);
                canvas2Texture = nullptr;
            }
            #ifdef AMARA_OPENGL
            if (glCanvas2ID != 0) {
                glDeleteTextures(1, &glCanvas2ID);
                glCanvas2ID = 0;
            }
            if (glBuffer2ID != 0) {
                glDeleteFramebuffers(1, &glBuffer2ID);
                glBuffer2ID = 0;
            }
            #endif
        }

        virtual void createCanvas(int _w, int _h) override {
            Amara::TextureContainer::createCanvas(_w, _h);

            if (gameProps->graphics == GraphicsEnum::Render2D && gameProps->renderer) { 
                canvas2Texture = SDL_CreateTexture(
                    gameProps->renderer,
                    SDL_PIXELFORMAT_RGBA32,
                    SDL_TEXTUREACCESS_TARGET,
                    _w,
                    _h
                );
            }
            canvas1Texture = canvasTexture;

            #ifdef AMARA_OPENGL
            if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                glGenTextures(1, &glCanvas2ID);
                glBindTexture(GL_TEXTURE_2D, glCanvas2ID);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _w, _h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            }
            glBuffer1ID = glBufferID;
            #endif
        }

        void drawPass() {
            if (gameProps->graphics == GraphicsEnum::Render2D && gameProps->renderer) {
                SDL_Texture* stamp_texture = (canvas_flip) ? canvas2Texture : canvas1Texture;
                SDL_Texture* target_texture = (canvas_flip) ? canvas1Texture : canvas2Texture;

                SDL_SetRenderTarget(gameProps->renderer, target_texture);
                SDL_SetRenderDrawColor(gameProps->renderer, 0, 0, 0, 0);
                SDL_RenderClear(gameProps->renderer);
                SDL_SetRenderDrawColor(gameProps->renderer, 255, 255, 255, 255);

                SDL_SetTextureScaleMode(stamp_texture, SDL_SCALEMODE_NEAREST);
                SDL_SetTextureColorMod(stamp_texture, 255, 255, 255);
                SDL_SetTextureAlphaMod(stamp_texture, 255);
                Apply_SDL_BlendMode(gameProps, stamp_texture, Amara::BlendMode::Alpha);

                SDL_RenderTexture(gameProps->renderer, stamp_texture, NULL, NULL);
            }
            #ifdef AMARA_OPENGL
            if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                GLuint stamp_buffer = (canvas_flip) ? glBuffer2ID : glBuffer1ID;
                GLuint stamp_canvas = (canvas_flip) ? glCanvas2ID : glCanvas1ID;

                GLuint target_buffer = (canvas_flip) ? glBuffer1ID : glBuffer2ID;
                GLuint target_canvas = (canvas_flip) ? glCanvas1ID : glCanvas2ID;

                gameProps->renderBatch->flush();

                GLint prevBuffer = 0;
                glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevBuffer);
                glBindFramebuffer(GL_FRAMEBUFFER, target_buffer);
                
                glViewport(0, 0, width, height);
                glClearColor(0, 0, 0, 0);
                glClear(GL_COLOR_BUFFER_BIT);

                SDL_FRect srcRect = Rectangle::makeSDLFRect(container_viewport);
                Quad srcQuad = Quad(
                    { srcRect.x/width, srcRect.y/height },
                    { (srcRect.x+srcRect.w)/width, srcRect.y/height },
                    { (srcRect.x+srcRect.w)/width, (srcRect.y+srcRect.h)/height },
                    { srcRect.x/width, (srcRect.y+srcRect.h)/height }
                );
                Quad destQuad = glTranslateQuad(
                    container_viewport,
                    Quad(container_viewport),
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
                    stamp_canvas,
                    vertices, 1, Amara::Color::White,
                    container_viewport, true,
                    blendMode
                );
            }
            #endif
        }

        void addShaderPass(std::string shader_key) {
            #ifdef AMARA_OPENGL
            ShaderProgram* prog = gameProps->shaders->getShaderProgram(shader_key);
            if (prog) {
                shader_passes.push_back(prog);
            }
            else {
                debug_log("Error: ShaderProgram \"", shader_key, "\" not found.");
                gameProps->breakWorld();
            }
            #endif
        }
        void luaAddShaderPass(sol::object val) {
            if (val.is<std::string>()) {
                addShaderPass(val.as<std::string>());
            }
            else if (val.is<sol::table>()) {
                nlohmann::json config = val.as<nlohmann::json>();
                if (config.is_array()) {
                    for (auto it = config.begin(); it != config.end(); ++it) {
                        addShaderPass(it.value());
                    }
                }
            }
        }

        void drawCanvas(const Rectangle& v) {
            ShaderProgram* rec_shader = currentShaderProgram;
            canvas_flip = false;

            if (gameProps->graphics == GraphicsEnum::Render2D && gameProps->renderer) {
                canvasTexture = canvas1Texture;
            }
            #ifdef AMARA_OPENGL
            if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                glBufferID = glBuffer1ID;
                glCanvasID = glCanvas1ID;
            }
            #endif

            Amara::ShaderContainer::drawCanvas(v);

            PassOnProps rec_props = gameProps->passOn;
            PassOnProps new_props;
            new_props.insideTextureContainer = true;

            for (Amara::ShaderProgram* prog: shader_passes) {
                currentShaderProgram = prog;
                drawPass();
                canvas_flip = !canvas_flip;
            }

            gameProps->passOn = rec_props;

            currentShaderProgram = rec_shader;

            if (canvas_flip) {
                if (gameProps->graphics == GraphicsEnum::Render2D && gameProps->renderer) {
                    canvasTexture = (canvas_flip) ? canvas1Texture : canvas2Texture;
                }
                #ifdef AMARA_OPENGL
                if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                    glBufferID = (canvas_flip) ? glBuffer1ID : glBuffer2ID;
                    glCanvasID = (canvas_flip) ? glCanvas1ID : glCanvas2ID;
                }
                #endif
            }
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<ShaderContainer>("ShaderContainer",
                sol::base_classes, sol::bases<Amara::TextureContainer, Amara::Node>(),
                "shaderPasses", sol::property(&ShaderContainer::luaAddShaderPass)
            );
        }
    };
}