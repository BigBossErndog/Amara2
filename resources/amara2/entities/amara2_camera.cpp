namespace Amara {
    class Camera: public Entity {
    public:
        Rectangle viewport;
        bool isSizeTethered = true;

        float width = 0;
        float height = 0;

        Vector2 scroll;
        Vector2 zoom;

        Camera(): Entity() {
            set_base_entity_id("Camera");
            is_camera = true;
        }
        
        virtual void drawChildren(const Rectangle& v) override {
            children_copy_list = parent->children;

            if (isSizeTethered) {
                viewport = v;
            }
            else {

            }

            Amara::Entity* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = *it;
				if (child == nullptr || child->isDestroyed || child->parent != parent) {
					++it;
					continue;
				}
                update_properties();
				if (!child->is_camera) child->draw(viewport);
				++it;
			}
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Camera>("Camera",
                sol::base_classes, sol::bases<Amara::Entity>(),
                "w", &Camera::width,
                "h", &Camera::height
            );
        }
    };
}