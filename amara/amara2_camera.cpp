namespace Amara {
    class Camera: public Entity {
    public:
        Camera(): Entity() {
            baseEntityID = "Camera";
            is_camera = true;
        }

        virtual void drawChildren() override {
            children_copy_list = parent->children;

            Amara::Entity* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
				update_properties();

                child = *it;
				if (child == nullptr || child->isDestroyed || child->parent != parent) {
					++it;
					continue;
				}
				if (!child->is_camera) child->draw();
				++it;
			}
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Scene>("Camera",
                sol::constructors<Camera()>(),
                sol::base_classes, sol::bases<Amara::Entity>()
            );
        }
    };
}