namespace Amara {
    class NineSlice: public Amara::TextureContainer {
    public:
        NineSlice(): Amara::TextureContainer() {
            set_base_node_id("NineSlice");
        }

        Amara::ImageAsset* image = nullptr;
        Amara::SpritesheetAsset* spritesheet = nullptr;

        int drawWidth = 128;
        int drawHeight = 128;

        int frame = 0;

        int textureWidth = 0;
        int textureHeight = 0;
        
        int frameWidth = 0;
        int frameHeight = 0;

        float marginLeft = -1;
        float marginRight = -1;
        float marginTop = -1;
        float marginBottom = -1;

        int extrusion = 1;

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "width")) drawWidth = json_extract(config, "width");
            if (json_has(config, "height")) drawHeight = json_extract(config, "height");

            if (drawWidth < 0) drawWidth = 0;
            if (drawHeight < 0) drawHeight = 0;

            if (json_has(config, "maxWidth")) width = json_extract(config, "maxWidth");
            if (json_has(config, "maxHeight")) height = json_extract(config, "maxHeight");

            if (width < drawWidth) width = drawWidth;
            if (height < drawHeight) height = drawHeight;

            if (json_has(config, "texture")) setTexture(config["texture"]);
            if (json_has(config, "frame")) frame = config["frame"];
            
            if (json_has(config, "marginLeft")) marginLeft = json_extract(config, "marginLeft");
            if (json_has(config, "marginRight")) marginRight = json_extract(config, "marginRight");
            if (json_has(config, "marginTop")) marginTop = json_extract(config, "marginTop");
            if (json_has(config, "marginBottom")) marginBottom = json_extract(config, "marginBottom");

            return Amara::TextureContainer::configure(config);
        }

        bool setTexture(std::string key) {
            image = nullptr;
            spritesheet = nullptr;

            if (!gameProps->assets->has(key)) {
                debug_log("Error: Asset \"", key, "\" was not found.");
                return false;
            }

            image = gameProps->assets->get(key)->as<ImageAsset*>();
            
            if (image == nullptr) {
                debug_log("Error: Asset \"", key, "\" is not a valid texture asset.");
                gameProps->breakWorld();
                return false;
            }
            textureWidth = image->width;
            textureHeight = image->height;

            spritesheet = image->as<SpritesheetAsset*>();
            if (spritesheet) {
                frameWidth = spritesheet->frameWidth;
                frameHeight = spritesheet->frameHeight;
            }
            else {
                frameWidth = 0;
                frameHeight = 0;
            }

            return true;
        }

        void drawSlice(const Rectangle& v, const SDL_FRect& srcRect, const SDL_FRect& destRect) {
            if (image == nullptr) return;

            if (image->texture && gameProps->renderer) {
                SDL_SetTextureScaleMode(image->texture, SDL_SCALEMODE_NEAREST);
                SDL_SetTextureColorMod(image->texture, 255, 255, 255);
                SDL_SetTextureAlphaMod(image->texture, 255);
                Apply_SDL_BlendMode(gameProps, image->texture, Amara::BlendMode::None);
 
                SDL_RenderTexture(
                    gameProps->renderer, 
                    image->texture,
                    &srcRect,
                    &destRect
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
                    Vector2( destRect.x,destRect.y ),
                    0
                ), true);

                vertices = {
                    destQuad.p1.x, destQuad.p1.y, srcQuad.p1.x, srcQuad.p1.y,
                    destQuad.p2.x, destQuad.p2.y, srcQuad.p2.x, srcQuad.p2.y,
                    destQuad.p3.x, destQuad.p3.y, srcQuad.p3.x, srcQuad.p3.y,
                    destQuad.p4.x, destQuad.p4.y, srcQuad.p4.x, srcQuad.p4.y
                };

                gameProps->renderBatch->pushQuad(
                    gameProps->currentShaderProgram,
                    image->glTextureID,
                    vertices, 1.0f, Color::White,
                    v, true,
                    Amara::BlendMode::None
                );
            }
            #endif
        }

        virtual void drawCanvasContents(const Rectangle& v) override {
            if (image == nullptr) return;

            float logicalFrameW = (spritesheet && spritesheet->frameWidth > 0) ? spritesheet->frameWidth : image->width;
            float logicalFrameH = (spritesheet && spritesheet->frameHeight > 0) ? spritesheet->frameHeight : image->height;

            float frameAtlasX = 0;
            float frameAtlasY = 0;

            if (spritesheet && spritesheet->frameWidth > 0 && spritesheet->frameHeight > 0) {
                if (textureWidth > 0 && logicalFrameW > 0) { 
                    int framesPerRow = textureWidth / logicalFrameW;
                    if (framesPerRow == 0) framesPerRow = 1; 
                    
                    int numFrames = static_cast<int>(floor(static_cast<float>(textureWidth) / logicalFrameW) * floor(static_cast<float>(textureHeight) / logicalFrameH));
                    if (numFrames == 0) numFrames = 1;
                    int fixedFrame = frame % numFrames;

                    frameAtlasX = static_cast<float>((fixedFrame % framesPerRow) * logicalFrameW);
                    frameAtlasY = static_cast<float>(floor(static_cast<float>(fixedFrame) / framesPerRow) * logicalFrameH);
                }
            }

            float actualML = (marginLeft < 0) ? logicalFrameW / 3.0f : marginLeft;
            float actualMR = (marginRight < 0) ? logicalFrameW / 3.0f : marginRight;
            float actualMT = (marginTop < 0) ? logicalFrameH / 3.0f : marginTop;
            float actualMB = (marginBottom < 0) ? logicalFrameH / 3.0f : marginBottom;

            if (actualML + actualMR > logicalFrameW) {
                float sum = actualML + actualMR;
                if (sum == 0) { 
                    actualML = logicalFrameW / 2.0f; 
                    actualMR = logicalFrameW / 2.0f; 
                }
                else {
                    actualML = (actualML / sum) * logicalFrameW;
                    actualMR = (actualMR / sum) * logicalFrameW;
                }
            }
            if (actualMT + actualMB > logicalFrameH) {
                float sum = actualMT + actualMB;
                if (sum == 0) { 
                    actualMT = logicalFrameH / 2.0f;
                    actualMB = logicalFrameH / 2.0f;
                }
                else {
                    actualMT = (actualMT / sum) * logicalFrameH;
                    actualMB = (actualMB / sum) * logicalFrameH;
                }
            }

            float destML = actualML;
            float destMR = actualMR;
            float destMT = actualMT;
            float destMB = actualMB;

            float srcHorStretch = logicalFrameW - (actualML + actualMR);
            float srcVerStretch = logicalFrameH - (actualMT + actualMB);

            float destHorStretch = drawWidth - (actualML + actualMR); 
            float destVerStretch = drawHeight - (actualMT + actualMB); 

            if (destHorStretch < 0) {
                float sumHorMargins = actualML + actualMR;
                if (sumHorMargins > 0) {
                    destML = drawWidth * (actualML / sumHorMargins);
                    destMR = drawWidth * (actualMR / sumHorMargins);
                }
                else {
                    destML = drawWidth / 2.0f;
                    destMR = drawWidth / 2.0f;
                }
                destHorStretch = 0;
            }

            if (destVerStretch < 0) {
                float sumVerMargins = actualMT + actualMB;
                if (sumVerMargins > 0) {
                    destMT = drawHeight * (actualMT / sumVerMargins);
                    destMB = drawHeight * (actualMB / sumVerMargins);
                } else {
                    destMT = drawHeight / 2.0f;
                    destMB = drawHeight / 2.0f;
                }
                destVerStretch = 0;
            }
            
            if (srcHorStretch < 0) srcHorStretch = 0;
            if (srcVerStretch < 0) srcVerStretch = 0;
            SDL_FRect src, dest;

            // Top-Left
            src  = { frameAtlasX, frameAtlasY, actualML, actualMT };
            dest = { 0, 0, destML, destMT };
            if (src.w > 0 && src.h > 0 && dest.w > 0 && dest.h > 0) drawSlice(v, src, dest);
            // Top-Center
            src  = { frameAtlasX + actualML, frameAtlasY, srcHorStretch, actualMT };
            dest = { destML, 0, destHorStretch, destMT };
            if (src.w > 0 && src.h > 0 && dest.w > 0 && dest.h > 0) drawSlice(v, src, dest);
            // Top-Right
            src  = { frameAtlasX + actualML + srcHorStretch, frameAtlasY, actualMR, actualMT };
            dest = { destML + destHorStretch, 0, destMR, destMT };
            if (src.w > 0 && src.h > 0 && dest.w > 0 && dest.h > 0) drawSlice(v, src, dest);
            // Middle-Left
            src  = { frameAtlasX, frameAtlasY + actualMT, actualML, srcVerStretch };
            dest = { 0, destMT, destML, destVerStretch };
            if (src.w > 0 && src.h > 0 && dest.w > 0 && dest.h > 0) drawSlice(v, src, dest);
            // Center
            src  = { frameAtlasX + actualML, frameAtlasY + actualMT, srcHorStretch, srcVerStretch };
            dest = { destML, destMT, destHorStretch, destVerStretch };
            if (src.w > 0 && src.h > 0 && dest.w > 0 && dest.h > 0) drawSlice(v, src, dest);
            // Middle-Right
            src  = { frameAtlasX + actualML + srcHorStretch, frameAtlasY + actualMT, actualMR, srcVerStretch };
            dest = { destML + destHorStretch, destMT, destMR, destVerStretch };
            if (src.w > 0 && src.h > 0 && dest.w > 0 && dest.h > 0) drawSlice(v, src, dest);
            // Bottom-Left
            src  = { frameAtlasX, frameAtlasY + actualMT + srcVerStretch, actualML, actualMB };
            dest = { 0, destMT + destVerStretch, destML, destMB };
            if (src.w > 0 && src.h > 0 && dest.w > 0 && dest.h > 0) drawSlice(v, src, dest);
            // Bottom-Center
            src  = { frameAtlasX + actualML, frameAtlasY + actualMT + srcVerStretch, srcHorStretch, actualMB };
            dest = { destML, destMT + destVerStretch, destHorStretch, destMB };
            if (src.w > 0 && src.h > 0 && dest.w > 0 && dest.h > 0) drawSlice(v, src, dest);
            // Bottom-Right
            src  = { frameAtlasX + actualML + srcHorStretch, frameAtlasY + actualMT + srcVerStretch, actualMR, actualMB };
            dest = { destML + destHorStretch, destMT + destVerStretch, destMR, destMB };
            if (src.w > 0 && src.h > 0 && dest.w > 0 && dest.h > 0) drawSlice(v, src, dest);
        }

        virtual void drawCanvas(const Rectangle& v) override {
            SDL_Texture* rec_target = nullptr;
            if (canvasTexture != nullptr && gameProps->renderer) {
                rec_target = SDL_GetRenderTarget(gameProps->renderer);
                SDL_SetRenderTarget(gameProps->renderer, canvasTexture);
                if (clearOnDraw) SDL_SetRenderDrawColor(gameProps->renderer, fill.r, fill.g, fill.b, fill.a);
                else SDL_SetRenderDrawColor(gameProps->renderer, 0,0,0,0);

                SDL_Rect setv = { 0, 0, TextureContainer::width, TextureContainer::height };
                SDL_SetRenderViewport(gameProps->renderer, &setv);

                if (clearOnDraw) SDL_RenderClear(gameProps->renderer);
            }
            #ifdef AMARA_OPENGL
            GLint prevBuffer = 0;
            ShaderProgram* rec_shader = gameProps->currentShaderProgram;

            if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL && glBufferID != 0) {
                GLint prevBuffer = 0;
                ShaderProgram* rec_shader = gameProps->currentShaderProgram;
                
                if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                    gameProps->renderBatch->flush();
                    
                    gameProps->currentShaderProgram = gameProps->defaultShaderProgram;
                    
                    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevBuffer);
                    glBindFramebuffer(GL_FRAMEBUFFER, glBufferID);
                    
                    glViewport(0, 0, TextureContainer::width, TextureContainer::height);
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
            }
            #endif

            drawCanvasContents(TextureContainer::container_viewport);

            if (canvasTexture && gameProps->renderer) {
                SDL_SetRenderTarget(gameProps->renderer, rec_target);
                SDL_Rect setv = Rectangle::makeSDLRect(v);
                SDL_SetRenderViewport(gameProps->renderer, &setv);
            }
            #ifdef AMARA_OPENGL
            else if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL && glBufferID != 0) {
                gameProps->renderBatch->flush();
                gameProps->currentShaderProgram = rec_shader;

                glBindFramebuffer(GL_FRAMEBUFFER, prevBuffer);
                glViewport(v.x, gameProps->window_dim.h - v.y - v.h, v.w, v.h);
            }
            #endif
        }
        
        virtual SDL_FRect getSrcRect() override {
            return {
                static_cast<float>(cropLeft),
                static_cast<float>(cropTop),
                static_cast<float>(drawWidth - cropLeft - cropRight),
                static_cast<float>(drawHeight - cropTop - cropBottom)
            };
        }

        virtual void drawObjects(const Rectangle& v) override {
            if (fixedToCamera && !gameProps->passOn.insideTextureContainer) {
                gameProps->passOn.reset();
            }
            passOn = gameProps->passOn;
            
            if (drawWidth > TextureContainer::width) drawWidth = TextureContainer::width;
            if (drawHeight > TextureContainer::height) drawHeight = TextureContainer::height;
            if (drawWidth < 0) drawWidth = 0;
            if (drawHeight < 0) drawHeight = 0;

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
                anchoredPos.x + (cropLeft - drawWidth*origin.x)*scale.x*passOn.scale.x, 
                anchoredPos.y - anchoredPos.z + (cropTop - drawHeight*origin.y)*scale.y*passOn.scale.y,
                (drawWidth - cropLeft - cropRight)*scale.x*passOn.scale.x,
                (drawHeight - cropTop - cropBottom)*scale.y*passOn.scale.y
            };
            
            destRect.x = vcenter.x + dim.x*totalZoom.x;
            destRect.y = vcenter.y + dim.y*totalZoom.y;
            destRect.w = dim.w * totalZoom.x;
            destRect.h = dim.h * totalZoom.y;

            SDL_FPoint dorigin = {
                (drawWidth*origin.x - cropLeft)*scale.x*passOn.scale.x*totalZoom.x,
                (drawHeight*origin.y - cropTop)*scale.y*passOn.scale.y*totalZoom.y
            };
            
            srcRect = getSrcRect();

            float diag_distance = distanceBetween(0, 0, destRect.w, destRect.h);
            if (!Shape::collision(
                Rectangle(destRect), Rectangle(
                    v.x - diag_distance, v.y - diag_distance,
                    v.w + diag_distance*2, v.h + diag_distance*2
                )
            )) return;

            if (input.active) {
                Quad inputQuad = rotateQuad(
                    Quad(destRect),
                    Vector2(
                        destRect.x + dorigin.x,
                        destRect.y + dorigin.y
                    ),
                    passOn.rotation + rotation
                );
                input.queueInput(inputQuad);
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
            #endif

            if (depthSortChildrenEnabled) sortChildren();
            drawChildren(v);

            #ifdef AMARA_OPENGL                
            if (rec_shader && shaderProgram && shaderProgram != rec_shader) {
                gameProps->currentShaderProgram = rec_shader;
            }
            #endif
        }

        virtual void drawChildren(const Rectangle v) {
            Amara::Node::drawChildren(v);
        }

        void setDrawWidth(double _w) {
            drawWidth = _w;
            if (drawWidth < 0) drawWidth = 0;
            if (drawWidth > TextureContainer::width) {
                drawWidth = TextureContainer::width;
            }
            update_canvas = true;
        }
        void setDrawHeight(double _h) {
            drawHeight = _h;
            if (drawHeight < 0) drawHeight = 0;
            if (drawHeight > TextureContainer::height) {
                drawHeight = TextureContainer::height;
            }
            update_canvas = true;
        }

        Rectangle getRectangle() {
            return Rectangle(
                pos.x - (drawWidth*scale.x)*origin.x,
                pos.y - (drawHeight*scale.y)*origin.y,
                drawWidth*scale.x,
                drawHeight*scale.y
            );
        }

        Rectangle resize(const Rectangle& rect) {
            rotation = 0;
            pos.x = rect.x + rect.w*origin.x;
            pos.y = rect.y + rect.h*origin.y;
            drawWidth = rect.w;
            drawHeight = rect.h;
            return getRectangle();
        }

        Rectangle stretchTo(const Rectangle& rect) {
            rotation = 0;
            pos.x = rect.x + rect.w*origin.x;
            pos.y = rect.y + rect.h*origin.y;
            scale.x = rect.w / static_cast<float>(drawWidth);
            scale.y = rect.h / static_cast<float>(drawHeight);
            return rect;
        }
        
        sol::object fitWithin(const Rectangle& rect) {
            if (rect.w == 0 || rect.h == 0) return get_lua_object();

            rotation = 0;

            float horFactor = rect.w / static_cast<float>(drawWidth);
            float verFactor = rect.h / static_cast<float>(drawHeight);
            
            if (horFactor < verFactor) {
                scale.x = horFactor;
                scale.y = horFactor;
            }
            else {
                scale.x = verFactor;
                scale.y = verFactor;
            }

            float scaledWidth  = drawWidth  * scale.x;
            float scaledHeight = drawHeight * scale.y;

            pos.x = rect.x + (rect.w - scaledWidth)/2 + scaledWidth*origin.x;
            pos.y = rect.y + (rect.h - scaledHeight)/2 + scaledHeight*origin.y;
            
            return get_lua_object();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::NineSlice>("NineSlice",
                sol::base_classes, sol::bases<Amara::TextureContainer, Amara::Node>(),
                "maxWidth", sol::property([](Amara::NineSlice& n) -> int { return n.width; }, [](Amara::NineSlice& n, double v) { n.setWidth(v); }),
                "maxHeight", sol::property([](Amara::NineSlice& n) -> int { return n.height; }, [](Amara::NineSlice& n, double v) { n.setHeight(v); }),
                "w", sol::property([](Amara::NineSlice& n) -> int { return n.drawWidth; }, [](Amara::NineSlice& n, double v) { n.setDrawWidth(v); }),
                "h", sol::property([](Amara::NineSlice& n) -> int { return n.drawHeight; }, [](Amara::NineSlice& n, double v) { n.setDrawHeight(v); }),
                "width", sol::property([](Amara::NineSlice& n) -> int { return n.drawWidth; }, [](Amara::NineSlice& n, double v) { n.setDrawWidth(v); }),
                "height", sol::property([](Amara::NineSlice& n) -> int { return n.drawHeight; }, [](Amara::NineSlice& n, double v) { n.setDrawHeight(v); }),
                "marginLeft", &Amara::NineSlice::marginLeft,
                "marginRight", &Amara::NineSlice::marginRight,
                "marginTop", &Amara::NineSlice::marginTop,
                "marginBottom", &Amara::NineSlice::marginBottom,
                "texture", sol::property([](Amara::NineSlice& t) -> std::string { return t.image ? t.image->key : ""; }, [](Amara::NineSlice& t, std::string v) { t.setTexture(v); }),
                "rect", sol::property([](Amara::NineSlice& t) -> Rectangle { return t.getRectangle(); }, [](Amara::NineSlice& t, Rectangle v) { t.resize(v); }),
                "size", sol::property([](Amara::NineSlice& t) -> Rectangle { return Rectangle(t.pos.x, t.pos.y, t.drawWidth, t.drawHeight); }, &NineSlice::resize),
                "resize", &Amara::NineSlice::resize,
                "stretchTo", &Amara::NineSlice::stretchTo,
                "fitWithin", &Amara::NineSlice::fitWithin
            );
        }
    };
}