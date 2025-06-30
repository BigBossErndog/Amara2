namespace Amara {
    class PhysicsBody: public Amara::Action {
    public:
        cpBody* body = nullptr;

        Amara::PhysicsSpace* spaceNode = nullptr;

        Vector2 velocity = Vector2(0, 0);

        Vector2 rec_position = Vector2(0, 0);

        PhysicsBody() : Amara::Action() {
            set_base_node_id("PhysicsBody");
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "velocity")) {
                velocity = config["velocity"];
            }
            return Amara::Action::configure(config);
        }

        virtual sol::object luaConfigure(std::string key, sol::object val) override {
            if (String::equal(key, "space")) {
                if (val.is<Amara::PhysicsSpace>()) {
                    spaceNode = val.as<Amara::PhysicsSpace*>();
                    if (spaceNode && spaceNode->space) {
                        spaceNode->addBody(this);
                    }
                }
            }
            return Amara::Action::luaConfigure(key, val);
        }

        virtual void prepare() override {
            Amara::Action::prepare();
            if (body) {
                cpBodyFree(body);
            }
            body = cpBodyNew(1.0, 1.0);
            if (!body) {
                debug_log("Error: Failed to create Chipmunk physics body.");
                return;
            }
        }

        virtual void act(double deltaTime) override {
            Amara::Action::act(deltaTime);

            if (hasStarted) {
                if (spaceNode && spaceNode->destroyed) {
                    destroy();
                }
            }
        }

        virtual void destroy() override {
            if (body) {
                cpBodyFree(body);
                body = nullptr;
            }
            Amara::Node::destroy();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::PhysicsBody>("PhysicsBody",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>()
            );
        }
    };

    void Amara::PhysicsSpace::addBody(Amara::PhysicsBody* bodyNode) {
        if (space && bodyNode && bodyNode->body) {
            cpSpaceAddBody(space, bodyNode->body);
            bodyNode->spaceNode = this;
        }
        else {
            debug_log("Error: Physics space or body is not initialized.");
        }
    }
}