namespace Amara {
    class Collider: public Amara::Action {
    public:
        Vector2 velocity = Vector2(0, 0);
        Vector2 acceleration = Vector2(0, 0);
        Vector2 friction = Vector2(0, 0);

        std::vector<Amara::Node*> collisionTargets;

        Shape shape;
        bool set_shape = false;

        Collider(): Amara::Action() {
            set_base_node_id("Collider");
        }

        virtual void prepare() override {
            Amara::Action::prepare();
            if (actor) {
                if (actor->collider && actor->collider != this) {
                    actor->collider->destroy();
                }
                actor->collider = this;
            }
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "velocity")) velocity = config["velocity"];
            if (json_has(config, "acceleration")) acceleration = config["acceleration"];
            if (json_has(config, "friction")) friction = config["friction"];

            return Amara::Action::configure(config);
        }

        void addCollisionTarget(Amara::Node* other) {
            if (other == nullptr || other->destroyed || other == this || other == actor) return;
            if (other->collider && !other->collider->destroyed) {
                collisionTargets.push_back(other->collider);
            }
            else {
                collisionTargets.push_back(other);
            }
        }

        virtual void act(double deltaTime) override {
            Amara::Action::act(deltaTime);

            if (has_started) {
                Shape check_shape = getCollisionShape();
            }
        }

        virtual Shape getCollisionShape() override {
            if (set_shape) return shape.move(actor->pos);
            return actor->getCollisionShape();
        }
        
        virtual void act(double deltaTime) override {
            Amara::Action::act(delaTime);

            if (has_started) {
                velocity.x += acceleration.x * deltaTime;
                velocity.y += acceleration.y * deltaTime;

                parent->pos.x += velocity.x * deltaTime;
                parent->pos.y += velocity.y * deltaTime;

                velocity.x *= (1 - friction.x);
                velocity.y *= (1 - friction.y);
            }
        }

        virtual void destroy() override {
            if (actor && actor->collider == (Amara::Node*)this) actor->collider = nullptr;
            Amara::Action::destroy();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<SimpleCollider>("SimpleCollider",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
                "velocity", sol::property(
                    [](SimpleCollider& t) -> Vector2 { return t.velocity; },
                    [](SimpleCollider& t, Vector2 v) { t.velocity = v; }
                ),
                "acceleration", sol::property(
                    [](SimpleCollider& t) -> Vector2 { return t.acceleration; },
                    [](SimpleCollider& t, Vector2 v) { t.acceleration = v; }
                ),
                "friction", sol::property(
                    [](SimpleCollider& t) -> Vector2 { return t.friction; },
                    [](SimpleCollider& t, Vector2 v) { t.friction = v; }
                )
            );
        }
    };
}