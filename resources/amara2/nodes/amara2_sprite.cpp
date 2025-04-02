namespace Amara {
    class Animation;
    /**
     * This is used for 2D sprites.
     */
    class Sprite: public Amara::Node {
    public:
        ImageAsset* image = nullptr;
        SpritesheetAsset* spritesheet = nullptr;

        Vector2 origin = { 0.5, 0.5 };

        int imageWidth = 0;
        int imageHeight = 0;

        int frameWidth = 0;
        int frameHeight = 0;

        int cropLeft = 0;
        int cropRight = 0;
        int cropTop = 0;
        int cropBottom = 0;

        int frame = 0;

        Animation* animation = nullptr;

        #ifdef AMARA_OPENGL
            unsigned int VAO, VBO, EBO;
            unsigned int texture;

            std::array<float, 16> vertices = {
                -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
                 0.5f, -0.5f,  1.0f, 0.0f, // Bottom-right
                 0.5f,  0.5f,  1.0f, 1.0f, // Top-right
                -0.5f,  0.5f,  0.0f, 1.0f  // Top-left
            };

            static constexpr std::array<unsigned int, 6> indices = {
                0, 1, 2,  // First Triangle
                0, 2, 3   // Second Triangle
            };
        #endif

        Sprite(): Amara::Node() {
            set_base_node_id("Sprite");
        }

        void init() {
            Amara::Node::init();

            #ifdef AMARA_OPENGL
            // Generate VAO, VBO, and EBO
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            // Bind VAO
            glBindVertexArray(VAO);
            
            // Bind & Fill VBO
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
            
            // Bind & Fill EBO
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
            
            // Position attribute
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // Texture coordinate attribute
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glBindVertexArray(0);
            #endif
        }

        bool setTexture(std::string key) {
            image = nullptr;
            spritesheet = nullptr;

            if (!Props::assets->has(key)) {
                debug_log("Error: Asset \"", key, "\" was not found.");
                return false;
            }

            image = Props::assets->get(key)->as<ImageAsset*>();
            
            if (image == nullptr) {
                debug_log("Error: Asset \"", key, "\" is not a valid texture asset.");
                return false;
            }
            imageWidth = image->width;
            imageHeight = image->height;

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

        virtual nlohmann::json toJSON() override {
            nlohmann::json data = Amara::Node::toJSON();
            
            if (image) {
                data["texture"] = image->key;
            }
            data["frame"] = frame;
            
            data["originX"] = origin.x;
            data["originY"] = origin.y;

            data["cropLeft"] = cropLeft;
            data["cropRight"] = cropRight;
            data["cropTop"] = cropTop;
            data["cropBottom"] = cropBottom;

            return data;
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            Amara::Node::configure(config);

            if (json_has(config, "texture")) setTexture(config["texture"]);
            if (json_has(config, "frame")) frame = config["frame"];
            if (json_has(config, "animation")) animate(config["animation"]);

            if (json_has(config, "originX")) origin.x = config["originX"];
            if (json_has(config, "originY")) origin.y = config["originY"];

            if (json_has(config, "cropLeft")) cropLeft = config["cropLeft"];
            if (json_has(config, "cropRight")) cropRight = config["cropRight"];
            if (json_has(config, "cropTop")) cropTop = config["cropTop"];
            if (json_has(config, "cropBottom")) cropBottom = config["cropBottom"];

            return this;
        }

        Amara::Action* animate(nlohmann::json config);

        sol::object animate(sol::object config) {
            Amara::Action* anim = animate(lua_to_json(config));
            return anim->get_lua_object();
        }

        sol::object setOrigin(float _x, float _y) {
            origin = { _x, _y };
            return get_lua_object();
        }

        virtual void drawSelf(const Rectangle& v) override {
            if (image == nullptr) return;

            SDL_Rect setv = Rectangle::makeSDLRect(v);

            if (cropLeft < 0) cropLeft = 0;
            if (cropRight < 0) cropRight = 0;
            if (cropTop < 0) cropTop = 0;
            if (cropBottom < 0) cropBottom = 0;

            Vector2 vcenter = centerOf(v);

            float imgw = (spritesheet ? frameWidth : imageWidth);
            float imgh = (spritesheet ? frameHeight : imageHeight);

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

            if (spritesheet) {
                int fixedFrame = frame % (int)floor(((float)image->width / (float)spritesheet->frameWidth) * ((float)image->height / (float)spritesheet->frameHeight));
                srcRect.x = static_cast<float>((fixedFrame % (imageWidth / frameWidth)) * frameWidth + cropLeft);
                srcRect.y = static_cast<float>(floor(fixedFrame / (imageWidth / frameWidth)) * frameHeight + cropTop);
                srcRect.w = static_cast<float>(frameWidth - cropLeft - cropRight);
                srcRect.h = static_cast<float>(frameHeight - cropTop - cropBottom);
            }
            else {
                srcRect = {
                    static_cast<float>(cropLeft),
                    static_cast<float>(cropTop),
                    static_cast<float>(imageWidth - cropLeft - cropRight),
                    static_cast<float>(imageHeight - cropTop - cropBottom)
                };
            }

            Rectangle dim = {
                anchoredPos.x + (cropLeft - imgw*origin.x)*scale.x*passOn.scale.x, 
                anchoredPos.y - anchoredPos.z + (cropTop - imgh*origin.y)*scale.y*passOn.scale.y,
                (imgw - cropLeft - cropRight)*scale.x*passOn.scale.x,
                (imgh - cropTop - cropBottom)*scale.y*passOn.scale.y
            };

            destRect.x = vcenter.x + dim.x*passOn.zoom.x;
            destRect.y = vcenter.y + dim.y*passOn.zoom.y;
            destRect.w = dim.w * passOn.zoom.x;
            destRect.h = dim.h * passOn.zoom.y;

            SDL_FPoint dorigin = {
                (imgw*origin.x - cropLeft)*scale.x*passOn.scale.x*passOn.zoom.x,
                (imgh*origin.y - cropTop)*scale.y*passOn.scale.y*passOn.zoom.y
            };

            if (image->texture && Props::renderer) {
                // 2D Rendering
                SDL_SetRenderViewport(Props::renderer, &setv);
                SDL_SetTextureScaleMode(image->texture, SDL_SCALEMODE_NEAREST);

                SDL_RenderTextureRotated(
                    Props::renderer, 
                    image->texture,
                    &srcRect,
                    &destRect,
                    getDegrees(passOn.rotation + rotation),
                    &dorigin,
                    SDL_FLIP_NONE
                );
            }
            else if (image->gpuTexture && Props::gpuDevice) {
                // GPU Rendering
            }
            #ifdef AMARA_OPENGL
            else if (image->glTextureID != 0 && Props::glContext != NULL) {
                glViewport(v.x, Props::window_dim.h - v.y - v.h, v.w, v.h);
                Quad srcQuad = Quad(
                    { srcRect.x/imageWidth, srcRect.y/imageHeight },
                    { (srcRect.x+srcRect.w)/imageWidth, srcRect.y/imageHeight },
                    { (srcRect.x+srcRect.w)/imageWidth, (srcRect.y+srcRect.h)/imageHeight },
                    { srcRect.x/imageWidth, (srcRect.y+srcRect.h)/imageHeight }
                );
                Quad destQuad = glTranslateQuad(v, rotateQuad(
                    Quad(destRect),
                    Vector2(
                        destRect.x + dorigin.x,
                        destRect.y + dorigin.y
                    ), 
                    passOn.rotation + rotation
                ));

                vertices = {
                    destQuad.p1.x, destQuad.p1.y, srcQuad.p1.x, srcQuad.p1.y,
                    destQuad.p2.x, destQuad.p2.y, srcQuad.p2.x, srcQuad.p2.y,
                    destQuad.p3.x, destQuad.p3.y, srcQuad.p3.x, srcQuad.p3.y,
                    destQuad.p4.x, destQuad.p4.y, srcQuad.p4.x, srcQuad.p4.y
                };

                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
                
                GLint location = glGetUniformLocation(Props::currentShaderProgram->programID, "spriteTexture");
                if (location == -1) {
                    debug_log("Error: Uniform 'spriteTexture' not found in shader: \"", Props::currentShaderProgram->key, "\".");
                }
                glUniform1i(location, 0);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, image->glTextureID);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                // Draw the sprite
                glBindVertexArray(VAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
            #endif
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Sprite>("Sprite",
                sol::base_classes, sol::bases<Node>(),
                "setTexture", &Sprite::setTexture,
                "frame", &Sprite::frame,
                "animate",  sol::resolve<sol::object(sol::object)>(&Sprite::animate),
                "imagew", sol::readonly(&Sprite::imageWidth),
                "imageh", sol::readonly(&Sprite::imageHeight),
                "framew", sol::readonly(&Sprite::frameWidth),
                "frameh", sol::readonly(&Sprite::frameHeight),
                "cropLeft", &Sprite::cropLeft,
                "cropRight", &Sprite::cropRight,
                "cropTop", &Sprite::cropTop,
                "cropBottom", &Sprite::cropBottom,
                "setOrigin", &Sprite::setOrigin
            );
        }
    };
}