namespace Amara {
    class PhysicsBody;

    class PhysicsSpace: public Amara::Node {
    public:
        cpSpace* space = nullptr;

        Vector2 gravity = Vector2(0, 0);

        double damping = 1;

        PhysicsSpace() : Amara::Node() {
            set_base_node_id("PhysicsSpace");
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            Amara::Node::configure(config);
            if (json_has(config, "gravity")) {
                if (config["gravity"].is_array() && config["gravity"].size() == 2) {
                    setGravity(config["gravity"][0], config["gravity"][1]);
                }
                else if (config["gravity"].is_number()) {
                    setGravity(config["gravity"]);
                }
                else if (config["gravity"].is_object()) {
                    gravity = config["gravity"];
                    setGravity(gravity.x, gravity.y);
                }
            }
            if (json_has(config, "damping")) {
                setDamping(config["damping"]);
            }
            return this;
        }

        virtual void create() override {
            if (space) {
                cpSpaceFree(space);
            }
            space = cpSpaceNew();
            if (!space) {
                fatal_error("Error: Failed to initiate PhysicsSpace.");
                return;
            }
            else {
                setGravity(gravity.x, gravity.y);
                setDamping(damping);
            }
        }

        void setGravity(double x, double y) {
            gravity = Vector2(x, y);

            if (space) {
                cpVect gravity = cpv(x, y);
                cpSpaceSetGravity(space, gravity);
            }
        }
        void setGravity(double value) {
            setGravity(0, value);
        }

        void setDamping(double value) {
            damping = value;
            if (space) {
                cpSpaceSetDamping(space, value);
            }
        }

        void addBody(sol::object);
        void addBody(cpBody* body) {
            if (space && body) {
                cpSpaceAddBody(space, body);
            }
            else {
                fatal_error("Error: Physics space or body is not initialized.");
            }
        }

        virtual void update(double deltaTime) override {
            Amara::Node::update(deltaTime);
            if (space) {
                cpSpaceStep(space, deltaTime);
            }
        }

        virtual void destroy() override {
            if (space) {
                cpSpaceFree(space);
                space = nullptr;
            }
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::PhysicsSpace>("PhysicsSpace",
                sol::base_classes, sol::bases<Amara::Node>(),
                "setGravity", sol::overload(
                    static_cast<void (Amara::PhysicsSpace::*)(double, double)>(&Amara::PhysicsSpace::setGravity),
                    static_cast<void (Amara::PhysicsSpace::*)(double)>(&Amara::PhysicsSpace::setGravity)
                ),
                "gravity", sol::property(&Amara::PhysicsSpace::gravity, sol::resolve<void(double)>(&Amara::PhysicsSpace::setGravity)),
                "setDamping", &Amara::PhysicsSpace::setDamping,
                "damping", sol::property(&Amara::PhysicsSpace::damping, &Amara::PhysicsSpace::setDamping)
            );
        }
    };
}