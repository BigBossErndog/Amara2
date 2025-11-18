namespace Amara {
    class ShaderContainer: public Amara::TextureContainer {
    public:
        #ifdef AMARA_OPENGL
        GLuint glCanvases[2] = { 0, 0 };
        GLuint glBuffers[2] = { 0, 0 };
        #endif
        
        SDL_Texture* canvases[2] = { nullptr, nullptr };

        bool canvas_flip = false;

        int repeats = 1;

        std::vector<Amara::ShaderProgram*> shader_passes;

        ShaderContainer(): Amara::TextureContainer() {
            set_base_node_id("ShaderContainer");
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            auto it = config.find("shaderPasses");
            if (it != config.end()) {
                if (it->is_array()) {
                    for (nlohmann::json s: *it) {
                        addShaderPass(s);
                    }
                }
                else if (it->is_string()) {
                    addShaderPass(*it);
                }
            }
            it = config.find("shaderPass");
            if (it != config.end()) {
                if (it->is_string()) {
                    addShaderPass(*it);
                }
            }
            it = config.find("repeats");
            if (it != config.end() && it->is_number_integer() && *it > 0) {
                repeats = *it;
            }
            return Amara::TextureContainer::configure(config);
        }

        virtual void deletePipeline() override {
            Amara::TextureContainer::deletePipeline();

            if (canvases[1]) {
                SDL_DestroyTexture(canvases[1]);
                canvases[1] = nullptr;
            }
            #ifdef AMARA_OPENGL
            if (glCanvases[1] != 0) {
                glDeleteTextures(1, &glCanvases[1]);
                glCanvases[1] = 0;
            }
            if (glBuffers[1] != 0) {
                glDeleteFramebuffers(1, &glBuffers[1]);
                glBuffers[1] = 0;
            }
            #endif
        }

        virtual void createCanvas(int _w, int _h) override {
            Amara::TextureContainer::createCanvas(_w, _h);

            canvases[0] = canvasTexture;

            if (gameProps->graphics == GraphicsEnum::Render2D && gameProps->renderer) { 
                canvases[1] = SDL_CreateTexture(
                    gameProps->renderer,
                    SDL_PIXELFORMAT_RGBA32,
                    SDL_TEXTUREACCESS_TARGET,
                    _w,
                    _h
                );
                SDL_SetTextureBlendMode(canvases[0], SDL_BLENDMODE_NONE);
                SDL_SetTextureBlendMode(canvases[1], SDL_BLENDMODE_NONE);
            }
            
            #ifdef AMARA_OPENGL
            glBuffers[0] = glBufferID;
            glCanvases[0] = glCanvasID;

            if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                GLint prevBuffer = 0;
                glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevBuffer);

                glMakeFrameBuffer(glCanvases[1], glBuffers[1], width, height);
                glBindFramebuffer(GL_FRAMEBUFFER, glBuffers[1]);

                glBindFramebuffer(GL_FRAMEBUFFER, prevBuffer);
            }
            #endif
        }

        void drawPass() {
            if (gameProps->graphics == GraphicsEnum::Render2D && gameProps->renderer) {
                SDL_Texture* stamp_texture = (canvas_flip) ? canvases[1] : canvases[0];
                SDL_Texture* target_texture = (canvas_flip) ? canvases[0] : canvases[1];

                SDL_SetRenderTarget(gameProps->renderer, target_texture);
                SDL_SetRenderDrawColor(gameProps->renderer, 0, 0, 0, 0);
                SDL_RenderClear(gameProps->renderer);

                SDL_Rect setv = Rectangle::makeSDLRect(container_viewport);
                SDL_SetRenderViewport(gameProps->renderer, &setv);
                
                SDL_SetTextureScaleMode(stamp_texture, SDL_SCALEMODE_NEAREST);
                SDL_SetTextureColorMod(stamp_texture, 255, 255, 255);
                SDL_SetTextureAlphaMod(stamp_texture, 255);
                Apply_SDL_BlendMode(gameProps, stamp_texture, Amara::BlendMode::None);

                SDL_RenderTexture(gameProps->renderer, stamp_texture, NULL, NULL);
                return;
            }
            
            #ifdef AMARA_OPENGL
            if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                GLuint stamp_canvas = (canvas_flip) ? glCanvases[1] : glCanvases[0];
                GLuint target_buffer = (canvas_flip) ? glBuffers[0] : glBuffers[1];

                gameProps->renderBatch->flush();
                glBindFramebuffer(GL_FRAMEBUFFER, target_buffer);
                
                glViewport(0, 0, width, height);
                glClearColor(0, 0, 0, 0);
                glClear(GL_COLOR_BUFFER_BIT);

                SDL_FRect srcRect = Rectangle::makeSDLFRect(container_viewport);
                Quad srcQuad = Quad(
                    { srcRect.x/width, (srcRect.y+srcRect.h)/height },
                    { (srcRect.x+srcRect.w)/width, (srcRect.y+srcRect.h)/height },
                    { (srcRect.x+srcRect.w)/width, srcRect.y/height },
                    { srcRect.x/width, srcRect.y/height }
                );
                Quad destQuad = glTranslateQuad(
                    container_viewport,
                    Quad(container_viewport),
                    true
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
                    (Amara::BlendMode)SDL_BLENDMODE_NONE
                );
            }
            #endif
        }

        void addShaderPass(std::string shader_key) {
            #ifdef AMARA_OPENGL
            if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                ShaderProgram* prog = gameProps->shaders->getShaderProgram(shader_key);
                if (prog) {
                    shader_passes.push_back(prog);
                }
                else {
                    Amara::fatal_error("Error: ShaderProgram \"", shader_key, "\" not found.");
                }
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
                    for (nlohmann::json s: config) {
                        addShaderPass(s);
                    }
                }
            }
        }

        void swapCanvases() {
            canvas_flip = !canvas_flip;
            if (gameProps->graphics == GraphicsEnum::Render2D) {
                canvasTexture = (canvas_flip) ? canvases[1] : canvases[0];
            }
            #ifdef AMARA_OPENGL
            else if (gameProps->graphics == GraphicsEnum::OpenGL) {
                glBufferID = (canvas_flip) ? glBuffers[1] : glBuffers[0];
                glCanvasID = (canvas_flip) ? glCanvases[1] : glCanvases[0];
            }
            #endif
        }

        void drawCanvas(const Rectangle& v) {
            canvas_flip = true;

            SDL_Rect prevSDLViewport;
            
            if (gameProps->graphics == GraphicsEnum::Render2D && gameProps->renderer) {
                canvasTexture = canvases[0];
                SDL_SetTextureBlendMode(canvases[0], SDL_BLENDMODE_NONE);
                SDL_SetTextureBlendMode(canvases[1], SDL_BLENDMODE_NONE);
                SDL_GetRenderViewport(gameProps->renderer, &prevSDLViewport);
            }
            #ifdef AMARA_OPENGL
            GLint prevBuffer = 0;
            GLint prevViewport[4];
            ShaderProgram* originalShaderProgram = currentShaderProgram;
            if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                gameProps->renderBatch->flush();
                glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevBuffer);
                glGetIntegerv(GL_VIEWPORT, prevViewport);
                glBufferID = glBuffers[0];
                glCanvasID = glCanvases[0];
            }
            #endif

            PassOnProps originalPassOnProps = gameProps->passOn;

            Amara::TextureContainer::drawCanvas(v);

            PassOnProps new_props;
            new_props.insideTextureContainer = true;

            #ifdef AMARA_OPENGL
            for (int i = 0; i < repeats; i++) {
                for (Amara::ShaderProgram* prog: shader_passes) {
                    swapCanvases();
                    gameProps->passOn = new_props;
                    passOn = gameProps->passOn;
                    currentShaderProgram = prog;
                    drawPass();
                }
            }
            
            if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                if ((repeats * shader_passes.size()) % 2 != 0) {
                    swapCanvases();
                    currentShaderProgram = gameProps->defaultShaderProgram;
                    drawPass();
                }
            }
            #endif

            gameProps->passOn = originalPassOnProps;
            passOn = gameProps->passOn;

            if (gameProps->graphics == GraphicsEnum::Render2D && gameProps->renderer) {
                canvasTexture = (canvas_flip) ? canvases[0] : canvases[1];
                SDL_SetRenderViewport(gameProps->renderer, &prevSDLViewport);
            }
            #ifdef AMARA_OPENGL
            else if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                currentShaderProgram = originalShaderProgram;
                gameProps->renderBatch->flush();

                glBindFramebuffer(GL_FRAMEBUFFER, prevBuffer);
                glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
                
                glBufferID = (canvas_flip) ? glBuffers[0] : glBuffers[1];
                glCanvasID = (canvas_flip) ? glCanvases[0] : glCanvases[1];
            }
            #endif
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<ShaderContainer>("ShaderContainer",
                sol::base_classes, sol::bases<Amara::TextureContainer, Amara::Node>(),
                "repeats", &ShaderContainer::repeats,
                "shaderPasses", sol::property(
                    [](Amara::ShaderContainer& sc) -> sol::table {
                        if (sc.shader_passes.size() == 0) return sol::nil;
                        
                        sol::state_view lua = sc.gameProps->lua;
                        sol::table t = lua.create_table();
                        int index = 1;
                        for (Amara::ShaderProgram* prog: sc.shader_passes) {
                            t[index] = prog->key;
                            index += 1;
                        }
                        return t;
                    },
                    [](Amara::ShaderContainer& sc, sol::object val) {
                        sc.luaAddShaderPass(val);
                    }
                ),
                "shaderPass", sol::overload(
                    [](Amara::ShaderContainer& sc) -> sol::object {
                        if (sc.shader_passes.size() == 0) return sol::nil;
                        if (sc.shader_passes.size() == 1) return sol::make_object(sc.gameProps->lua, sc.shader_passes[0]);
                        
                        sol::state_view lua = sc.gameProps->lua;
                        sol::table t = lua.create_table();
                        int index = 1;
                        for (Amara::ShaderProgram* prog: sc.shader_passes) {
                            t[index] = prog->key;
                            index += 1;
                        }
                        return t;
                    },
                    [](Amara::ShaderContainer& sc, sol::object val) {
                        sc.luaAddShaderPass(val);
                    }
                )
            );
        }
    };
}