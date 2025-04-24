namespace Amara {
    class TextureContainer: public Amara::Node {
    public:
        SDL_Texture* texture = nullptr;
        SDL_GPUTexture* gpuTexture = nullptr;

        Amara::BlendMode blendMode = Amara::BlendMode::Alpha;
        Amara::Color tint = Amara::Color::White;

        Amara::Color fill = Amara::Color::Transparent;

        #ifdef AMARA_OPENGL
        GLuint glCanvasID = 0;
        GLuint glBufferID = 0;

        std::array<float, 16> vertices = {
            -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
             0.5f, -0.5f,  1.0f, 0.0f, // Bottom-right
             0.5f,  0.5f,  1.0f, 1.0f, // Top-right
            -0.5f,  0.5f,  0.0f, 1.0f  // Top-left
        };
        #endif

        SDL_Texture* canvasTexture = nullptr;
        
        int width =  128;
        int height = 128;
        int rec_width = -1;
        int rec_height = -1;

        float left = 0;
        float right = 0;
        float top = 0;
        float bottom = 0;

        int cropLeft = 0;
        int cropRight = 0;
        int cropTop = 0;
        int cropBottom = 0;

        bool canvasLocked = false;
        bool update_canvas = false;

        bool clearOnDraw = true;

        Vector2 origin = { 0.5, 0.5 };

        Rectangle container_viewport;

        TextureContainer(): Amara::Node() {
            set_base_node_id("TextureContainer");
        }

        virtual nlohmann::json toJSON() override {
            nlohmann::json data = Amara::Node::toJSON();

            data["tint"] = tint.toJSON();
            data["blendMode"] = static_cast<int>(blendMode);

            data["width"] = width;
            data["height"] = height;

            data["originX"] = origin.x;
            data["originY"] = origin.y;

            data["cropLeft"] = cropLeft;
            data["cropRight"] = cropRight;
            data["cropTop"] = cropTop;
            data["cropBottom"] = cropBottom;
            
            data["canvasLocked"] = canvasLocked;

            return data;
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "tint")) tint = config["tint"];
            if (json_has(config, "fill")) fill = config["fill"];
            if (json_has(config, "blendMode")) blendMode = static_cast<Amara::BlendMode>(config["blendMode"].get<int>());

            if (json_has(config, "w")) width = config["w"];
            if (json_has(config, "h")) height = config["h"];
            if (json_has(config, "width")) width = config["width"];
            if (json_has(config, "height")) height = config["height"];
            if (json_has(config, "canvasLocked")) canvasLocked = config["canvasLocked"];

            if (json_has(config, "originX")) origin.x = config["originX"];
            if (json_has(config, "originY")) origin.y = config["originY"];
            if (json_has(config, "origin")) origin = config["origin"];

            if (json_has(config, "cropLeft")) cropLeft = config["cropLeft"];
            if (json_has(config, "cropRight")) cropRight = config["cropRight"];
            if (json_has(config, "cropTop")) cropTop = config["cropTop"];
            if (json_has(config, "cropBottom")) cropBottom = config["cropBottom"];

            if (json_has(config, "clearOnDraw")) clearOnDraw = config["clearOnDraw"];

            update_size();
            
            return Amara::Node::configure(config);
        }

        void deletePipeline() {
            if (canvasTexture) {
                SDL_DestroyTexture(canvasTexture);
                canvasTexture = nullptr;
            }
            #ifdef AMARA_OPENGL
            if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                if (glCanvasID != 0) {
                    glDeleteTextures(1, &glCanvasID);
                    glCanvasID = 0;
                }
                if (glBufferID != 0) {
                    glDeleteFramebuffers(1, &glBufferID);
                    glBufferID = 0;
                }
            }
            #endif
        }

        void createCanvas(int _w, int _h) {
            width = rec_width = _w;
            height = rec_height = _h;

            deletePipeline();

            if (gameProps->graphics == GraphicsEnum::Render2D && gameProps->renderer) {
                canvasTexture = SDL_CreateTexture(
                    gameProps->renderer,
                    SDL_PIXELFORMAT_RGBA32,
                    SDL_TEXTUREACCESS_TARGET,
                    width,
                    height
                );
            }
            #ifdef AMARA_OPENGL
            else if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                GLint prevBuffer = 0;
                glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevBuffer);

                glMakeFrameBuffer(glCanvasID, glBufferID, width, height);
                glBindFramebuffer(GL_FRAMEBUFFER, glBufferID);

                glBindFramebuffer(GL_FRAMEBUFFER, prevBuffer);
            }
            #endif

            container_viewport = Rectangle( 0, 0, static_cast<float>(width), static_cast<float>(height) );

            update_size();

            update_canvas = true;
        }

        Rectangle getCanvasSize() {
            return Rectangle(0, 0, static_cast<float>(width), static_cast<float>(height));
        }

        void setCanvasSize(const Rectangle& rect) {
            width = rect.w;
            height = rect.h;
            pos.x = rect.x + width*origin.x;
            pos.y = rect.y + height*origin.y;
        }

        void update_size() {
            left = -width/2.0;
            right = width/2.0;
            top = -height/2.0;
            bottom = height/2.0;
        }

        void paintOnce() {
            update_canvas = true;
            canvasLocked = true;
        }

        virtual void drawCanvasContents(const Rectangle& v) {
            if (depthSortChildrenEnabled) sortChildren();
            drawChildren(v);
        }

        void drawCanvas(const Rectangle& v) {
            SDL_Texture* rec_target = nullptr;
            if (canvasTexture != nullptr && gameProps->renderer) {
                rec_target = SDL_GetRenderTarget(gameProps->renderer);
                SDL_SetRenderTarget(gameProps->renderer, canvasTexture);
                SDL_SetRenderDrawColor(gameProps->renderer, 0, 0, 0, 0);

                SDL_Rect setv = Rectangle::makeSDLRect(container_viewport);
                SDL_SetRenderViewport(gameProps->renderer, &setv);

                if (clearOnDraw) SDL_RenderClear(gameProps->renderer);
            }

            #ifdef AMARA_OPENGL
            GLint prevBuffer = 0;
            ShaderProgram* rec_shader = gameProps->currentShaderProgram;
            
            if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                gameProps->renderBatch->flush();
                
                gameProps->currentShaderProgram = gameProps->defaultShaderProgram;
                
                glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevBuffer);
                glBindFramebuffer(GL_FRAMEBUFFER, glBufferID);
                
                glViewport(0, 0, width, height);
                if (clearOnDraw) {
                    glClearColor(
                        fill.r / 255.0f,
                        fill.g / 255.0f,
                        fill.b / 255.0f,
                        fill.a / 255.0f
                    );
                }
                glClear(GL_COLOR_BUFFER_BIT);
            }
            #endif
            
            drawCanvasContents(container_viewport);

            if (canvasTexture && gameProps->renderer) {
                SDL_SetRenderTarget(gameProps->renderer, rec_target);
                SDL_Rect setv = Rectangle::makeSDLRect(v);
                SDL_SetRenderViewport(gameProps->renderer, &setv);
            }
            #ifdef AMARA_OPENGL
            else if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                gameProps->renderBatch->flush();

                gameProps->currentShaderProgram = rec_shader;

                glBindFramebuffer(GL_FRAMEBUFFER, prevBuffer);
                glViewport(v.x, gameProps->window_dim.h - v.y - v.h, v.w, v.h);
            }
            #endif
        }

        virtual void drawObjects(const Rectangle& v) override {
            if (fixedToCamera && !gameProps->passOn.insideTextureContainer) {
                gameProps->passOn.reset();
            }
            passOn = gameProps->passOn;

            if (rec_width != width || rec_height != height) {
                createCanvas(width, height);
            }

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

            Vector2 vcenter = { v.w/2.0f, v.h/2.0f };
            Vector2 totalZoom = { passOn.zoom.x*passOn.window_zoom.x, passOn.zoom.y*passOn.window_zoom.y };

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

            Rectangle dim = {
                anchoredPos.x + (cropLeft - width*origin.x)*scale.x*passOn.scale.x, 
                anchoredPos.y - anchoredPos.z + (cropTop - height*origin.y)*scale.y*passOn.scale.y,
                (width - cropLeft - cropRight)*scale.x*passOn.scale.x,
                (height - cropTop - cropBottom)*scale.y*passOn.scale.y
            };
            
            destRect.x = vcenter.x + dim.x*totalZoom.x;
            destRect.y = vcenter.y + dim.y*totalZoom.y;
            destRect.w = dim.w * totalZoom.x;
            destRect.h = dim.h * totalZoom.y;

            SDL_FPoint dorigin = {
                (width*origin.x - cropLeft)*scale.x*passOn.scale.x*totalZoom.x,
                (height*origin.y - cropTop)*scale.y*passOn.scale.y*totalZoom.y
            };
            
            srcRect = {
                static_cast<float>(cropLeft),
                static_cast<float>(cropTop),
                static_cast<float>(width - cropLeft - cropRight),
                static_cast<float>(height - cropTop - cropBottom)
            };

            float diag_distance = distanceBetween(0, 0, destRect.w, destRect.h);
            if (!Shape::checkCollision(
                Rectangle(destRect), Rectangle(
                    v.x - diag_distance, v.y - diag_distance,
                    v.w + diag_distance*2, v.h + diag_distance*2
                )
            )) return;
            
            if (canvasTexture && gameProps->renderer) {
                SDL_SetTextureScaleMode(canvasTexture, SDL_SCALEMODE_NEAREST);
                SDL_SetTextureColorMod(canvasTexture, tint.r, tint.g, tint.b);
                SDL_SetTextureAlphaMod(canvasTexture, alpha * passOn.alpha * 255);
                setSDLBlendMode(canvasTexture, blendMode);

                SDL_RenderTextureRotated(
                    gameProps->renderer, 
                    canvasTexture,
                    &srcRect,
                    &destRect,
                    getDegrees(passOn.rotation + rotation),
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

        virtual void drawChildren(const Rectangle& v) {
            children_copy_list = children;

            PassOnProps rec_props = gameProps->passOn;
            PassOnProps new_props;
            new_props.insideTextureContainer = true;

            gameProps->passOn = new_props;

            Amara::Node* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = *it;
				if (child == nullptr || child->destroyed || !child->visible || child->parent != this) {
					++it;
					continue;
				}

                update_properties();
				child->draw(v);

                gameProps->passOn = new_props;
				++it;
			}

            gameProps->passOn = rec_props;
        }

        virtual void destroy() override {
            deletePipeline();
            Amara::Node::destroy();
        }

        Rectangle getRectangle() {
            return Rectangle(
                pos.x - (width*scale.x)*origin.x,
                pos.y - (height*scale.y)*origin.y,
                width*scale.x,
                height*scale.y
            );
        }
        
        Rectangle stretchTo(const Rectangle& rect) {
            rotation = 0;
            pos.x = rect.x + rect.w*origin.x;
            pos.y = rect.y + rect.h*origin.y;
            scale.x = rect.w / static_cast<float>(width);
            scale.y = rect.h / static_cast<float>(height);
            return rect;
        }

        sol::object fitWithin(const Rectangle& rect) {
            if (rect.w == 0 || rect.h == 0) return get_lua_object();

            rotation = 0;

            float horFactor = rect.w / static_cast<float>(width);
            float verFactor = rect.h / static_cast<float>(height);
            
            if (horFactor < verFactor) {
                scale.x = horFactor;
                scale.y = horFactor;
            }
            else {
                scale.x = verFactor;
                scale.y = verFactor;
            }

            float scaledWidth  = width  * scale.x;
            float scaledHeight = height * scale.y;

            pos.x = rect.x + (rect.w - scaledWidth)/2 + scaledWidth*origin.x;
            pos.y = rect.y + (rect.h - scaledHeight)/2 + scaledHeight*origin.y;
            
            return get_lua_object();
        }

        Vector2 getCenter() {
            return getRectangle().getCenter();
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<TextureContainer>("TextureContainer",
                sol::base_classes, sol::bases<Node>(),
                "tint", sol::property([](Amara::TextureContainer& t) -> Amara::Color { return t.tint; }, [](Amara::TextureContainer& t, sol::object v) { t.tint = v; }),
                "fill", sol::property([](Amara::TextureContainer& t) -> Amara::Color { return t.fill; }, [](Amara::TextureContainer& t, sol::object v) { t.fill = v; }),
                "blendMode", &TextureContainer::blendMode,
                "w", &TextureContainer::width,
                "h", &TextureContainer::height,
                "width", &TextureContainer::width,
                "height", &TextureContainer::height,
                "canvas", sol::property(&TextureContainer::getCanvasSize, &TextureContainer::setCanvasSize),
                "rect", sol::property(&TextureContainer::getRectangle, &TextureContainer::stretchTo),
                "stretchTo", &TextureContainer::stretchTo,
                "fitWithin", &TextureContainer::fitWithin,
                "center", sol::property(&TextureContainer::getCenter),
                "cropLeft", &TextureContainer::cropLeft,
                "cropRight", &TextureContainer::cropRight,
                "cropTop", &TextureContainer::cropTop,
                "cropBottom", &TextureContainer::cropBottom,
                "left", sol::readonly(&TextureContainer::left),
                "right", sol::readonly(&TextureContainer::right),
                "top", sol::readonly(&TextureContainer::top),
                "bottom", sol::readonly(&TextureContainer::bottom),
                "origin", &TextureContainer::origin,
                "originX", sol::property([](Amara::TextureContainer& t) -> float { return t.origin.x; }, [](Amara::TextureContainer& t, float v) { t.origin.x = v; }),
                "originY", sol::property([](Amara::TextureContainer& t) -> float { return t.origin.y; }, [](Amara::TextureContainer& t, float v) { t.origin.y = v; }),
                "canvasLocked", &TextureContainer::canvasLocked,
                "paintOnce", &TextureContainer::paintOnce,
                "clearOnDraw", &TextureContainer::clearOnDraw
            );
        }
    };
}