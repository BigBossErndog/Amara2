namespace Amara {
    class Scene: public Amara::Node {
    public:
        Amara::Camera* camera = nullptr;
        std::vector<Amara::Node*> cameras;

        Scene(): Node() {
            set_base_node_id("Scene");
            scene = this;
        }

        virtual void update_properties() override {
            gameProps->scene = this;
            Amara::Node::update_properties();
        }

        virtual void create() override {
            setMainCamera(createChild("Camera")->as<Amara::Camera*>());
            Amara::Node::create();
        }

        virtual void run(double deltaTime) override {
            Amara::Node::run(deltaTime);
            if (
                camera != nullptr &&
                camera->destroyed
            ) camera = nullptr;
            clean_node_list(cameras);
        }

        virtual void drawChildren(const Rectangle& v) override {
            children_copy_list = cameras;

            pass_on_properties();

            Amara::Node* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = *it;
				if (child == nullptr || child->destroyed || child->parent != this) {
					++it;
					continue;
				}
                update_properties();
				child->draw(v);

                gameProps->passOn = passOn;
				++it;
			}
        }

        Amara::Camera* setMainCamera(Amara::Camera* cam, bool destroyExisting) {
            if (destroyExisting && camera != nullptr && camera != cam) camera->destroy();
            camera = cam;
            return cam;
        }
        Amara::Camera* setMainCamera(Amara::Camera* cam) {
            return setMainCamera(cam, true);
        }

        Amara::Node* addChild(Amara::Node* node) override {
            Amara::Camera* cam = node->as<Amara::Camera*>();
            if (cam) {
                cameras.push_back(cam);
            }
            return Amara::Node::addChild(node);
        }

        sol::object getCamera() {
            if (camera) return camera->get_lua_object();
            else return sol::nil;
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Scene>("Scene",
                sol::base_classes, sol::bases<Amara::Node>(),
                "camera", sol::property(&Scene::getCamera, sol::resolve<Amara::Camera*(Amara::Camera*)>(&Scene::setMainCamera)),
                "setMainCamera", sol::overload(
                    sol::resolve<Amara::Camera*(Amara::Camera*, bool)>(&Scene::setMainCamera),
                    sol::resolve<Amara::Camera*(Amara::Camera*)>(&Scene::setMainCamera)
                )
            );
            
            sol::usertype<Node> node_type = lua["Node"];
            node_type["scene"] = sol::readonly(&Node::scene);
        }
    };
}