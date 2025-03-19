namespace Amara {
    class Scene: public Amara::Entity {
    public:
        Amara::Camera* mainCamera = nullptr;
        std::vector<Amara::Entity*> cameras;

        Scene() {
            set_base_entity_id("Scene");
            scene = this;
            is_scene = true;
        }

        virtual void update_properties() override {
            Properties::scene = this;
            Amara::Entity::update_properties();
        }

        virtual void create() override {
            setMainCamera(createChild("Camera")->as<Amara::Camera*>());
        }

        virtual void run(double deltaTime) {
            Amara::Entity::run(deltaTime);
            if (
                mainCamera != nullptr &&
                mainCamera->isDestroyed
            ) mainCamera = nullptr;
            clean_entity_list(cameras);
        }

        virtual void drawChildren() override {
            children_copy_list = cameras;

            Amara::Entity* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = *it;
				if (child == nullptr || child->isDestroyed || child->parent != this) {
					++it;
					continue;
				}
                update_properties();
				child->draw();
				++it;
			}
        }

        Amara::Camera* setMainCamera(Amara::Camera* cam, bool destroyExisting) {
            if (destroyExisting && mainCamera != nullptr) mainCamera->destroy();
            mainCamera = cam;
            return cam;
        }
        Amara::Camera* setMainCamera(Amara::Camera* cam) {
            return setMainCamera(cam, true);
        }

        Amara::Entity* addChild(Amara::Entity* entity) {
            Amara::Camera* cam = entity->as<Amara::Camera*>();
            if (cam) {
                cameras.push_back(cam);
            }
            return Amara::Entity::addChild(entity);
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