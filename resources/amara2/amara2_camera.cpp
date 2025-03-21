namespace Amara {
    class Camera: public Entity {
    public:
        Rectangle viewport;

        Camera(): Entity() {
            set_base_entity_id("Camera");
            is_camera = true;
        }

        virtual void update_properties() override {
            Props::viewport = viewport;
        }

        virtual void drawChildren() override {
            children_copy_list = parent->children;

            Amara::Entity* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = *it;
				if (child == nullptr || child->isDestroyed || child->parent != parent) {
					++it;
					continue;
				}
                update_properties();
				if (!child->is_camera) child->draw();
				++it;
			}
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Scene>("Camera",
                sol::base_classes, sol::bases<Amara::Entity>()
            );
        }
    };
}