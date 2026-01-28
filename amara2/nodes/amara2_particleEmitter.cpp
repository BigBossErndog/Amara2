namespace Amara {
    class Particle {
    public:
        Particle() = default;

        static float constexpr invalid = std::numeric_limits<float>::lowest();
        
        sol::object luaobject;
        sol::table luatable;
        Particle* keep = nullptr;

        Amara::Vector2 pos;
        Amara::Vector2 scale = 1.0f;
        Amara::Vector2 origin = 0.5f;
        float rotation = 0;
        float alpha = 1;

        int frame = 0;

        Amara::Color tint = Color::White;

        Amara::Vector2 velocity;
        Amara::Vector2 acceleration;

        float rotationalVelocity = 0;

        float lifeTime = 0;

        Vector2 endScale = Vector2(1);
        bool end_scale_set = false;

        bool in_use = false;
        
        double progress = 0;
        
        bool waitingYoyo = false;
        
        double delay = 0;
        
        nlohmann::json start_data;
        nlohmann::json end_data;

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Particle>("Particle",
                "pos", sol::property([](Amara::Particle& t) -> Vector2& { return t.pos; }, [](Amara::Particle& t, sol::object v) { t.pos = v; }),
                "x", sol::property([](Amara::Particle& t) -> double { return t.pos.x; }, [](Amara::Particle& t, double v) { t.pos.x = v; }),
                "y", sol::property([](Amara::Particle& t) -> double { return t.pos.y; }, [](Amara::Particle& t, double v) { t.pos.y = v; }),
                "frame", sol::property([](Amara::Particle& t) -> int { return t.frame; }, [](Amara::Particle& t, int v) { t.frame = v; }),
                "origin", sol::property([](Amara::Particle& t) -> Vector2& { return t.origin; }, [](Amara::Particle& t, sol::object v) { t.origin = v; }),
                "originX", sol::property([](Amara::Particle& t) -> double { return t.origin.x; }, [](Amara::Particle& t, double v) { t.origin.x = v; }),
                "originY", sol::property([](Amara::Particle& t) -> double { return t.origin.y; }, [](Amara::Particle& t, double v) { t.origin.y = v; }),
                "rotation", sol::property([](Amara::Particle& t) -> double { return t.rotation; }, [](Amara::Particle& t, double v) { t.rotation = v; }),
                "scale", sol::property([](Amara::Particle& t) -> Vector2& { return t.scale; }, [](Amara::Particle& t, sol::object v) { t.scale = v; }),
                "scaleX", sol::property([](Amara::Particle& t) -> double { return t.scale.x; }, [](Amara::Particle& t, double v) { t.scale.x = v; }),
                "scaleY", sol::property([](Amara::Particle& t) -> double { return t.scale.y; }, [](Amara::Particle& t, double v) { t.scale.y = v; }),
                "alpha", sol::property([](Amara::Particle& t) -> double { return t.alpha; }, [](Amara::Particle& t, double v) { t.alpha = v; }),
                "velocity", sol::property([](Amara::Particle& t) -> Vector2& { return t.velocity; }, [](Amara::Particle& t, sol::object v) { t.velocity = v; }),
                "velocityX", sol::property([](Amara::Particle& t) -> double { return t.velocity.x; }, [](Amara::Particle& t, double v) { t.velocity.x = v; }),
                "velocityY", sol::property([](Amara::Particle& t) -> double { return t.velocity.y; }, [](Amara::Particle& t, double v) { t.velocity.y = v; }),
                "acceleration", sol::property([](Amara::Particle& t) -> Vector2& { return t.acceleration; }, [](Amara::Particle& t, sol::object v) { t.acceleration = v; }),
                "accelerationX", sol::property([](Amara::Particle& t) -> double { return t.acceleration.x; }, [](Amara::Particle& t, double v) { t.acceleration.x = v; }),
                "accelerationY", sol::property([](Amara::Particle& t) -> double { return t.acceleration.y; }, [](Amara::Particle& t, double v) { t.acceleration.y = v; }),
                "rotationalVelocity", sol::property([](Amara::Particle& t) -> double { return t.rotationalVelocity; }, [](Amara::Particle& t, double v) { t.rotationalVelocity = v; }),
                "lifeTime", sol::readonly(&Amara::Particle::lifeTime),
                "progress", sol::readonly(&Amara::Particle::progress),
                "delay", &Amara::Particle::delay,
                "tint", sol::property([](Amara::Particle& t) -> Color& { return t.tint; }, [](Amara::Particle& t, sol::object v) { t.tint = v; })
            );
        }
    };

    class ParticleEmitter: public Amara::Sprite {
    public:
        std::vector<Particle> particles;

        Ease easing = Ease::Linear;

        int poolSize = 128;
        double spawnRate = 1;
        double spawn_timer = 0;
        
        bool spawning = false;
        int spawnedCount = 0;
        int end_particle = -1;
        double particle_lifetime = 1;
        
        nlohmann::json start_data;
        nlohmann::json end_data;

        bool updated_this_frame = false;
        
        bool yoyo = false;
        
        ParticleEmitter(): Amara::Sprite() {
            set_base_node_id("ParticleEmitter");
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "spawning")) {
                spawning = config["spawning"];
            }
            if (json_has(config, "ease")) {
                easing = config["ease"];
            }
            if (json_has(config, "poolSize")) {
                int _poolSize = config["poolSize"];
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
            if (json_has(config, "spawnRate")) {
                spawnRate = config["spawnRate"];
            }
            if (json_has(config, "spawning")) {
                spawning = config["spawning"];
            }
            if (json_has(config, "from")) {
                start_data = config["from"];
            }
            if (json_has(config, "to")) {
                end_data = config["to"];
            }
            if (json_has(config, "lifeTime")) {
                particle_lifetime = config["lifeTime"];
            }
            if (json_has(config, "yoyo")) {
                yoyo = config["yoyo"];
            }
            
            return Amara::Sprite::configure(config);
        }

        virtual sol::object luaConfigure(std::string key, sol::object val) override {
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
            return Amara::Sprite::luaConfigure(key, val);
        }
        
        virtual void update(double deltaTime) override {
            Amara::Sprite::update(deltaTime);
            updated_this_frame = false;
        }
        
        void setProperty(Particle& p, std::string key, const nlohmann::json& data) {
            if (p.luatable[key].is<int>()) {
                p.luatable[key] = static_cast<int>(std::floor(data.get<double>()));
            }
            else if (p.luatable[key].is<double>()) {
                p.luatable[key] = (float)data.get<double>();
            }
            else if (p.luatable[key].is<Amara::Vector2>()) {
                Vector2 val = data; 
                p.luatable[key] = val;
            }
            else if (p.luatable[key].is<Amara::Rectangle>()) {
                Rectangle val = data;
                p.luatable[key] = val;
            }
            else if (p.luatable[key].is<Amara::Color>()) {
                Color val = data;
                p.luatable[key] = val;
            }
        }
        
        void tweenProperty(Particle& p, std::string key, const nlohmann::json& val1, const nlohmann::json& val2) {
            double real_progress = (yoyo && !p.waitingYoyo) ? 1.0 - p.progress : p.progress;
            real_progress = std::clamp(real_progress, 0.0, 1.0);
            
            if (p.luatable[key].is<int>()) {
                int start = val1.get<int>();
                int end = val2.get<int>();
                p.luatable[key] = static_cast<int>(std::round(ease((double)start, (double)end, real_progress)));
            }
            else if (p.luatable[key].is<double>()) {
                float start = val1.get<float>();
                float end = val2.get<float>();
                p.luatable[key] = ease((double)start, (double)end, real_progress, easing);
            }
            else if (p.luatable[key].is<Amara::Vector2>()) {
                Vector2 start = val1;
                Vector2 end = val2;
                p.luatable[key] = Vector2(
                    ease(start.x, end.x, real_progress, easing),
                    ease(start.y, end.y, real_progress, easing)
                );
            }
            else if (p.luatable[key].is<Amara::Rectangle>()) {
                Rectangle start = val1;
                Rectangle end = val2;
                p.luatable[key] = Rectangle(
                    ease(start.x, end.x, real_progress, easing),
                    ease(start.y, end.y, real_progress, easing),
                    ease(start.w, end.w, real_progress, easing),
                    ease(start.h, end.h, real_progress, easing)
                );
            }
            else if (p.luatable[key].is<Amara::Color>()) {
                Color start = val1;
                Color end = val2;
                p.luatable[key] = Color(
                    ease(start.r, end.r, real_progress, easing),
                    ease(start.g, end.g, real_progress, easing),
                    ease(start.b, end.b, real_progress, easing),
                    ease(start.a, end.a, real_progress, easing)
                );
            }
        }
        
        nlohmann::json convertProperty(Particle& p, std::string key, const nlohmann::json& data) {
            if (data.is_object() && json_has(data, "min", "max")) {
                nlohmann::json min = data["min"];
                nlohmann::json max = data["max"];
                if (p.luatable[key].is<int>()) {
                    int minVal = min.get<double>();
                    int maxVal = max.get<double>();
                    return static_cast<int>(std::round(ease((double)minVal, (double)maxVal, lua_random(gameProps->lua))));
                }
                else if (p.luatable[key].is<double>()) {
                    double minVal = min.get<double>();
                    double maxVal = max.get<double>();
                    return ease(minVal, maxVal, lua_random(gameProps->lua));
                }
                else if (p.luatable[key].is<Vector2>()) {
                    Vector2 minVal = min;
                    Vector2 maxVal = max;
                    p.luatable[key] = Vector2(
                        ease(minVal.x, maxVal.x, lua_random(gameProps->lua)),
                        ease(minVal.y, maxVal.y, lua_random(gameProps->lua))
                    );
                }
                else if (p.luatable[key].is<Rectangle>()) {
                    Rectangle minVal = min;
                    Rectangle maxVal = max;
                    return Rectangle(
                        ease(minVal.x, maxVal.x, lua_random(gameProps->lua)),
                        ease(minVal.y, maxVal.y, lua_random(gameProps->lua)),
                        ease(minVal.w, maxVal.w, lua_random(gameProps->lua)),
                        ease(minVal.h, maxVal.h, lua_random(gameProps->lua))
                    ).toJSON();
                }
                else if (p.luatable[key].is<Color>()) {
                    Color minVal = min;
                    Color maxVal = max;
                    return Color(
                        ease(minVal.r, maxVal.r, lua_random(gameProps->lua)),
                        ease(minVal.g, maxVal.g, lua_random(gameProps->lua)),
                        ease(minVal.b, maxVal.b, lua_random(gameProps->lua)),
                        ease(minVal.a, maxVal.a, lua_random(gameProps->lua))
                    ).toJSON();
                }
            }
            return data;
        }

        void initParticle(Particle& particle, const nlohmann::json& config) {
            particle.lifeTime = 0;
            particle.progress = 0;

            particle.pos = Vector2(0);
            particle.frame = frame;
            particle.origin = Vector2(0.5);
            particle.scale = Vector2(1);
            particle.velocity = Vector2(0);
            particle.acceleration = Vector2(0);
            particle.rotation = 0;
            particle.rotationalVelocity = 0;
            particle.alpha = 1;
            particle.delay = 0;
            particle.tint = Color::White;
            
            particle.waitingYoyo = yoyo;
            
            if (!particle.luaobject.valid() || particle.keep != &particle) {
                particle.luaobject = sol::make_object(gameProps->lua, &particle);
                particle.luatable = particle.luaobject.as<sol::table>();
                particle.keep = &particle;
            }
            
            for (auto it = config.begin(); it != config.end(); ++it) {
                setProperty(particle, it.key(), convertProperty(particle, it.key(), it.value()));
            }
            
            particle.start_data = nlohmann::json::object();
            for (auto it = end_data.begin(); it != end_data.end(); ++it) {
                particle.end_data[it.key()] = convertProperty(particle, it.key(), it.value());
                particle.start_data[it.key()] = lua_to_json(particle.luatable[it.key()]);
            }
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
                    particle.luatable = particle.luaobject.as<sol::table>();
                    particle.keep = &particle;
                }
            }
        }

        void burst(double amount, sol::object lua_config) {
            sol::function onSpawn = funcs.getFunction("onParticleSpawn");

            nlohmann::json new_config = start_data;
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
                                particle.luatable = particle.luaobject.as<sol::table>();
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
                        (passOn.anchor.x + render_pos.x*passOn.scale.x),
                        (passOn.anchor.y + render_pos.y*passOn.scale.y)
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
                anchoredPos.x + (cropLeft - imgw*particle.origin.x)*scale.x*passOn.scale.x*particle.scale.x,
                anchoredPos.y - anchoredPos.z + (cropTop - imgh*particle.origin.y)*scale.y*passOn.scale.y*particle.scale.y,
                (imgw - cropLeft - cropRight)*scale.x*passOn.scale.x*particle.scale.x,
                (imgh - cropTop - cropBottom)*scale.y*passOn.scale.y*particle.scale.y
            };

            destRect.x = vcenter.x + dim.x*totalZoom.x;
            destRect.y = vcenter.y + dim.y*totalZoom.y;
            destRect.w = dim.w * totalZoom.x;
            destRect.h = dim.h * totalZoom.y;

            SDL_FPoint dorigin = {
                (imgw*particle.origin.x - cropLeft)*scale.x*passOn.scale.x*totalZoom.x*particle.scale.x,
                (imgh*particle.origin.y - cropTop)*scale.y*passOn.scale.y*totalZoom.y*particle.scale.y
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
                    passOn.rotation + rotation + particle.rotation
                );
                input.queueInput(moveQuad(inputQuad, v.x, v.y), v, nullptr);
            }

            Amara::Color particleTint = particle.tint * tint;
            
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
                    if (particle.lifeTime >= particle_lifetime && !updated_this_frame) {\
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
                    initParticle(particle, start_data);
                    if (onSpawn.valid()) {
                        try {
                            if (!particle.luaobject.valid() || particle.keep != &particle) {
                                particle.luaobject = sol::make_object(gameProps->lua, &particle);
                                particle.luatable = particle.luaobject.as<sol::table>();
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
                    if (particle.delay > 0) {
                        particle.delay -= deltaTime;
                        if (particle.delay < 0) {
                            particle.delay = 0;
                        }
                    }
                    else {
                        particle.pos += particle.velocity * deltaTime + particle.acceleration * deltaTime * deltaTime * 0.5f;
                        particle.velocity += particle.acceleration * deltaTime;
                        particle.rotation += particle.rotationalVelocity * deltaTime;
                        
                        for (auto it = particle.end_data.begin(); it != particle.end_data.end(); ++it) {
                            tweenProperty(particle, it.key(), particle.start_data[it.key()], it.value());
                        }
                        
                        if (onUpdate_defined) {
                            sol::function func = funcs.getFunction("onParticleUpdate");
                            try {
                                if (!particle.luaobject.valid() || particle.keep != &particle) {
                                    particle.luaobject = sol::make_object(gameProps->lua, &particle);
                                    particle.luatable = particle.luaobject.as<sol::table>();
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
                }

                last_particle = i;

                drawParticle(v, particle);

                if (!updated_this_frame && particle.delay == 0) {
                    particle.lifeTime += deltaTime;
                    if (particle.lifeTime >= particle_lifetime) {
                        if (particle.waitingYoyo) {
                            particle.lifeTime -= particle_lifetime;
                            particle.waitingYoyo = false;
                        }
                    }
                }
                particle.progress = particle.lifeTime / particle_lifetime;
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