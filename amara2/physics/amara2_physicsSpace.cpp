namespace Amara {
    class PhysicsBody;

    class PhysicsSpace: public Amara::Node {
    public:
        cpSpace* space = nullptr;

        PhysicsSpace() : Amara::Node() {
            set_base_node_id("PhysicsSpace");
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            Amara::Node::configure(config);
            if (json_has(config, "gravity")) {
                if (config["gravity"].is_array() && config["gravity"].size() == 2) {
                    setGravity(config["gravity"][0].get<double>(), config["gravity"][1].get<double>());
                }
                else if (config["gravity"].is_number()) {
                    setGravity(config["gravity"].get<double>());
                }
            }
            return this;
        }

        virtual void create() override {
            if (space) {
                cpSpaceFree(space);
            }
            space = cpSpaceNew();
            if (!space) {
                debug_log("Error: Failed to create Chipmunk physics space.");
                return;
            }
        }

        void setGravity(double x, double y) {
            if (space) {
                cpVect gravity = cpv(x, y);
                cpSpaceSetGravity(space, gravity);
            }
            else {
                debug_log("Error: Physics space is not initialized.");
            }
        }
        void setGravity(double value) {
            setGravity(0, value);
        }

        void addBody(Amara::PhysicsBody* body);
        void addBody(cpBody* body) {
            if (space && body) {
                cpSpaceAddBody(space, body);
            }
            else {
                debug_log("Error: Physics space or body is not initialized.");
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
                )
            );
        }
    };
}