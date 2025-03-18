namespace Amara {
    class Scene: public Amara::Entity {
    public:
        Amara::Camera* mainCamera = nullptr;

        Scene() {
            baseEntityID = "Scene";
            scene = this;
            is_scene = true;
        }

        void update_properties() {
            Properties::scene = this;
            Properties::lua()["scene"] = get_lua_object();
            Amara::Entity::update_properties();
        }

        virtual void create() override {
            // setMainCamera(createChild("Camera")->as<Amara::Camera*>());
        }

        virtual void run(double deltaTime) {
            Amara::Entity::run(deltaTime);
            if (
                mainCamera != nullptr &&
                mainCamera->isDestroyed
            ) mainCamera = nullptr;
        }

        virtual void drawChildren() override {
            children_copy_list = children;

            Amara::Entity* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = *it;
				if (child == nullptr || child->isDestroyed || child->parent != this) {
					++it;
					continue;
				}
                update_properties();
				if (child->is_camera) child->draw();
				++it;
			}
        }

        Amara::Camera* setMainCamera(Amara::Camera* cam, bool destroyExisting) {
            if (destroyExisting) mainCamera->destroy();
            mainCamera = cam;
            return cam;
        }
        Amara::Camera* setMainCamera(Amara::Camera* cam) {
            return setMainCamera(cam, true);
        }
        
        static void bindLua(sol::state& lua) {
            lua.new_usertype<Scene>("Scene",
                sol::base_classes, sol::bases<Entity>(),
                "setMainCamera", sol::overload(
                    sol::resolve<Amara::Camera*(Amara::Camera*, bool)>(&Scene::setMainCamera),
                    sol::resolve<Amara::Camera*(Amara::Camera*)>(&Scene::setMainCamera)
                )
            );

            sol::usertype<Entity> entity_type = lua["Entity"];
            entity_type["scene"] = sol::readonly(&Entity::scene);
        }
    };
}