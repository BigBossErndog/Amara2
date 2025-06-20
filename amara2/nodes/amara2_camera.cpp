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

        Vector2 origin = { 0, 0 };

        Vector2 scroll = { 0, 0 };
        Vector2 zoom = { 1, 1 };

        Vector2 center = { 0, 0 };
        float left = 0;
        float right = 0;
        float top = 0;
        float bottom = 0;

        Rectangle view;

        bool hasBounds = false;
        Rectangle bounds;

        Amara::Node* followTarget = nullptr;
        Vector2 lerp = { 0, 0 };

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

            if (json_has(config, "scrollX")) scroll.x = config["scrollX"];
            if (json_has(config, "scrollY")) scroll.y = config["scrollY"];
            if (json_has(config, "scroll")) scroll = config["scroll"];

            if (json_has(config, "zoomX")) zoom.x = config["zoomX"];
            if (json_has(config, "zoomY")) zoom.y = config["zoomY"];
            if (json_has(config, "zoom")) zoom = config["zoom"];

            if (json_has(config, "originX")) origin.x = config["originX"];
            if (json_has(config, "originY")) origin.y = config["originY"];
            if (json_has(config, "origin")) origin = config["origin"];

            if (json_has(config, "lerpX")) lerp.x = config["lerpX"];
            if (json_has(config, "lerpY")) lerp.y = config["lerpY"];
            if (json_has(config, "lerp")) lerp = config["lerp"];

            if (json_has(config, "width")) width = config["width"];
            if (json_has(config, "height")) height = config["height"];

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

        virtual void run(double deltaTime) override {
            Amara::Node::run(deltaTime);

            if (followTarget) {
                float tx = followTarget->pos.x + followTarget->cameraFollowOffset.x;
                if (lerp.x > 0) scroll.x = scroll.x + (tx - scroll.x)*(1.0f - std::exp(-lerp.x*deltaTime));
                else scroll.x = tx;

                float ty = followTarget->pos.y + followTarget->cameraFollowOffset.y;
                if (lerp.y) scroll.y = scroll.y + (ty - scroll.y)*(1.0f - std::exp(-lerp.y*deltaTime));
                else scroll.y = ty;
            }

            update_bounds();
        }

        void update_borders() {
            center = scroll;
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
                    if (left < bounds.x) scroll.x = bounds.x + view.w/2;
                    if (right > bounds.x + bounds.w) scroll.x = bounds.x + bounds.w - view.w/2;
                }
                if (bounds.h < view.h) {
                    scroll.y = bounds.y + bounds.h/2;
                }
                else {
                    if (top < bounds.y) scroll.y = bounds.y + view.h/2;
                    if (bottom > bounds.y + bounds.h) scroll.y = bounds.y + bounds.h - view.h/2;
                }
            }

            update_borders();
        }
        
        sol::object setBounds(sol::object _bounds) {
            if (!_bounds.is<sol::nil_t>()) {
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
            return startFollow(node, 0);
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

        virtual void pass_on_properties() override {
            passOn = gameProps->passOn;

            if (passOnPropsEnabled) {
                passOn.anchor = Vector3( 
                    rotateAroundAnchor(
                        Vector2(0, 0),
                        Vector2(-scroll.x, -scroll.y),
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
            children_copy_list = parent->children;
            SDL_Rect setv = Rectangle::makeSDLRect(v);
            SDL_SetRenderViewport(gameProps->renderer, &setv);

            if (sizeTethered) {
                viewport = v;
                width = ((v.w / passOn.scale.x) / passOn.zoom.x)/passOn.window_zoom.x;
                height = ((v.h / passOn.scale.y) / passOn.zoom.y)/passOn.window_zoom.y;
            }
            else {
                viewport = Rectangle(
                    (pos.x - (width*origin.x)*scale.x*passOn.scale.x)*passOn.zoom.x*passOn.window_zoom.x,
                    (pos.y - (height*origin.y)*scale.y*passOn.scale.y - pos.z)*passOn.zoom.y*passOn.window_zoom.y,
                    width*scale.x*passOn.scale.x*passOn.zoom.x*passOn.window_zoom.x, 
                    height*scale.y*passOn.scale.x*passOn.zoom.y*passOn.window_zoom.y
                );
            }

            pass_on_properties();

            Amara::Node* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = *it;
				if (child == nullptr || child->destroyed || !child->visible || child->parent != parent) {
					++it;
					continue;
				}
                
                update_properties();
				if (!child->is_camera) child->draw(viewport);

                gameProps->passOn = passOn;
				++it;
			}
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Camera>("Camera",
                sol::base_classes, sol::bases<Amara::Node>(),
                "w", &Camera::width,
                "h", &Camera::height,
                "width", &Camera::width,
                "height", &Camera::height,
                "scroll", &Camera::scroll,
                "scrollX", sol::property([](Camera& cam) { return cam.scroll.x; }, [](Camera& cam, float val) { cam.scroll.x = val; }),
                "scrollY", sol::property([](Camera& cam) { return cam.scroll.x; }, [](Camera& cam, float val) { cam.scroll.y = val; }),
                "changeScroll", sol::overload(
                    sol::resolve<sol::object(float, float)>(&Camera::changeScroll),
                    sol::resolve<sol::object(float)>(&Camera::changeScroll)
                ),
                "zoom", sol::property(
                    [] (Camera& cam) -> Vector2 { return cam.zoom; },
                    [] (Camera& cam, sol::object _z) { cam.zoom = _z; }
                ),
                "zoomX", sol::property([](Camera& cam) { return cam.zoom.x; }, [](Camera& cam, float val) { cam.zoom.x = val; }),
                "zoomY", sol::property([](Camera& cam) { return cam.zoom.y; }, [](Camera& cam, float val) { cam.zoom.y = val; }),
                "changeZoom", sol::overload(
                    sol::resolve<sol::object(float, float)>(&Camera::changeZoom),
                    sol::resolve<sol::object(float)>(&Camera::changeZoom)
                ),
                "lerp", &Camera::lerp,
                "lerpX", sol::property([](Camera& cam) { return cam.lerp.x; }, [](Camera& cam, float val) { cam.lerp.x = val; }),
                "lerpY", sol::property([](Camera& cam) { return cam.lerp.y; }, [](Camera& cam, float val) { cam.lerp.y = val; }),
                "center", sol::readonly(&Camera::center),
                "left", sol::readonly(&Camera::left),
                "right", sol::readonly(&Camera::right),
                "top", sol::readonly(&Camera::top),
                "bottom", sol::readonly(&Camera::bottom),
                "view", sol::readonly(&Camera::view),
                "rect", sol::readonly(&Camera::view),
                "startFollow", sol::overload(
                    sol::resolve<sol::object(Amara::Node*)>(&Camera::startFollow),
                    sol::resolve<sol::object(Amara::Node*, float, float)>(&Camera::startFollow),
                    sol::resolve<sol::object(Amara::Node*, float)>(&Camera::startFollow)
                ),
                "stopFollow", &Camera::stopFollow,
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