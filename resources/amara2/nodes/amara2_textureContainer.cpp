namespace Amara {
    class TextureContainer: public Amara::Node {
    public:
        SDL_Texture* texture = nullptr;
        SDL_GPUTexture* gpuTexture = nullptr;

        Amara::BlendMode blendMode = Amara::BlendMode::Alpha;
        Amara::Color tint = Amara::Color::White;

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
        
        int width = 0;
        int height = 0;
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

        Vector2 origin = { 0.5, 0.5 };

        Rectangle container_viewport;

        TextureContainer(): Amara::Node() {
            set_base_node_id("TextureContainer");
        }

        virtual nlohmann::json toJSON() {
            nlohmann::json data = Amara::Node::toJSON();

            data["tint"] = tint.toJSON();
            data["blendMode"] = static_cast<int>(blendMode);

            data["width"] = width;
            data["height"] = height;
            
            data["canvasLocked"] = canvasLocked;

            return data;
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "tint")) tint = config["tint"];
            if (json_has(config, "blendMode")) blendMode = static_cast<Amara::BlendMode>(config["blendMode"].get<int>());

            if (json_has(config, "w")) width = config["w"];
            if (json_has(config, "h")) height = config["h"];
            if (json_has(config, "width")) width = config["width"];
            if (json_has(config, "height")) height = config["height"];
            if (json_has(config, "canvasLocked")) canvasLocked = config["canvasLocked"];
            
            return Amara::Node::configure(config);
        }

        void deletePipeline() {
            if (canvasTexture) {
                SDL_DestroyTexture(canvasTexture);
                canvasTexture = nullptr;
            }
            #ifdef AMARA_OPENGL
            if (Props::graphics == GraphicsEnum::OpenGL && Props::glContext != NULL) {
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

            if (Props::graphics == GraphicsEnum::Render2D && Props::renderer) {
                canvasTexture = SDL_CreateTexture(
                    Props::renderer,
                    SDL_PIXELFORMAT_RGBA32,
                    SDL_TEXTUREACCESS_TARGET,
                    width,
                    height
                );
            }
            #ifdef AMARA_OPENGL
            else if (Props::graphics == GraphicsEnum::OpenGL && Props::glContext != NULL) {
                GLint prevBuffer = 0;
                glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevBuffer);

                glMakeFrameBuffer(glCanvasID, glBufferID, width, height);
                glBindFramebuffer(GL_FRAMEBUFFER, glBufferID);

                glBindFramebuffer(GL_FRAMEBUFFER, prevBuffer);
            }
            #endif

            container_viewport = { 0, 0, static_cast<float>(width), static_cast<float>(height) };

            left = -width/2.0;
            right = width/2.0;
            top = -height/2.0;
            bottom = height/2.0;

            update_canvas = true;
        }

        sol::object setOrigin(float _x, float _y) {
            origin = { _x, _y };
            return get_lua_object();
        }
        sol::object setOrigin(float _o) {
            return setOrigin(_o, _o);
        }

        virtual void drawCanvasContents(const Rectangle& v) {
            if (depthSortChildrenEnabled) sortChildren();
            drawChildren(v);
        }

        void drawCanvas(const Rectangle& v) {
            SDL_Texture* rec_target = nullptr;
            if (canvasTexture != nullptr && Props::renderer) {
                rec_target = SDL_GetRenderTarget(Props::renderer);
                SDL_SetRenderTarget(Props::renderer, canvasTexture);
                SDL_SetRenderDrawColor(Props::renderer, 0, 0, 0, 0);

                SDL_Rect setv = Rectangle::makeSDLRect(container_viewport);
                SDL_SetRenderViewport(Props::renderer, &setv);

                SDL_RenderClear(Props::renderer);
            }

            #ifdef AMARA_OPENGL
            GLint prevBuffer = 0;
            ShaderProgram* rec_shader = Props::currentShaderProgram;
            
            if (Props::graphics == GraphicsEnum::OpenGL && Props::glContext != NULL) {
                Props::renderBatch->flush();
                
                Props::currentShaderProgram = Props::defaultShaderProgram;
                
                glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevBuffer);
                glBindFramebuffer(GL_FRAMEBUFFER, glBufferID);
                
                glViewport(0, 0, width, height);
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT);
            }
            #endif
            
            drawCanvasContents(container_viewport);

            if (canvasTexture && Props::renderer) {
                SDL_SetRenderTarget(Props::renderer, rec_target);
                SDL_Rect setv = Rectangle::makeSDLRect(v);
                SDL_SetRenderViewport(Props::renderer, &setv);
            }
            #ifdef AMARA_OPENGL
            else if (Props::graphics == GraphicsEnum::OpenGL && Props::glContext != NULL) {
                Props::renderBatch->flush();

                Props::currentShaderProgram = rec_shader;

                glBindFramebuffer(GL_FRAMEBUFFER, prevBuffer);
                glViewport(v.x, Props::window_dim.h - v.y - v.h, v.w, v.h);
            }
            #endif
        }

        virtual void drawObjects(const Rectangle& v) override {
            if (fixedToCamera) {
                Props::passOn.reset();
            }
            passOn = Props::passOn;

            if (rec_width != width || rec_height != height) {
                createCanvas(width, height);
            }

            if (update_canvas || !canvasLocked) {
                drawCanvas(v);
                update_canvas = false;
            }

            #ifdef AMARA_OPENGL
            ShaderProgram* rec_shader = Props::currentShaderProgram;
            if (Props::graphics == GraphicsEnum::OpenGL && Props::glContext != NULL) {
                if (shaderProgram && shaderProgram != Props::currentShaderProgram) {
                    Props::currentShaderProgram = shaderProgram;
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
                    v.w + diag_distance*2, v.w + diag_distance*2
                )
            )) return;
            
            if (canvasTexture && Props::renderer) {
                SDL_SetTextureScaleMode(canvasTexture, SDL_SCALEMODE_NEAREST);
                SDL_SetTextureColorMod(canvasTexture, tint.r, tint.g, tint.b);
                SDL_SetTextureAlphaMod(canvasTexture, alpha * passOn.alpha * 255);
                setSDLBlendMode(canvasTexture, blendMode);

                SDL_RenderTextureRotated(
                    Props::renderer, 
                    canvasTexture,
                    &srcRect,
                    &destRect,
                    getDegrees(passOn.rotation + rotation),
                    &dorigin,
                    SDL_FLIP_NONE
                );
            }
            #ifdef AMARA_OPENGL
            else if (Props::graphics == GraphicsEnum::OpenGL && Props::glContext != NULL) {
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
                ), passOn.insideFrameBuffer);

                vertices = {
                    destQuad.p1.x, destQuad.p1.y, srcQuad.p1.x, srcQuad.p1.y,
                    destQuad.p2.x, destQuad.p2.y, srcQuad.p2.x, srcQuad.p2.y,
                    destQuad.p3.x, destQuad.p3.y, srcQuad.p3.x, srcQuad.p3.y,
                    destQuad.p4.x, destQuad.p4.y, srcQuad.p4.x, srcQuad.p4.y
                };

                Props::renderBatch->pushQuad(
                    Props::currentShaderProgram,
                    glCanvasID,
                    vertices, passOn.alpha * alpha, tint,
                    v, passOn.insideFrameBuffer,
                    blendMode
                );
            }
                
            if (rec_shader && shaderProgram && shaderProgram != rec_shader) {
                Props::currentShaderProgram = rec_shader;
            }
            #endif
        }

        virtual void drawChildren(const Rectangle& v) {
            children_copy_list = children;

            PassOnProps rec_props = Props::passOn;
            PassOnProps new_props;
            new_props.insideFrameBuffer = true;

            Props::passOn = new_props;

            Amara::Node* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = *it;
				if (child == nullptr || child->destroyed || !child->visible || child->parent != this) {
					++it;
					continue;
				}

                update_properties();
				child->draw(v);

                Props::passOn = new_props;
				++it;
			}

            Props::passOn = rec_props;
        }

        virtual void destroy() override {
            deletePipeline();
            Amara::Node::destroy();
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<TextureContainer>("TextureContainer",
                sol::base_classes, sol::bases<Node>(),
                "tint", &TextureContainer::tint,
                "blendMode", &TextureContainer::blendMode,
                "w", &TextureContainer::width,
                "h", &TextureContainer::height,
                "width", &TextureContainer::width,
                "height", &TextureContainer::height,
                "left", sol::readonly(&TextureContainer::left),
                "right", sol::readonly(&TextureContainer::right),
                "top", sol::readonly(&TextureContainer::top),
                "bottom", sol::readonly(&TextureContainer::bottom),
                "origin", &TextureContainer::origin,
                "setOrigin", sol::overload(
                    sol::resolve<sol::object(float, float)>(&TextureContainer::setOrigin),
                    sol::resolve<sol::object(float)>(&TextureContainer::setOrigin)
                )
            );
        }
    };
}