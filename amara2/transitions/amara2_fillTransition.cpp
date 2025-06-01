namespace Amara {
    class FillTransition: public Amara::Transition {
    public:
        Amara::StateMachine sm;

        Amara::Color tint = Amara::Color::Black;
        Amara::BlendMode blendMode = Amara::BlendMode::Alpha;

        float fadeIn = 1;
        float fadeOut = 1;

        int textureWidth = 1;
        int textureHeight = 1;

        Amara::ImageAsset* image = nullptr;

        #ifdef AMARA_OPENGL
        std::array<float, 16> vertices = {
            -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
             0.5f, -0.5f,  1.0f, 0.0f, // Bottom-right
             0.5f,  0.5f,  1.0f, 1.0f, // Top-right
            -0.5f,  0.5f,  0.0f, 1.0f  // Top-left
        };
        #endif

        FillTransition(): Amara::Transition() {
            set_base_node_id("FillTransition");
        }

        virtual void init() override {
            Amara::Transition::init();
            sm.gameProps = gameProps;
            image = gameProps->assets->whitePixel;
            alpha = 0;
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "color")) tint = json_extract(config, "color");
            if (json_has(config, "tint")) tint = json_extract(config, "tint");
            if (json_has(config, "blendMode")) blendMode = json_extract(config, "blendMode");
            if (json_has(config, "fadeIn")) fadeIn = json_extract(config, "fadeIn");
            if (json_has(config, "fadeOut")) fadeOut = json_extract(config, "fadeOut");
            return Amara::Transition::configure(config);
        }

        virtual void drawSelf(const Rectangle& v) override {
            if (image == nullptr) return;

            SDL_FRect srcRect, destRect;
            srcRect = { 0, 0, 1, 1 };
            destRect = { 0, 0, v.w, v.h };

            if (image->texture && gameProps->renderer) {
                // 2D Rendering
                SDL_SetTextureScaleMode(image->texture, SDL_SCALEMODE_NEAREST);
                SDL_SetTextureColorMod(image->texture, tint.r, tint.g, tint.b);
                SDL_SetTextureAlphaMod(image->texture, alpha * passOn.alpha * 255);
                Apply_SDL_BlendMode(gameProps, image->texture, blendMode);

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
                    vertices, passOn.alpha * alpha, tint,
                    v, passOn.insideTextureContainer,
                    blendMode
                );
            }
            #endif
        }

        virtual void update(double deltaTime) override {    
            Amara::Transition::update(deltaTime);
            
            sm.start();
            if (sm.once()) bringToFront();
            
            if (sm.event()) {
                alpha += deltaTime / fadeIn;
                if (alpha >= 1) {
                    alpha = 1;
                    sm.nextEvent();
                }
            }

            if (sm.once()) doTransition();
            if (sm.once()) bringToFront();
            sm.wait(0.1);

            if (sm.event()) {
                alpha -= deltaTime / fadeOut;
                if (alpha <= 0) {
                    alpha = 0;
                    sm.nextEvent();
                }
            }

            if (sm.once()) complete();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<FillTransition>("FillTransition",
                sol::base_classes, sol::bases<Amara::Transition, Amara::Node>(),
                "color", sol::property([](Amara::FillTransition& t) -> Amara::Color { return t.tint; }, [](Amara::FillTransition& t, sol::object c) { t.tint = c; }),
                "tint", sol::property([](Amara::FillTransition& t) -> Amara::Color { return t.tint; }, [](Amara::FillTransition& t, sol::object c) { t.tint = c; })
            );
        }
    };
}