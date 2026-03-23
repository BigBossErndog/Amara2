namespace Amara {
    /**
     * Camera for a 2D scene.
     */
    class Camera: public Amara::Node {
    public:
        Rectangle viewport;
        bool sizeTethered = true;

        float width = 0;
        float height = 0;

        Vector2 origin = { 0.5, 0.5 };

        Vector2 scroll = { 0, 0 };
        Vector2 zoom = { 1, 1 };
        Vector2 offset = { 0, 0 };
        
        Vector2 center = { 0, 0 };
        float left = 0;
        float right = 0;
        float top = 0;
        float bottom = 0;

        Rectangle view;

        bool hasBounds = false;
        Rectangle bounds;

        Amara::Node* followTarget = nullptr;
        Vector2 lerp = { 1, 1 };
        static constexpr float lerpPower = 3.0f;

        Camera(): Node() {
            set_base_node_id("Camera");
            is_camera = true;
        }

        virtual nlohmann::json toJSON() {
            nlohmann::json data = Amara::Node::toJSON();

            data["scrollX"] = scroll.x;
            data["scrollY"] = scroll.y;

            data["zoomX"] = zoom.x;
            data["zoomY"] = zoom.y;
            
            data["originX"] = origin.x;
            data["originY"] = origin.y;

            data["lerpX"] = lerp.x;
            data["lerpY"] = lerp.y;

            return data;
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            Amara::Node::configure(config);
            
            if (json_has(config, "scrollX")) scroll.x = json_get<float>(config, "scrollX");
            if (json_has(config, "scrollY")) scroll.y = json_get<float>(config, "scrollY");
            if (json_has(config, "scroll")) scroll = json_get<Vector2>(config, "scroll");

            if (json_has(config, "zoomX")) zoom.x = json_get<float>(config, "zoomX");
            if (json_has(config, "zoomY")) zoom.y = json_get<float>(config, "zoomY");
            if (json_has(config, "zoom")) zoom = json_get<Vector2>(config, "zoom");

            if (json_has(config, "originX")) origin.x = json_get<float>(config, "originX");
            if (json_has(config, "originY")) origin.y = json_get<float>(config, "originY");
            if (json_has(config, "origin")) origin = json_get<Vector2>(config, "origin");

            if (json_has(config, "lerpX")) lerp.x = json_get<float>(config, "lerpX");
            if (json_has(config, "lerpY")) lerp.y = json_get<float>(config, "lerpY");
            if (json_has(config, "lerp")) lerp = json_get<Vector2>(config, "lerp");
            
            if (json_has(config, "width")) setWidth(json_get<float>(config, "width"));
            else if (json_has(config, "w")) setWidth(json_get<float>(config, "w"));
            
            if (json_has(config, "height")) setHeight(json_get<float>(config, "height"));
            else if (json_has(config, "h")) setHeight(json_get<float>(config, "h"));
            
            if (json_has(config, "size")) {
                setSize(json_get<Rectangle>(config, "size"));
            }
            if (json_has(config, "bounds")) setBounds(json_get<Rectangle>(config, "bounds"));
            
            update_bounds();

            return this;
        }

        virtual void init() override {
            Amara::Node::init();
            
            passOn = gameProps->passOn;
            if (sizeTethered) {
                viewport = gameProps->master_viewport;
                width = ((viewport.w / passOn.scale.x) / passOn.zoom.x)/passOn.window_zoom.x;
                height = ((viewport.h / passOn.scale.y) / passOn.zoom.y)/passOn.window_zoom.y;
            }

            update_bounds();
        }

        virtual void create() override {
            Amara::Node::create();
            update_bounds();
        }

        virtual void run(double deltaTime) override {
            Amara::Node::run(deltaTime);

            if (followTarget) {
                float tx = followTarget->pos.x + followTarget->cameraFollowOffset.x;
                float mappedLerpX = 1.0f - std::pow(1.0f - lerp.x, lerpPower * deltaTime);
                scroll.x += (tx - scroll.x) * mappedLerpX;

                float ty = followTarget->pos.y + followTarget->cameraFollowOffset.y;
                float mappedLerpY = 1.0f - std::pow(1.0f - lerp.y, lerpPower * deltaTime);
                scroll.y += (ty - scroll.y) * mappedLerpY;
            }

            update_bounds();
        }

        void update_borders() {
            center = scroll + offset;
            left = center.x - (width/zoom.x)/2.0;
            right = center.x + (width/zoom.x)/2.0;
            top = center.y - (height/zoom.y)/2.0;
            bottom = center.y + (height/zoom.y)/2.0;

            view.x = left;
            view.y = top;
            
            view.w = right - left;
            view.h = bottom - top;
        }

        void update_bounds() {
            update_borders();

            if (hasBounds) {
                if (bounds.w < view.w) {
                    scroll.x = bounds.x + bounds.w/2;
                }
                else {
                    if (left < bounds.x) scroll.x = bounds.x + view.w/2 - offset.x;
                    if (right > bounds.x + bounds.w) scroll.x = bounds.x + bounds.w - view.w/2 - offset.x;
                }
                if (bounds.h < view.h) {
                    scroll.y = bounds.y + bounds.h/2 - offset.y;
                }
                else {
                    if (top < bounds.y) scroll.y = bounds.y + view.h/2 - offset.y;
                    if (bottom > bounds.y + bounds.h) scroll.y = bounds.y + bounds.h - view.h/2 - offset.y;
                }
            }
            
            update_borders();
        }
        
        sol::object setBounds(sol::object _bounds) {
            if (_bounds.valid()) {
                hasBounds = true;
                bounds = _bounds;
            }
            else {
                hasBounds = false;
            }
            update_bounds();
            return get_lua_object();
        }
        sol::object setBounds(const Rectangle& _bounds) {
            hasBounds = true;
            bounds = _bounds;
            update_bounds();
            return get_lua_object();
        }
        sol::object setBounds(float _x, float _y, float _w, float _h) {
            return setBounds(Rectangle( _x, _y, _w, _h ));
        }
        void setBounds(nlohmann::json config) {
            Rectangle rect = config;
            setBounds(rect);
        }

        sol::object removeBounds() {
            hasBounds = false;
            return get_lua_object();
        }

        sol::object focusOn(float _x, float _y) {
            scroll = Vector2( _x, _y );
            update_bounds();
            return get_lua_object();
        }

        sol::object focusOn(Amara::Node* node) {
            return focusOn(node->pos.x, node->pos.y);
        }

        sol::object startFollow(Amara::Node* node, float _lx, float _ly) {
            lerp = Vector2( _lx, _ly );
            followTarget = node;
            focusOn(node);
            return get_lua_object();
        }

        sol::object startFollow(Amara::Node* node, float _l) {
            return startFollow(node, _l, _l);
        }

        sol::object startFollow(Amara::Node* node) {
            return startFollow(node, lerp.x, lerp.y);
        }

        sol::object stopFollow() {
            followTarget = nullptr;
            return get_lua_object();
        }
        
        sol::object changeScroll(float _sx, float _sy) {
            scroll.x += _sx;
            scroll.y += _sy;
            return get_lua_object();
        }

        sol::object changeScroll(float _s) {
            return changeScroll(_s, _s);
        }

        sol::object changeZoom(float _zx, float _zy) {
            zoom.x += _zx;
            zoom.y += _zy;
            return get_lua_object();
        }

        sol::object changeZoom(float _z) {
            return changeZoom(_z, _z);
        }

        void setSize(Rectangle rect) {
            pos.x = rect.x + (rect.w * origin.x);
            pos.y = rect.y + (rect.h * origin.y);
            width = rect.w;
            height = rect.h;

            sizeTethered = false;
        }
        void setSize(nlohmann::json config) {
            Rectangle rect = config;
            setSize(rect);
        }

        sol::object lua_setSize(sol::object _s) {
            Rectangle rect = _s;
            setSize(rect);
            return get_lua_object();
        }

        sol::object setWidth(double _w) {
            sizeTethered = false;
            width = _w;
            return get_lua_object();
        }

        sol::object setHeight(double _h) {
            sizeTethered = false;
            height = _h;
            return get_lua_object();
        }

        virtual void pass_on_properties() override {
            passOn = gameProps->passOn;

            if (passOnPropsEnabled) {
                passOn.anchor = Vector3( 
                    rotateAroundAnchor(
                        Vector2(0, 0),
                        Vector2(-scroll.x -offset.x, -scroll.y -offset.y),
                        rotation
                    ),
                    0
                );

                passOn.rotation += rotation;

                passOn.scale = Vector2(
                    gameProps->passOn.scale.x * scale.x,
                    gameProps->passOn.scale.y * scale.y
                );

                passOn.scroll = Vector2(
                    gameProps->passOn.scroll.x + scroll.x,
                    gameProps->passOn.scroll.y + scroll.y
                );

                passOn.zoom = Vector2(
                    gameProps->passOn.zoom.x * zoom.x,
                    gameProps->passOn.zoom.y * zoom.y
                );

                gameProps->passOn = passOn;
            }
        }
        
        virtual void drawChildren(const Rectangle& v) override {
            if (parent->children.size() == 0) return;
            
            children_copy_list = parent->children;

            Vector2 vcenter = Vector2(v.w/2.0f, v.h/2.0f);

            if (sizeTethered) {
                viewport = v;
                width = ((v.w / passOn.scale.x) / passOn.zoom.x)/passOn.window_zoom.x;
                height = ((v.h / passOn.scale.y) / passOn.zoom.y)/passOn.window_zoom.y;
            }
            else {
                viewport = Rectangle(
                    vcenter.x + (pos.x - (width*origin.x)*scale.x*passOn.scale.x)*passOn.zoom.x*passOn.window_zoom.x,
                    vcenter.y + (pos.y - (height*origin.y)*scale.y*passOn.scale.y - pos.z)*passOn.zoom.y*passOn.window_zoom.y,
                    width*scale.x*passOn.scale.x*passOn.zoom.x*passOn.window_zoom.x,
                    height*scale.y*passOn.scale.x*passOn.zoom.y*passOn.window_zoom.y
                );
            }

            SDL_Rect old_sdl_viewport;
            if (gameProps->graphics == GraphicsEnum::Render2D && gameProps->renderer) {
                SDL_GetRenderViewport(gameProps->renderer, &old_sdl_viewport);
                
                SDL_Rect setv = Rectangle::makeSDLRect(viewport);
                SDL_SetRenderViewport(gameProps->renderer, &setv);
            }
            #ifdef AMARA_OPENGL
            GLint old_gl_viewport[4];
            if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                glGetIntegerv(GL_VIEWPORT, old_gl_viewport);
                glViewport(viewport.x, gameProps->master_viewport.h - (viewport.y + viewport.h), viewport.w, viewport.h);
            }
            #endif
            
            PassOnProps rec_passOn = gameProps->passOn;
            pass_on_properties();

            passOn.insideCamera = true;
            gameProps->passOn = passOn;

            Amara::Node* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = *it;
				if (child == nullptr || child->destroyed || !child->visible || child->parent != parent) {
					++it;
					continue;
				}

                if (passOn.insideCamera && child->is_camera) {
                    ++it;
                    continue;
                }
                
                update_properties();
				child->draw(viewport);
                
                gameProps->passOn = passOn;
				++it;
			}

            gameProps->passOn = rec_passOn;

            if (gameProps->graphics == GraphicsEnum::Render2D && gameProps->renderer) {
                SDL_SetRenderViewport(gameProps->renderer, &old_sdl_viewport);
            }
            #ifdef AMARA_OPENGL
            else if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                gameProps->renderBatch->flush();
                glViewport(old_gl_viewport[0], old_gl_viewport[1], old_gl_viewport[2], old_gl_viewport[3]);
            }
            #endif
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Camera>("Camera",
                sol::base_classes, sol::bases<Amara::Node>(),
                "w", &Camera::width,
                "h", &Camera::height,
                "width", sol::property(
                    [](const Camera& cam) { return cam.width; },
                    &Camera::setWidth
                ),
                "height", sol::property(
                    [](const Camera& cam) { return cam.height; },
                    &Camera::setHeight
                ),
                "scroll", sol::property(
                    [] (Camera& cam) -> Vector2& { return cam.scroll; },
                    [] (Camera& cam, sol::object _s) { cam.scroll = _s; }
                ),
                "scrollX", sol::property([](Camera& cam) { return cam.scroll.x; }, [](Camera& cam, float val) { cam.scroll.x = val; }),
                "scrollY", sol::property([](Camera& cam) { return cam.scroll.y; }, [](Camera& cam, float val) { cam.scroll.y = val; }),
                "changeScroll", sol::overload(
                    sol::resolve<sol::object(float, float)>(&Camera::changeScroll),
                    sol::resolve<sol::object(float)>(&Camera::changeScroll)
                ),
                "offset", sol::property(
                    [] (Camera& cam) -> Vector2& { return cam.offset; },
                    [] (Camera& cam, sol::object _o) { cam.offset = _o; }
                ),
                "offsetX", sol::property([](Camera& cam) { return cam.offset.x; }, [](Camera& cam, float val) { cam.offset.x = val; }),
                "offsetY", sol::property([](Camera& cam) { return cam.offset.y; }, [](Camera& cam, float val) { cam.offset.y = val; }),
                "zoom", sol::property(
                    [] (Camera& cam) -> Vector2& { return cam.zoom; },
                    [] (Camera& cam, sol::object _z) { cam.zoom = _z; }
                ),
                "zoomX", sol::property([](Camera& cam) { return cam.zoom.x; }, [](Camera& cam, float val) { cam.zoom.x = val; }),
                "zoomY", sol::property([](Camera& cam) { return cam.zoom.y; }, [](Camera& cam, float val) { cam.zoom.y = val; }),
                "origin", sol::property([](Amara::Camera& t) -> Vector2& { return t.origin; }, [](Amara::Camera& t, sol::object v) { t.origin = v; }),
                "originX", sol::property([](Amara::Camera& t) -> float { return t.origin.x; }, [](Amara::Camera& t, float v) { t.origin.x = v; }),
                "originY", sol::property([](Amara::Camera& t) -> float { return t.origin.y; }, [](Amara::Camera& t, float v) { t.origin.y = v; }),
                "changeZoom", sol::overload(
                    sol::resolve<sol::object(float, float)>(&Camera::changeZoom),
                    sol::resolve<sol::object(float)>(&Camera::changeZoom)
                ),
                "lerp", sol::property([](Amara::Camera& cam) -> Vector2& { return cam.lerp; }, [](Amara::Camera& cam, sol::object val) { cam.lerp = val; } ),
                "lerpX", sol::property([](Camera& cam) { return cam.lerp.x; }, [](Camera& cam, float val) { cam.lerp.x = val; }),
                "lerpY", sol::property([](Camera& cam) { return cam.lerp.y; }, [](Camera& cam, float val) { cam.lerp.y = val; }),
                "center", sol::readonly(&Camera::center),
                "left", sol::readonly(&Camera::left),
                "right", sol::readonly(&Camera::right),
                "top", sol::readonly(&Camera::top),
                "bottom", sol::readonly(&Camera::bottom),
                "view", sol::readonly(&Camera::view),
                "rect", sol::readonly(&Camera::view),
                "size", sol::property(
                    [](Camera& cam) { return Rectangle(cam.pos.x - (cam.width * cam.origin.x), cam.pos.y - (cam.height * cam.origin.y), cam.width, cam.height); },
                    [](Camera& cam, sol::object val) { cam.setSize(val); }
                ),
                "startFollow", sol::overload(
                    sol::resolve<sol::object(Amara::Node*)>(&Camera::startFollow),
                    sol::resolve<sol::object(Amara::Node*, float, float)>(&Camera::startFollow),
                    sol::resolve<sol::object(Amara::Node*, float)>(&Camera::startFollow)
                ),
                "stopFollow", &Camera::stopFollow,
                "followTarget", sol::property(
                    [](Amara::Camera& c) { return c.followTarget; },
                    [](Amara::Camera& c, sol::object val) {
                        if (val.is<Amara::Node>()) {
                            c.startFollow(val.as<Amara::Node*>());
                        }
                        else {
                            c.stopFollow();
                        }
                    }
                ),
                "sizeTethered", &Camera::sizeTethered,
                "focusOn", sol::overload(
                    sol::resolve<sol::object(float, float)>(&Camera::focusOn),
                    sol::resolve<sol::object(Amara::Node*)>(&Camera::focusOn)
                ),
                "setBounds", sol::overload(
                    sol::resolve<sol::object(const Rectangle&)>(&Camera::setBounds),
                    sol::resolve<sol::object(float, float, float, float)>(&Camera::setBounds),
                    sol::resolve<sol::object(sol::object)>(&Camera::setBounds)
                ),
                "removeBounds", &Camera::removeBounds,
                "bounds", sol::property([](Camera& cam) -> sol::object { if (cam.hasBounds) { return sol::make_object(cam.gameProps->lua, cam.bounds); } else { return sol::nil; } }, [](Camera& cam, sol::object obj) { return cam.setBounds(obj); }),
                "hasBounds", sol::readonly(&Camera::hasBounds)
            );
        }
    };
}