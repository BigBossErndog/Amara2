namespace Amara {
    class FillCircle: public Sprite {
    public:
        using Amara::Sprite::tint;

        float radius = 1.0f;
        int baseSegments = 32;
        
        std::vector<SDL_Vertex> verts;
        std::vector<int> indices;

        FillCircle(): Amara::Sprite() {
            set_base_node_id("FillCircle");
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "color")) tint = json_get<Amara::Color>(config, "color");
            if (json_has(config, "radius")) setRadius(json_strict_extract<float>(config, "radius"));
            if (json_has(config, "diameter")) setDiameter(json_strict_extract<float>(config, "diameter"));
            if (json_has(config, "r")) setRadius(json_strict_extract<float>(config, "r"));
            if (json_has(config, "d")) setDiameter(json_strict_extract<float>(config, "d"));
            if (json_has(config, "segments")) baseSegments = json_strict_extract<int>(config, "segments");
            
            if (json_has(config, "width")) json_extract(config, "width");
            if (json_has(config, "height")) json_extract(config, "height");
            if (json_has(config, "w")) json_extract(config, "w");
            if (json_has(config, "h")) json_extract(config, "h");

            return Amara::Sprite::configure(config);
        }

        virtual void init() override {
            Amara::Sprite::init();

            if (!image) {
                image = gameProps->assets->whitePixel;
                textureWidth = image->width;
                textureHeight = image->height;

                setRadius(radius);
            }
        }

        virtual void drawChildren(const Rectangle& v) override {
            if (children.size() == 0) return;

            children_copy_list = children;

            Vector2 rec_scale = gameProps->passOn.scale;

            pass_on_properties();

            passOn.scale = rec_scale;
            gameProps->passOn.scale = passOn.scale;

            Amara::Node* child;
            for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = *it;
                if (child == nullptr || child->destroyed || !child->visible || child->parent != this) {
                    ++it;
                    continue;
                }

                if (passOn.insideCamera && child->is_camera) {
                    ++it;
                    continue;
                }

                update_properties();
                child->draw(v);

                gameProps->passOn = passOn;
                ++it;
            }
        }

        virtual void drawSelf(const Rectangle& v) override {
            if (image == nullptr) return;

            if (cropLeft < 0) cropLeft = 0;
            if (cropRight < 0) cropRight = 0;
            if (cropTop < 0) cropTop = 0;
            if (cropBottom < 0) cropBottom = 0;

            Vector2 vcenter = { v.w/2.0f, v.h/2.0f };
            Vector2 totalZoom = { passOn.zoom.x*passOn.window_zoom.x, passOn.zoom.y*passOn.window_zoom.y };

            float imgw = (spritesheet ? frameWidth : textureWidth);
            float imgh = (spritesheet ? frameHeight : textureHeight);

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
            if (spritesheet) {
                int maxFrames = (int)floor(((float)image->width / (float)spritesheet->frameWidth) * ((float)image->height / (float)spritesheet->frameHeight));
                int fixedFrame = ((frame - 1) % maxFrames);
                while (fixedFrame < 0) fixedFrame += maxFrames;
                srcRect.x = static_cast<float>((fixedFrame % (textureWidth / frameWidth)) * frameWidth + cropLeft);
                srcRect.y = static_cast<float>(floor(fixedFrame / (textureWidth / frameWidth)) * frameHeight + cropTop);
                srcRect.w = static_cast<float>(frameWidth - cropLeft - cropRight);
                srcRect.h = static_cast<float>(frameHeight - cropTop - cropBottom);
            }
            else {
                srcRect = {
                    static_cast<float>(cropLeft),
                    static_cast<float>(cropTop),
                    static_cast<float>(textureWidth - cropLeft - cropRight),
                    static_cast<float>(textureHeight - cropTop - cropBottom)
                };
            }

            float diameter = radius * 2.0f;
            float world_w = diameter * scale.x * passOn.scale.x;
            float world_h = diameter * scale.y * passOn.scale.y;

            Rectangle dim;
            dim.w = world_w;
            dim.h = world_h;
            dim.x = anchoredPos.x - diameter * origin.x * scale.x * passOn.scale.x;
            dim.y = anchoredPos.y - anchoredPos.z - diameter * origin.y * scale.y * passOn.scale.y;

            SDL_FRect destRect;
            destRect.x = vcenter.x + dim.x*totalZoom.x;
            destRect.y = vcenter.y + dim.y*totalZoom.y;
            destRect.w = dim.w * totalZoom.x;
            destRect.h = dim.h * totalZoom.y;

            SDL_FPoint dorigin = {
                (imgw*origin.x - cropLeft)*scale.x*passOn.scale.x*totalZoom.x,
                (imgh*origin.y - cropTop)*scale.y*passOn.scale.y*totalZoom.y
            };

            float diag_distance = distanceBetween(0, 0, destRect.w, destRect.h);
            if (!Shape::checkCollision(
                Rectangle(destRect), Rectangle(
                    -diag_distance, -diag_distance,
                    v.w + diag_distance*2, v.h + diag_distance*2
                )
            )) return;
            
            if (input.active && !passOn.inputDisabled) {
                Quad inputQuad = rotateQuad(
                    Quad(destRect),
                    Vector2(
                        destRect.x + dorigin.x,
                        destRect.y + dorigin.y
                    ),
                    passOn.rotation + rotation
                );
                Shape inputShape = inputQuad.getIncircle();
                
                input.queueInput(inputShape.move(Vector2(v.x, v.y)), v, nullptr);
            }

            if (gameProps->renderer) {
                int adaptive = (int)std::ceil(std::max(destRect.w * 0.5f, destRect.h * 0.5f) / 6.0f);
                int segments = std::min(128, std::max(12, baseSegments + adaptive));

                float centerX = destRect.x + destRect.w * 0.5f;
                float centerY = destRect.y + destRect.h * 0.5f;
                float pixelRadiusX = destRect.w * 0.5f;
                float pixelRadiusY = destRect.h * 0.5f;

                float rot = passOn.rotation + rotation;
                float cosr = cosf(rot);
                float sinr = sinf(rot);

                verts.resize((size_t)segments + 2);
                indices.resize((size_t)segments * 3);

                Uint8 finalA = (Uint8)std::round((float)tint.a * passOn.alpha * alpha);

                SDL_Vertex centerVert;
                centerVert.position.x = centerX;
                centerVert.position.y = centerY;
                centerVert.color = { (float)tint.r/255.0f, (float)tint.g/255.0f, (float)tint.b/255.0f, (float)finalA/255.0f };
                verts[0] = centerVert;

                for (int i = 0; i <= segments; ++i) {
                    float t = (float)i / (float)segments;
                    float ang = t * 2.0f * M_PI;
                    float cx = cosf(ang);
                    float sy = sinf(ang);

                    float lx = cx * pixelRadiusX;
                    float ly = sy * pixelRadiusY;

                    float rx = lx * cosr - ly * sinr;
                    float ry = lx * sinr + ly * cosr;

                    SDL_Vertex vtx;
                    vtx.position.x = centerX + rx;
                    vtx.position.y = centerY + ry;
                    vtx.color = { (float)tint.r/255.0f, (float)tint.g/255.0f, (float)tint.b/255.0f, (float)finalA/255.0f };
                    verts[i + 1] = vtx;
                }

                // build indices for triangles (0, i, i+1)
                int index = 0;
                for (int i = 1; i <= segments; ++i) {
                    indices[index] = 0;
                    indices[index + 1] = i;
                    indices[index + 2] = i + 1;
                    index += 3;
                }

                SDL_SetRenderDrawBlendMode(gameProps->renderer, SDL_BLENDMODE_BLEND);

                // Render geometry
                SDL_RenderGeometry(
                    gameProps->renderer,
                    NULL,
                    verts.data(), (int)verts.size(),
                    indices.data(), (int)indices.size()
                );

                return;
            }
            #ifdef AMARA_OPENGL
            if (image->glTextureID != 0 && gameProps->glContext != NULL && gameProps->renderBatch) {
                float centerX = destRect.x + destRect.w * 0.5f;
                float centerY = destRect.y + destRect.h * 0.5f;
                float pixelRadiusX = destRect.w * 0.5f;
                float pixelRadiusY = destRect.h * 0.5f;

                float uvCenterX = (srcRect.x + srcRect.w * 0.5f) / (float)textureWidth;
                float uvCenterY = (srcRect.y + srcRect.h * 0.5f) / (float)textureHeight;
                float uvRadiusX = (srcRect.w * 0.5f) / (float)textureWidth;
                float uvRadiusY = (srcRect.h * 0.5f) / (float)textureHeight;

                int adaptive = (int)std::ceil(std::max(pixelRadiusX, pixelRadiusY) / 6.0f);
                int segments = std::min(128, std::max(12, baseSegments + adaptive));
                if (segments < 3) segments = 3;

                float rot = passOn.rotation + rotation;
                float cosr = cosf(rot);
                float sinr = sinf(rot);

                for (int i = 0; i < segments; ++i) {
                    float ta = (float)i / (float)segments;
                    float tb = (float)(i + 1) / (float)segments;
                    float anga = ta * 2.0f * M_PI;
                    float angb = tb * 2.0f * M_PI;

                    float cax = cosf(anga);
                    float cay = sinf(anga);
                    float cbx = cosf(angb);
                    float cby = sinf(angb);

                    float ax = cax * pixelRadiusX;
                    float ay = cay * pixelRadiusY;
                    float bx = cbx * pixelRadiusX;
                    float by = cby * pixelRadiusY;

                    float arx = ax * cosr - ay * sinr;
                    float ary = ax * sinr + ay * cosr;
                    float brx = bx * cosr - by * sinr;
                    float bry = bx * sinr + by * cosr;

                    float finalAx = centerX + arx;
                    float finalAy = centerY + ary;
                    float finalBx = centerX + brx;
                    float finalBy = centerY + bry;

                    float uA = uvCenterX + cax * uvRadiusX;
                    float vA = uvCenterY + cay * uvRadiusY;
                    float uB = uvCenterX + cbx * uvRadiusX;
                    float vB = uvCenterY + cby * uvRadiusY;
                    float uC = uvCenterX;
                    float vC = uvCenterY;

					float clipCx, clipCy, clipAx, clipAy, clipBx, clipBy;

					if (!passOn.insideTextureContainer) {
						clipCx = -1.0f + (centerX / v.w) * 2.0f;
						clipCy = -1.0f + (1.0f - centerY / v.h) * 2.0f;
						clipAx = -1.0f + (finalAx / v.w) * 2.0f;
						clipAy = -1.0f + (1.0f - finalAy / v.h) * 2.0f;
						clipBx = -1.0f + (finalBx / v.w) * 2.0f;
						clipBy = -1.0f + (1.0f - finalBy / v.h) * 2.0f;
					}
					else {
						clipCx = -1.0f + (centerX / v.w) * 2.0f;
						clipCy = -1.0f + (centerY / v.h) * 2.0f;
						clipAx = -1.0f + (finalAx / v.w) * 2.0f;
						clipAy = -1.0f + (finalAy / v.h) * 2.0f;
						clipBx = -1.0f + (finalBx / v.w) * 2.0f;
						clipBy = -1.0f + (finalBy / v.h) * 2.0f;
					}

                    std::array<float, 16> verts;
                    
                    verts[0]  = clipCx; verts[1]  = clipCy; verts[2]  = uC; verts[3]  = vC;
                    verts[4]  = clipAx; verts[5]  = clipAy; verts[6]  = uA; verts[7]  = vA;
                    verts[8]  = clipBx; verts[9]  = clipBy; verts[10] = uB; verts[11] = vB;
                    verts[12] = clipBx; verts[13] = clipBy; verts[14] = uB; verts[15] = vB;

                    gameProps->renderBatch->pushQuad(
                        gameProps->currentShaderProgram,
                        image->glTextureID,
                        verts,
                        passOn.alpha * alpha,
                        tint,
                        v,
                        passOn.insideTextureContainer,
                        blendMode
                    );
                }
            }
            #endif
        }

        float setRadius(float r) {
            radius = r;
            return radius;
        }
        
        virtual Shape getCollisionShape() override {
            return Circle(pos.x, pos.y, radius);
        }

        float setDiameter(float d) {
            return setRadius(d / 2.0f) * 2.0f;
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::FillCircle>("FillCircle",
                sol::base_classes, sol::bases<Amara::Sprite, Amara::Node>(),
                "color", sol::property([](Amara::FillCircle& t) -> Amara::Color { return t.tint; }, [](Amara::FillCircle& t, sol::object c) { t.tint = c; }),
                "r", sol::property([](Amara::FillCircle& t) -> float { return t.radius; }, &Amara::FillCircle::setRadius),
                "radius", sol::property([](Amara::FillCircle& t) -> float { return t.radius; }, &Amara::FillCircle::setRadius),
                "d", sol::property([](Amara::FillCircle& t) -> float { return t.radius * 2.0f; }, &Amara::FillCircle::setDiameter),
                "diameter", sol::property([](Amara::FillCircle& t) -> float { return t.radius * 2.0f; }, &Amara::FillCircle::setDiameter),
                "segments", sol::property([](Amara::FillCircle& t) -> float { return t.baseSegments; }, [](Amara::FillCircle& t, sol::object v) {
                    if (v.get_type() == sol::type::number) {
                        t.baseSegments = floor(v.as<double>());
                    }
                }),
                "texture", sol::property([](Amara::FillCircle& t) -> std::string { return t.image ? t.image->key : ""; }, &Amara::FillCircle::setTexture)
            );
        }
    };
}