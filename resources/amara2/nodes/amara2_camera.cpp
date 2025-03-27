namespace Amara {
    class Camera: public Amara::Node {
    public:
        Rectangle viewport;
        bool isSizeTethered = true;

        float width = 0;
        float height = 0;

        Vector2 origin = { 0, 0 };

        Vector2 scroll = { 0, 0 };
        Vector2 zoom = { 1, 1 };

        Vector2 center = { 0, 0 };
        float leftBorder = 0;
        float rightBorder = 0;
        float topBorder = 0;
        float bottomBorder = 0;

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

            if (json_has(config, "zoomX")) zoom.x = config["zoomX"];
            if (json_has(config, "zoomY")) zoom.y = config["zoomY"];

            if (json_has(config, "originX")) origin.x = config["originX"];
            if (json_has(config, "originY")) origin.y = config["originY"];

            if (json_has(config, "lerpX")) lerp.x = config["lerpX"];
            if (json_has(config, "lerpY")) lerp.y = config["lerpY"];

            return this;
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

            center = { scroll.x, scroll.y };
            leftBorder = center.x - (width/zoom.x)/2.0;
            rightBorder = center.x + (width/zoom.x)/2.0;
            topBorder = center.y - (height/zoom.y)/2.0;
            bottomBorder = center.y + (height/zoom.y)/2.0;
        }

        sol::object focusOn(float _x, float _y) {
            scroll = { _x, _y };
            return get_lua_object();
        }

        sol::object focusOn(Amara::Node* node) {
            return focusOn(node->pos.x, node->pos.y);
        }

        sol::object startFollow(Amara::Node* node, float _lx, float _ly) {
            lerp = { _lx, _ly };
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

        virtual void pass_on_properties() override {
            passOn = Props::passOn;

            if (passOnPropsEnabled) {
                passOn.anchor = { 0, 0, 0 };

                passOn.rotation += rotation;

                passOn.scale = {
                    Props::passOn.scale.x * scale.x,
                    Props::passOn.scale.y * scale.y
                };

                passOn.scroll = {
                    Props::passOn.scroll.x + scroll.x,
                    Props::passOn.scroll.y + scroll.y
                };

                passOn.zoom = {
                    Props::passOn.zoom.x * zoom.x,
                    Props::passOn.zoom.y * zoom.y
                };

                Props::passOn = passOn;
            }
        }
        
        virtual void drawChildren(const Rectangle& v) override {
            children_copy_list = parent->children;

            if (isSizeTethered) {
                viewport = v;
            }
            else {
                viewport = {
                    (pos.x - (width*origin.x)*scale.x*passOn.scale.x)*passOn.zoom.x,
                    (pos.y - (height*origin.y)*scale.y*passOn.scale.y - pos.z)*passOn.zoom.y,
                    width*scale.x*passOn.scale.x*passOn.zoom.x, 
                    height*scale.y*passOn.scale.x*passOn.zoom.y
                };
            }

            pass_on_properties();

            Amara::Node* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = *it;
				if (child == nullptr || child->isDestroyed || child->parent != parent) {
					++it;
					continue;
				}
                
                update_properties();
				if (!child->is_camera) child->draw(viewport);

                Props::passOn = passOn;
				++it;
			}
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Camera>("Camera",
                sol::base_classes, sol::bases<Amara::Node>(),
                "w", &Camera::width,
                "h", &Camera::height,
                "scroll", &Camera::scroll,
                "scrollX", sol::property([](Camera& cam) { return cam.scroll.x; }, [](Camera& cam, float val) { cam.scroll.x = val; }),
                "scrollY", sol::property([](Camera& cam) { return cam.scroll.x; }, [](Camera& cam, float val) { cam.scroll.y = val; }),
                "zoom", &Camera::zoom,
                "zoomX", sol::property([](Camera& cam) { return cam.zoom.x; }, [](Camera& cam, float val) { cam.zoom.x = val; }),
                "zoomY", sol::property([](Camera& cam) { return cam.zoom.y; }, [](Camera& cam, float val) { cam.zoom.y = val; }),
                "lerp", &Camera::lerp,
                "lerpX", sol::property([](Camera& cam) { return cam.lerp.x; }, [](Camera& cam, float val) { cam.lerp.x = val; }),
                "lerpY", sol::property([](Camera& cam) { return cam.lerp.y; }, [](Camera& cam, float val) { cam.lerp.y = val; }),
                "center", sol::readonly(&Camera::center),
                "leftBorder", sol::readonly(&Camera::leftBorder),
                "rightBorder", sol::readonly(&Camera::rightBorder),
                "topBorder", sol::readonly(&Camera::topBorder),
                "bottomBorder", sol::readonly(&Camera::bottomBorder),
                "startFollow", sol::overload(
                    sol::resolve<sol::object(Amara::Node*)>(&Camera::startFollow),
                    sol::resolve<sol::object(Amara::Node*, float, float)>(&Camera::startFollow),
                    sol::resolve<sol::object(Amara::Node*, float)>(&Camera::startFollow)
                ),
                "stopFollow", &Camera::stopFollow
            );
        }
    };
}