namespace Amara {
    class Container: public Amara::Node {
    public:
        float width = 128;
        float height = 128;

        float left = 0;
        float right = 0;
        float top = 0;
        float bottom = 0;

        Vector2 center = { 0, 0 };

        Vector2 origin = { 0.5, 0.5 };

        Container(): Amara::Node() {
            set_base_node_id("Container");
        }

        virtual void create() override {
            Amara::Node::create();
            update_size();
        }

        void update_size() {
            left = pos.x + -width/2.0;
            right = pos.x + width/2.0;
            top = pos.y + -height/2.0;
            bottom = pos.y + height/2.0;

            center = Vector2( left + width*origin.x, top + height*origin.y );
        }

        void setWidth(float _w) {
            width = _w;
            update_size();
        }
        void setHeight(float _h) {
            height = _h;
            update_size();
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "width")) setWidth(config["width"]);
            if (json_has(config, "height")) setHeight(config["height"]);

            if (json_has(config, "origin")) {
                origin = config["origin"];
            }

            return Amara::Node::configure(config);
        }

        virtual void drawChildren(const Rectangle& v) override {
            SDL_Rect old_sdl_viewport;
            if (gameProps->graphics == GraphicsEnum::Render2D && gameProps->renderer) {
                SDL_GetRenderViewport(gameProps->renderer, &old_sdl_viewport);
            }
            #ifdef AMARA_OPENGL
            GLint old_gl_viewport[4];
            if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                glGetIntegerv(GL_VIEWPORT, old_gl_viewport);
            }
            #endif

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

            float world_top_left_x = anchoredPos.x + (-width*origin.x)*scale.x*passOn.scale.x;
            float world_top_left_y = anchoredPos.y - anchoredPos.z + (-height*origin.y)*scale.y*passOn.scale.y;

            SDL_FRect new_viewport_f;
            new_viewport_f.x = (gameProps->master_viewport.w / 2.0f) + world_top_left_x * totalZoom.x;
            new_viewport_f.y = (gameProps->master_viewport.h / 2.0f) + world_top_left_y * totalZoom.y;
            new_viewport_f.w = width*scale.x*passOn.scale.x * totalZoom.x;
            new_viewport_f.h = height*scale.y*passOn.scale.y * totalZoom.y;

            SDL_Rect new_sdl_viewport = Rectangle::makeSDLRect(Rectangle(new_viewport_f));

            if (gameProps->graphics == GraphicsEnum::Render2D && gameProps->renderer) {
                SDL_SetRenderViewport(gameProps->renderer, &new_sdl_viewport);
            }
            #ifdef AMARA_OPENGL
            else if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) { // OpenGL uses bottom-left origin
                gameProps->renderBatch->flush();
                glViewport(new_sdl_viewport.x, gameProps->master_viewport.h - (new_sdl_viewport.y + new_sdl_viewport.h), new_sdl_viewport.w, new_sdl_viewport.h);
            }
            #endif
            
            children_copy_list = children;
            
            pass_on_properties();
            
            PassOnProps rec_props = gameProps->passOn;
            PassOnProps new_props = rec_props;

            new_props.anchor = Vector3(0, 0, 0);
            new_props.scroll = Vector2(0, 0);

            gameProps->passOn = new_props;

            Amara::Node* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = *it;
				if (child == nullptr || child->destroyed || !child->visible || child->parent != this) {
					++it;
					continue;
				}
                
                update_properties();
				child->draw({ 0, 0, static_cast<float>(new_sdl_viewport.w), static_cast<float>(new_sdl_viewport.h) });
                
                gameProps->passOn = passOn;
				++it;
			}

            gameProps->passOn = rec_props;

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
            lua.new_usertype<Container>("Container",
                sol::base_classes, sol::bases<Amara::Node>(),
                "width", sol::property([](Amara::Container& c) -> float { return c.width; }, [](Amara::Container& c, float v) { c.setWidth(v); }),
                "height", sol::property([](Amara::Container& c) -> float { return c.height; }, [](Amara::Container& c, float v) { c.setHeight(v); }),
                "left", sol::readonly(&Container::left),
                "right", sol::readonly(&Container::right),
                "top", sol::readonly(&Container::top),
                "bottom", sol::readonly(&Container::bottom),
                "center", sol::readonly(&Container::center),
                "origin", &Container::origin
            );
        }
    };
}