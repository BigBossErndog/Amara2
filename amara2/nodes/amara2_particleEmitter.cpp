namespace Amara {
    class Particle {
    public:
        Particle() = default;

        static float constexpr invalid = std::numeric_limits<float>::lowest();
        
        sol::object luaobject;
        Particle* keep = nullptr;

        Amara::Vector2 pos;
        Amara::Vector2 scale = 1.0f;
        Amara::Vector2 origin = 0.5f;
        float rotation = 0;
        float alpha = 1;

        int frame = 0;

        Amara::Color tint;
        Amara::Color endTint;
        Amara::Color drawTint;

        Amara::Vector2 velocity;
        Amara::Vector2 acceleration;

        float angularVelocity = 0;

        float lifeTime = 0;
        float maxLifeTime = 0;
        
        float startX = invalid;
        float startY = invalid;
        float endX = invalid;
        float endY = invalid;

        float startAlpha = invalid;
        float endAlpha = invalid;

        Vector2 endScale = Vector2(1);
        bool end_scale_set = false;

        bool in_use = false;

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Particle>("Particle",
                "pos", sol::property([](Amara::Particle& t) -> Vector2& { return t.pos; }, [](Amara::Particle& t, sol::object v) { t.pos = v; }),
                "x", sol::property([](Amara::Particle& t) -> float { return t.pos.x; }, [](Amara::Particle& t, float v) { t.pos.x = v; }),
                "y", sol::property([](Amara::Particle& t) -> float { return t.pos.y; }, [](Amara::Particle& t, float v) { t.pos.y = v; }),
                "origin", sol::property([](Amara::Particle& t) -> Vector2& { return t.origin; }, [](Amara::Particle& t, sol::object v) { t.origin = v; }),
                "originX", sol::property([](Amara::Particle& t) -> float { return t.origin.x; }, [](Amara::Particle& t, float v) { t.origin.x = v; }),
                "originY", sol::property([](Amara::Particle& t) -> float { return t.origin.y; }, [](Amara::Particle& t, float v) { t.origin.y = v; }),
                "rotation", sol::property([](Amara::Particle& t) -> float { return t.rotation; }, [](Amara::Particle& t, float v) { t.rotation = v; }),
                "scale", sol::property([](Amara::Particle& t) -> Vector2& { return t.scale; }, [](Amara::Particle& t, sol::object v) { t.scale = v; }),
                "scaleX", sol::property([](Amara::Particle& t) -> float { return t.scale.x; }, [](Amara::Particle& t, float v) { t.scale.x = v; }),
                "scaleY", sol::property([](Amara::Particle& t) -> float { return t.scale.y; }, [](Amara::Particle& t, float v) { t.scale.y = v; }),
                "alpha", sol::property([](Amara::Particle& t) -> float { return t.alpha; }, [](Amara::Particle& t, float v) { t.alpha = v; }),
                "velocity", sol::property([](Amara::Particle& t) -> Vector2& { return t.velocity; }, [](Amara::Particle& t, sol::object v) { t.velocity = v; }),
                "acceleration", sol::property([](Amara::Particle& t) -> Vector2& { return t.acceleration; }, [](Amara::Particle& t, sol::object v) { t.acceleration = v; }),
                "lifeTime", sol::readonly(&Amara::Particle::lifeTime)
            );
        }
    };

    class ParticleEmitter: public Amara::Sprite {
    public:
        std::vector<Particle> particles;

        nlohmann::json particle_config = nlohmann::json::object();

        static float constexpr invalid = std::numeric_limits<float>::lowest();

        Ease easing = Ease::Linear;

        int poolSize = 128;
        double spawnRate = 0;
        double spawn_timer = 0;

        bool spawning = false;
        int spawnedCount = 0;
        int end_particle = -1;

        bool updated_this_frame = false;

        ParticleEmitter(): Amara::Sprite() {
            set_base_node_id("ParticleEmitter");
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "spawning")) {
                spawning = config["spawning"];
            }
            return Amara::Sprite::configure(config);
        }

        virtual sol::object luaConfigure(std::string key, sol::object val) override {
            if (String::equal(key, "particles")) {
                particle_config.update(lua_to_json(val));
                
                if (json_has(particle_config, "ease")) {
                    easing = particle_config["ease"];
                }
                if (json_has(particle_config, "poolSize")) {
                    int _poolSize = particle_config["poolSize"];
                    if (_poolSize != poolSize) {
                        if (particles.size() > _poolSize) {
                            for (int i = particles.size() - 1; i >= _poolSize; i--) {
                                Amara::Particle& particle = particles[i];
                                if (particle.in_use) {
                                    particle.in_use = false;
                                    spawnedCount -= 1;
                                }
                            }
                        }
                        poolSize = _poolSize;
                        resizeParticles();
                    }
                }
                if (json_has(particle_config, "spawnRate")) {
                    spawnRate = particle_config["spawnRate"];
                }
                if (json_has(particle_config, "spawning")) {
                    spawning = particle_config["spawning"];
                }
                if (json_has(particle_config, "texture")) {
                    setTexture(particle_config["texture"]);
                }
                if (json_has(particle_config, "depth")) {
                    depth = particle_config["depth"];
                }

                if (val.is<sol::table>()) {
                    sol::table tbl = val.as<sol::table>();
                    if (tbl["onParticleUpdate"].valid()) {
                        sol::object func = tbl["onParticleUpdate"];
                        funcs.setFunction(nodeID, "onParticleUpdate", func.as<sol::function>());
                    }
                    if (tbl["onParticleSpawn"].valid()) {
                        sol::object func = tbl["onParticleSpawn"];
                        funcs.setFunction(nodeID, "onParticleSpawn", func.as<sol::function>());
                    }
                }
            }
            return Amara::Sprite::luaConfigure(key, val);
        }

        virtual void update(double deltaTime) override {
            Amara::Sprite::update(deltaTime);
            updated_this_frame = false;
        }

        Vector2 parseVector(const nlohmann::json& config) {
            if (config.is_array() && config.size() == 2) {
                Vector2 min = config[0];
                Vector2 max = config[1];
                return Vector2(
                    lua_random(gameProps->lua, min.x, max.x),
                    lua_random(gameProps->lua, min.y, max.y)
                );
            }
            else if (config.is_object() && json_has(config, "min", "max")) {
                Vector2 min = config["min"];
                Vector2 max = config["max"];
                return Vector2(
                    lua_random(gameProps->lua, min.x, max.x),
                    lua_random(gameProps->lua, min.y, max.y)
                );
            }
            else {
                return Vector2(config);
            }
        }

        double parseValue(const nlohmann::json& config) {
            if (config.is_number()) {
                return config;
            }
            else if (config.is_object() && json_has(config, "min", "max")) {
                double min = config["min"];
                double max = config["max"];
                return lua_random(gameProps->lua, min, max);
            }
            else if (config.is_array() && config.size() == 2) {
                double min = config[0];
                double max = config[1];
                return lua_random(gameProps->lua, min, max);
            }
            fatal_error("Error: Particle config expected number.");
        }
        int parseInteger(const nlohmann::json& config) {
            if (config.is_number()) {
                return config;
            }
            else if (config.is_object() && json_has(config, "min", "max")) {
                int min = config["min"];
                int max = config["max"];
                max += 1;
                return floor(lua_random(gameProps->lua, min, max));
            }
            else if (config.is_array() && config.size() == 2) {
                int min = config[0];
                int max = config[1];
                max += 1;
                return floor(lua_random(gameProps->lua, min, max));
            }
            fatal_error("Error: Particle config expected number.");
        }

        void initParticle(Particle& particle, const nlohmann::json& particle_config) {
            particle.lifeTime = 0;

            particle.pos = Vector2(0);
            if (json_has(particle_config, "pos")) {
                particle.pos = parseVector(particle_config["pos"]);
            }
            if (json_has(particle_config, "x")) {
                particle.pos.x = parseValue(particle_config["x"]);
            }
            if (json_has(particle_config, "y")) {
                particle.pos.y = parseValue(particle_config["y"]);
            }

            particle.endX = invalid;
            particle.endY = invalid;
            if (json_has(particle_config, "endPos")) {
                Vector2 endPos = parseVector(particle_config["endPos"]);
                particle.endX = endPos.x;
                particle.endY = endPos.y;
            }
            if (json_has(particle_config, "endX")) {
                particle.endX = parseValue(particle_config["endX"]);
            }
            if (json_has(particle_config, "endY")) {
                particle.endY = parseValue(particle_config["endY"]);
            }

            particle.frame = frame;
            if (json_has(particle_config, "frame")) {
                particle.frame = parseInteger(particle_config["frame"]);
            }

            particle.origin = Vector2(0.5);
            if (json_has(particle_config, "origin")) {
                particle.origin = parseVector(particle_config["origin"]);
            }
            if (json_has(particle_config, "originX")) {
                particle.origin.x = parseValue(particle_config["originX"]);
            }
            if (json_has(particle_config, "originY")) {
                particle.origin.y = parseValue(particle_config["originY"]);
            }

            particle.scale = Vector2(1);
            if (json_has(particle_config, "scale")) {
                particle.scale = parseVector(particle_config["scale"]);
            }
            if (json_has(particle_config, "scaleX")) {
                particle.scale.x = parseValue(particle_config["scaleX"]);
            }
            if (json_has(particle_config, "scaleY")) {
                particle.scale.y = parseValue(particle_config["scaleY"]);
            }

            particle.endScale = Vector2(1);
            particle.end_scale_set = false;
            if (json_has(particle_config, "endScale")) {
                particle.endScale = parseVector(particle_config["endScale"]);
                particle.end_scale_set = true;
            }
            if (json_has(particle_config, "endScaleX")) {
                particle.endScale.x = parseValue(particle_config["endScaleX"]);
            }
            if (json_has(particle_config, "endScaleY")) {
                particle.endScale.y = parseValue(particle_config["endScaleY"]);
            }

            particle.velocity = Vector2(0);
            if (json_has(particle_config, "velocity")) {
                particle.velocity = parseVector(particle_config["velocity"]);
            }
            if (json_has(particle_config, "velocityX")) {
                particle.velocity.x = parseValue(particle_config["velocityX"]);
            }
            if (json_has(particle_config, "velocityY")) {
                particle.velocity.y = parseValue(particle_config["velocityY"]);
            }

            particle.acceleration = Vector2(0);
            if (json_has(particle_config, "acceleration")) {
                particle.acceleration = parseVector(particle_config["acceleration"]);
            }
            if (json_has(particle_config, "accelerationX")) {
                particle.acceleration.x = parseValue(particle_config["accelerationX"]);
            }
            if (json_has(particle_config, "accelerationY")) {
                particle.acceleration.y = parseValue(particle_config["accelerationY"]);
            }

            particle.maxLifeTime = 1;
            if (json_has(particle_config, "lifeTime")) {
                particle.maxLifeTime = parseValue(particle_config["lifeTime"]);
            }

            particle.rotation = 0;
            if (json_has(particle_config, "rotation")) {
                particle.rotation = parseValue(particle_config["rotation"]);
            }

            particle.angularVelocity = 0;
            if (json_has(particle_config, "angularVelocity")) {
                particle.angularVelocity = parseValue(particle_config["angularVelocity"]);
            }
            
            particle.alpha = 1;
            if (json_has(particle_config, "alpha")) {
                particle.alpha = parseValue(particle_config["alpha"]);
            }

            particle.endAlpha = invalid;
            if (json_has(particle_config, "endAlpha")) {
                particle.endAlpha = parseValue(particle_config["endAlpha"]);
            }
            
            particle.tint = Color::White;
            if (json_has(particle_config, "tint")) {
                nlohmann::json tint_config = particle_config["tint"];
                if (tint_config.is_object()) {
                    if (json_has(tint_config, "min", "max")) {
                        Color min = tint_config["min"];
                        Color max = tint_config["max"];
                        particle.tint = Color(
                            lua_random(gameProps->lua, min.r, max.r),
                            lua_random(gameProps->lua, min.g, max.g),
                            lua_random(gameProps->lua, min.b, max.b),
                            lua_random(gameProps->lua, min.a, max.a)
                        );
                    }
                    else {
                        particle.tint = tint_config;
                    }
                }
                else if (tint_config.is_string()) {
                    particle.tint = Color(tint_config);
                }
                else if (tint_config.is_array()) {
                    if (tint_config.size() == 4) {
                        particle.tint = Color(tint_config[0], tint_config[1], tint_config[2], tint_config[3]);
                    }
                    else if (tint_config.size() == 2) {
                        Color min = tint_config[0];
                        Color max = tint_config[1];
                        particle.tint = Color(
                            lua_random(gameProps->lua, min.r, max.r),
                            lua_random(gameProps->lua, min.g, max.g),
                            lua_random(gameProps->lua, min.b, max.b),
                            lua_random(gameProps->lua, min.a, max.a)
                        );
                    }
                }
            }
            particle.drawTint = particle.tint;

            particle.endTint = particle.tint;
            if (json_has(particle_config, "endTint")) {
                nlohmann::json tint_config = particle_config["endTint"];
                if (tint_config.is_object()) {
                    if (json_has(tint_config, "min", "max")) {
                        Color min = tint_config["min"];
                        Color max = tint_config["max"];
                        particle.endTint = Color(
                            lua_random(gameProps->lua, min.r, max.r),
                            lua_random(gameProps->lua, min.g, max.g),
                            lua_random(gameProps->lua, min.b, max.b),
                            lua_random(gameProps->lua, min.a, max.a)
                        );
                    }
                    else {
                        particle.endTint = tint_config;
                    }
                }
                else if (tint_config.is_string()) {
                    particle.endTint = Color(tint_config);
                }
                else if (tint_config.is_array()) {
                    if (tint_config.size() == 4) {
                        particle.endTint = Color(tint_config[0], tint_config[1], tint_config[2], tint_config[3]);
                    }
                    else if (tint_config.size() == 2) {
                        Color min = tint_config[0];
                        Color max = tint_config[1];
                        particle.endTint = Color(
                            lua_random(gameProps->lua, min.r, max.r),
                            lua_random(gameProps->lua, min.g, max.g),
                            lua_random(gameProps->lua, min.b, max.b),
                            lua_random(gameProps->lua, min.a, max.a)
                        );
                    }
                }
            }

            particle.startX = particle.pos.x;
            particle.startY = particle.pos.y;

            particle.startAlpha = particle.alpha;
        }

        virtual void create() override {
            Amara::Sprite::create();
            resizeParticles();
        }
        
        void resizeParticles() {
            particles.resize(poolSize);
            for (auto& particle : particles) {
                particle.in_use = false;
                if (!particle.luaobject.valid() || particle.keep != &particle) {
                    particle.luaobject = sol::make_object(gameProps->lua, &particle);
                    particle.keep = &particle;
                }
            }
        }

        void burst(double amount, sol::object lua_config) {
            sol::function onSpawn = funcs.getFunction("onParticleSpawn");

            nlohmann::json new_config = particle_config;
            if (lua_config.is<sol::table>()) {
                new_config.update(lua_to_json(lua_config));
            }

            amount = floor(amount);
            for (int i = 0; i < amount; i++) {
                Amara::Particle& particle = particles[i];
                if (!particle.in_use) {
                    particle.in_use = true;
                    initParticle(particle, new_config);
                    spawnedCount += 1;
                    if (onSpawn.valid()) {
                        try {
                            if (!particle.luaobject.valid() || particle.keep != &particle) {
                                particle.luaobject = sol::make_object(gameProps->lua, &particle);
                                particle.keep = &particle;
                            }
                            onSpawn(particle.luaobject);
                        }
                        catch (const sol::error& e) {
                            fatal_error(e.what());
                            gameProps->breakWorld();
                        }
                        catch (const std::exception& e) {
                            fatal_error(e.what());
                            gameProps->breakWorld();
                        }
                    }
                    amount -=1;
                    if (i > end_particle) end_particle = i;

                    if (amount <= 0) {
                        break;
                    }   
                }
            }
        }

        void drawParticle(const Rectangle& v, const Particle& particle) {
            if (image == nullptr) return;

            if (cropLeft < 0) cropLeft = 0;
            if (cropRight < 0) cropRight = 0;
            if (cropTop < 0) cropTop = 0;
            if (cropBottom < 0) cropBottom = 0;

            Vector2 vcenter = { v.w/2.0f, v.h/2.0f };
            Vector2 totalZoom = { passOn.zoom.x*passOn.window_zoom.x, passOn.zoom.y*passOn.window_zoom.y };

            float imgw = (spritesheet ? frameWidth : textureWidth);
            float imgh = (spritesheet ? frameHeight : textureHeight);

            Vector2 render_pos = pos + particle.pos;
            if (renderPixelPerfect) render_pos = render_pos.round();

            Vector3 anchoredPos = Vector3(
                rotateAroundAnchor(
                    passOn.anchor, 
                    Vector2( 
                        (passOn.anchor.x + render_pos.x*passOn.scale.x*scale.x),
                        (passOn.anchor.y + render_pos.y*passOn.scale.y*scale.y)
                    ),
                    passOn.rotation
                ),
                passOn.anchor.z + pos.z
            );

            SDL_FRect srcRect, destRect;

            if (spritesheet) {
                int maxFrames = (int)floor(((float)image->width / (float)spritesheet->frameWidth) * ((float)image->height / (float)spritesheet->frameHeight));
                int fixedFrame = ((particle.frame - 1) % maxFrames);
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

            Rectangle dim = {
                anchoredPos.x + (cropLeft - imgw*particle.origin.x)*scale.x*passOn.scale.x, 
                anchoredPos.y - anchoredPos.z + (cropTop - imgh*particle.origin.y)*scale.y*passOn.scale.y,
                (imgw - cropLeft - cropRight)*scale.x*passOn.scale.x*particle.scale.x,
                (imgh - cropTop - cropBottom)*scale.y*passOn.scale.y*particle.scale.y
            };

            destRect.x = vcenter.x + dim.x*totalZoom.x;
            destRect.y = vcenter.y + dim.y*totalZoom.y;
            destRect.w = dim.w * totalZoom.x;
            destRect.h = dim.h * totalZoom.y;

            SDL_FPoint dorigin = {
                (imgw*particle.origin.x - cropLeft)*scale.x*passOn.scale.x*totalZoom.x,
                (imgh*particle.origin.y - cropTop)*scale.y*passOn.scale.y*totalZoom.y
            };

            float diag_distance = distanceBetween(0, 0, destRect.w, destRect.h);
            if (!Shape::checkCollision(
                Rectangle(destRect), Rectangle(
                    -diag_distance, -diag_distance,
                    v.w + diag_distance*2, v.h + diag_distance*2
                )
            )) return;

            if (input.active && !passOn.insideTextureContainer) {
                Quad inputQuad = rotateQuad(
                    Quad(destRect),
                    Vector2(
                        destRect.x + dorigin.x,
                        destRect.y + dorigin.y
                    ),
                    passOn.rotation + rotation + particle.rotation
                );
                input.queueInput(moveQuad(inputQuad, v.x, v.y), v, nullptr);
            }

            Amara::Color particleTint = particle.drawTint * tint;

            if (image->texture && gameProps->renderer) {
                // 2D Rendering
                SDL_SetTextureScaleMode(image->texture, SDL_SCALEMODE_NEAREST);
                SDL_SetTextureColorMod(image->texture, particleTint.r, particleTint.g, particleTint.b);
                SDL_SetTextureAlphaMod(image->texture, particle.alpha * alpha * passOn.alpha * 255);
                Apply_SDL_BlendMode(gameProps, image->texture, blendMode);

                SDL_RenderTextureRotated(
                    gameProps->renderer, 
                    image->texture,
                    &srcRect,
                    &destRect,
                    getDegrees(passOn.rotation + rotation + particle.rotation),
                    &dorigin,
                    SDL_FLIP_NONE
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
                    Vector2(
                        destRect.x + dorigin.x,
                        destRect.y + dorigin.y
                    ),
                    passOn.rotation + rotation + particle.rotation
                ), passOn.insideTextureContainer);

                vertices = {
                    destQuad.p1.x, destQuad.p1.y, srcQuad.p1.x, srcQuad.p1.y,
                    destQuad.p2.x, destQuad.p2.y, srcQuad.p2.x, srcQuad.p2.y,
                    destQuad.p3.x, destQuad.p3.y, srcQuad.p3.x, srcQuad.p3.y,
                    destQuad.p4.x, destQuad.p4.y, srcQuad.p4.x, srcQuad.p4.y
                };

                gameProps->renderBatch->pushQuad(
                    gameProps->currentShaderProgram,
                    image->glTextureID,
                    vertices, passOn.alpha * alpha * particle.alpha, particleTint,
                    v, passOn.insideTextureContainer,
                    blendMode
                );
            }
            #endif
        }

        virtual void drawSelf(const Rectangle& v) override {
            if (image == nullptr) return;

            double deltaTime = gameProps->deltaTime;

            int spawn_count = 0;
            if (!updated_this_frame) {
                if (spawning && spawnRate > 0) {
                    spawn_timer -= deltaTime;
                    while (spawn_timer <= 0) {
                        spawn_timer += 1/spawnRate;
                        spawn_count += 1;
                    }
                }
                else {
                    spawn_timer = 0;
                }
            }

            sol::function onSpawn = funcs.getFunction("onParticleSpawn");
            bool onUpdate_defined = funcs.hasFunction("onParticleUpdate");
            
            int last_particle = -1;
            bool spawn_new = false;
            for (int i = 0; i < particles.size(); i++) {
                if (i > end_particle && spawn_count == 0) break;
                
                Particle& particle = particles[i];
                
                spawn_new = false;

                if (!particle.in_use) {
                    if (spawn_count == 0) continue;
                    else spawn_new = true;
                }
                else {
                    if (particle.lifeTime >= particle.maxLifeTime && !updated_this_frame) {
                        spawnedCount -= 1;
                        if (spawn_count > 0) {
                            spawn_new = true;
                        }
                        else {
                            particle.in_use = false;
                            continue;
                        }
                    }
                }

                if (spawn_new) {
                    spawnedCount += 1;
                    spawn_count -= 1;
                    particle.in_use = true;
                    initParticle(particle, particle_config);
                    if (onSpawn.valid()) {
                        try {
                            if (!particle.luaobject.valid() || particle.keep != &particle) {
                                particle.luaobject = sol::make_object(gameProps->lua, &particle);
                                particle.keep = &particle;
                            }
                            onSpawn(particle.luaobject);
                        }
                        catch (const sol::error& e) {
                            fatal_error(e.what());
                            gameProps->breakWorld();
                        }
                        catch (const std::exception& e) {
                            fatal_error(e.what());
                            gameProps->breakWorld();
                        }
                    }
                }

                if (!updated_this_frame) {
                    double prog = particle.lifeTime / particle.maxLifeTime;
                    particle.pos += particle.velocity * deltaTime + particle.acceleration * deltaTime * deltaTime * 0.5f;
                    particle.velocity += particle.acceleration * deltaTime;
                    
                    if (particle.startX != invalid && particle.endX != invalid) {
                        particle.pos.x = ease(particle.startX, particle.endX, prog, easing); 
                    }
                    if (particle.startY != invalid && particle.endY != invalid) {
                        particle.pos.y = ease(particle.startY, particle.endY, prog, easing); 
                    }
                    
                    if (particle.startAlpha != invalid && particle.endAlpha != invalid) {
                        particle.alpha = ease(particle.startAlpha, particle.endAlpha, prog, easing); 
                    }

                    if (particle.end_scale_set) {
                        particle.scale = Vector2(
                            ease(particle.scale.x, particle.endScale.x, prog, easing),
                            ease(particle.scale.y, particle.endScale.y, prog, easing)
                        );
                    }

                    particle.drawTint = ease(particle.tint, particle.endTint, prog, easing);

                    particle.rotation += particle.angularVelocity * deltaTime;

                    if (onUpdate_defined) {
                        sol::function func = funcs.getFunction("onParticleUpdate");
                        try {
                            if (!particle.luaobject.valid() || particle.keep != &particle) {
                                particle.luaobject = sol::make_object(gameProps->lua, &particle);
                                particle.keep = &particle;
                            }
                            func(particle.luaobject, deltaTime);
                        }
                        catch (const sol::error& e) {
                            fatal_error(e.what());
                            gameProps->breakWorld();
                        }
                        catch (const std::exception& e) {
                            fatal_error(e.what());
                            gameProps->breakWorld();
                        }
                    }
                }

                last_particle = i;

                drawParticle(v, particle);

                if (!updated_this_frame) {
                    particle.lifeTime += deltaTime;
                }
            }
            end_particle = last_particle;

            updated_this_frame = true;
        }

        static void bind_lua(sol::state& lua) {
            Amara::Particle::bind_lua(lua);

            lua.new_usertype<ParticleEmitter>("ParticleEmitter",
                sol::base_classes, sol::bases<Amara::Sprite, Amara::Node>(),
                "poolSize", sol::readonly(&Amara::ParticleEmitter::poolSize),
                "spawnRate", &Amara::ParticleEmitter::spawnRate,
                "spawning", &Amara::ParticleEmitter::spawning,
                "burst", &Amara::ParticleEmitter::burst,
                "spawnedCount", sol::readonly(&Amara::ParticleEmitter::spawnedCount)
            );
        }
    };
}