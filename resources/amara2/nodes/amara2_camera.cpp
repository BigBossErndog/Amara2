namespace Amara {
    class Camera: public Node {
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

        Camera(): Node() {
            set_base_node_id("Camera");
            is_camera = true;
        }

        virtual void run(double deltaTime) override {
            Amara::Node::run(deltaTime);

            if (followTarget) {
                focusOn(followTarget);
            }

            center = { -scroll.x, -scroll.y };
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
            return focusOn(node->x, node->y);
        }

        sol::object startFollow(Amara::Node* node) {
            followTarget = node;
            focusOn(node);
            return get_lua_object();
        }

        sol::object stopFollow() {
            followTarget = nullptr;
            return get_lua_object();
        }

        virtual void pass_on_properties() override {
            props = Props::passOn;
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
                }

                Props::passOn = passOn;
            }
        }
        
        virtual void drawChildren(const Rectangle& v) override {
            children_copy_list = parent->children;

            if (isSizeTethered) {
                viewport = v;
            }
            else {
                viewport.x = {
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
                "scrollX", sol::property([](Camera& cam) { return cam.scrollX; }, [](Camera& cam, float val) { cam.scroll.x = val; }),
                "scrollY", sol::property([](Camera& cam) { return cam.scrollY; }, [](Camera& cam, float val) { cam.scroll.y = val; }),
                "zoom", &Camera::zoom,
                "zoomX", sol::property([](Camera& cam) { return cam.zoom.x }, [](Camera& cam, float val) { cam.zoom.x = val; }),
                "zoomY", sol::property([](Camera& cam) { return cam.zoom.y }, [](Camera& cam, float val) { cam.zoom.y = val; }),
                "center", sol::readonly(&Camera::center),
                "leftBorder", sol::readonly(&Camera::leftBorder),
                "rightBorder", sol::readonly(&Camera::rightBorder),
                "topBorder", sol::readonly(&Camera::topBorder),
                "bottomBorder", sol::readonly(&Camera::bottomBorder)
            );
        }
    };
}