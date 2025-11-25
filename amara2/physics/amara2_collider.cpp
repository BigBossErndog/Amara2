namespace Amara {
    class Collider: public Amara::Action {
    public:
        Vector2 velocity = Vector2(0, 0);
        Vector2 acceleration = Vector2(0, 0);
        Vector2 damping = Vector2(0, 0);
        Vector2 bounciness = Vector2(0, 0);
        float slopeDamping = 0.5f;

        std::vector<Amara::Node*> collisionTargets;

        Shape shape;
        bool set_shape = false;
        
        double targetAccuracy = 0.1;

        int maxChecks = 64;
        int splitChecks = 8;
        int correctionChecks = 16;

        int collisionDirections = 0;

        static constexpr float dampingPower = 3.0f;

        Collider(): Amara::Action() {
            set_base_node_id("Collider");
        }

        virtual void create() override {
            Amara::Action::create();
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
            if (json_has(config, "damping")) damping = config["damping"];
            if (json_has(config, "slopeDamping")) slopeDamping = config["slopeDamping"];
            if (json_has(config, "bounciness")) bounciness = config["bounciness"];

            if (json_has(config, "maxChecks")) maxChecks = config["maxChecks"];
            if (json_has(config, "targetAccuracy")) targetAccuracy = config["targetAccuracy"];
            if (json_has(config, "correctionChecks")) correctionChecks = config["correctionChecks"];

            if (json_has(config, "shape")) {
                shape = config["shape"];
                set_shape = true;
            }

            return Amara::Action::configure(config);
        }

        virtual sol::object luaConfigure(sol::object config) {
            if (config.is<sol::table>()) {
                sol::table t = config.as<sol::table>();

                if (t["targets"].valid()) {
                    collisionTargets.clear();
                    sol::object tobj = t["targets"];
                    if (tobj.is<Amara::Node>()) {
                        addCollisionTarget(tobj.as<Amara::Node*>());
                    }
                    else if (tobj.is<sol::table>()) {
                        sol::table targets = tobj;
                        for (auto& pair: targets) {
                            sol::object obj = pair.second;
                            if (obj.is<Amara::Node>()) {
                                addCollisionTarget(obj.as<Amara::Node*>());
                            }
                        }
                    }
                }
                if (t["target"].valid()) {
                    collisionTargets.clear();
                    sol::object tobj = t["target"];
                    if (tobj.is<Amara::Node>()) {
                        addCollisionTarget(tobj.as<Amara::Node*>());
                    }
                    else if (tobj.is<sol::table>()) {
                        sol::table targets = tobj.as<sol::table>();
                        for (auto& pair: targets) {
                            sol::object obj = pair.second;
                            if (obj.is<Amara::Node>()) {
                                addCollisionTarget(obj.as<Amara::Node*>());
                            }
                        }
                    }
                }
            }

            return Amara::Action::luaConfigure(config);
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

        void removeCollisionTarget(Amara::Node* other) {
            for (auto it = collisionTargets.begin(); it != collisionTargets.end();) {
                Amara::Node* target = *it;

                if (other == target || other->collider == target) {
                    it = collisionTargets.erase(it);
                }
                else {
                    ++it;
                }
            }
        }

        bool hasCollided() {
            for (Amara::Node* target: collisionTargets) {
                if (target->destroyed || target->paused) continue;
                if (collidesWith(target)) return true;
            }
            return false;
        }

        bool hasCollided(Amara::Node* other) {
            if (other == nullptr || other->destroyed || other == this || other == actor) return false;
            return collidesWith(other);
        }

        bool hasCollided(Amara::Direction _dir) {
            return  (collisionDirections & (int)_dir) != 0;
        }

        virtual Shape getCollisionShape() override {
            if (set_shape) {
                return shape.move(actor->pos);
            }
            return actor->getCollisionShape();
        }

        bool moveActor(Vector2 v, double deltaTime) {
            Vector2 start_pos = parent->pos;
            Vector2 change = v * deltaTime;

            Vector2 last_pos = parent->pos;
            Vector2 fix_pos, rec_pos;



            bool collided = false;
            Line line = Line(start_pos, start_pos + change);
            for (int i = 0; i < splitChecks; i++) {
                fix_pos = line.getPoint((float)i / ((float)splitChecks - 1.0f));

                parent->pos = fix_pos;

                if (hasCollided()) {
                    collided = true;
                    break;
                }

                last_pos = fix_pos;
            }

            if (collided) {
                int checks = 0;
                rec_pos = last_pos;

                while (true) {
                    fix_pos = (fix_pos + last_pos) / 2.0;
                    last_pos = rec_pos;

                    parent->pos = fix_pos;

                    if (hasCollided()) {
                        rec_pos = last_pos;
                    }
                    else {
                        rec_pos = fix_pos;

                        if (distanceBetween(last_pos, fix_pos) < targetAccuracy) {
                            break;
                        }
                    }

                    parent->pos = rec_pos;

                    checks += 1;
                    if (checks > maxChecks) {
                        break;
                    }
                }

                if (hasCollided()) {
                    parent->pos = start_pos;
                }

                return true;
            }

            return false;
        }

        virtual void act(double deltaTime) override {
            Amara::Action::act(deltaTime);

            if (has_started) {
                collisionDirections = 0;

                velocity.x += acceleration.x * deltaTime;
                velocity.y += acceleration.y * deltaTime;

                Vector2 original_pos = parent->pos;

                bool wall_hit = false;

                if (moveActor(velocity * Vector2(1, 0), deltaTime)) {
                    if (velocity.x < 0) collisionDirections |= (int)Direction::Left;
                    else if (velocity.x > 0) collisionDirections |= (int)Direction::Right;
                    velocity.x = -velocity.x * bounciness.x;
                    if (bounciness.x != 0) velocity.x = -velocity.x * bounciness.x;
                    wall_hit = true;
                }
                if (moveActor(velocity * Vector2(0, 1), deltaTime)) {
                    if (velocity.y < 0) collisionDirections |= (int)Direction::Up;
                    else if (velocity.y > 0) collisionDirections |= (int)Direction::Down;
                    if (bounciness.y != 0) velocity.y = -velocity.y * bounciness.y;
                    wall_hit = true;
                }

                if (wall_hit) {
                    if (bounciness.x == 0 && bounciness.y == 0) {
                        Vector2 leftover_force = velocity - (parent->pos - original_pos);
                        Vector2 wall_vector = findWallVector(velocity);
                        float angle_between = abs(angleDifference(
                            angleBetween(Vector2::Origin, velocity),
                            angleBetween(Vector2::Origin, wall_vector)
                        ));
                        debug_log("sliding at angle ", angle_between, " ", leftover_force, " ", wall_vector);
                        if (angle_between < M_PI/2.0) {
                            Vector2 slide_vector = wall_vector.normalize() * leftover_force.magnitude();
                            debug_log("slide force ", slide_vector);
                            Vector2 damped_slide = slide_vector * (angle_between/(M_PI/2.0));
                            slide_vector = Vector2(
                                ease(slide_vector.x, damped_slide.x, slopeDamping),
                                ease(slide_vector.y, damped_slide.y, slopeDamping)
                            );
                            debug_log("actual_slide ", slide_vector);
                            moveActor(slide_vector, deltaTime);
                        }
                    }
                }

                float mappeddampingX = 1.0f - std::pow(1.0f - damping.x, dampingPower);
                float mappeddampingY = 1.0f - std::pow(1.0f - damping.y, dampingPower);
                velocity.x *= std::pow(1.0f - mappeddampingX, deltaTime);
                velocity.y *= std::pow(1.0f - mappeddampingY, deltaTime);
            }
            cleanCollisionTargets();
        }

        void cleanCollisionTargets() {
            for (auto it = collisionTargets.begin(); it != collisionTargets.end();) {
                if ((*it)->destroyed) {
                    it = collisionTargets.erase(it);
                }
                else {
                    ++it;
                }
            }
        }

        void selfCorrect() {
            int change = 0;
            float angle;
            Vector2 start_pos = parent->pos;
            while(hasCollided() && change < maxChecks) {
                for (int i = 0; i < correctionChecks; i++) {
                    angle = 2*M_PI * (float)i / (float)correctionChecks;

                    parent->pos = start_pos + Vector2(
                        sin(angle)*change,
                        cos(angle)*change
                    )*0.1;

                    if (!hasCollided()) return;
                }
                
                change += 1;
            }
            if (hasCollided()) parent->pos = start_pos;
        }

        Vector2 findWallVector(Vector2 direction) {
            Vector2 normal1 = rotateAroundAnchor(direction, -M_PI/2.0).normalized();
            Vector2 normal2 = rotateAroundAnchor(direction, M_PI/2.0).normalized();
            double rate_of_rotation = M_PI * 0.05;

            if (checkForWall(normal1)) {
                if (checkForWall(normal2)) {
                    return normal1;
                }
                while (checkForWall(normal1)) {
                    normal1 = rotateAroundAnchor(normal1, rate_of_rotation);
                    normal2 = rotateAroundAnchor(normal2, rate_of_rotation);
                    if (!checkForWall(normal1)) {
                        return normal1;
                    }
                    if (checkForWall(normal2)) {
                        return normal1;
                    }
                }
            }
            else if (checkForWall(normal2)) {
                while (checkForWall(normal2)) {
                    normal1 = rotateAroundAnchor(normal1, -rate_of_rotation);
                    normal2 = rotateAroundAnchor(normal2, -rate_of_rotation);
                    if (!checkForWall(normal2)) {
                        return normal2;
                    }
                    if (checkForWall(normal1)) {
                        return normal2;
                    }
                }
            }
            
            return normal1;
        }

        bool checkForWall(Vector2 movement) {
            Vector2 rec_pos = parent->pos;
            parent->pos += movement;
            if (hasCollided()) {
                parent->pos = rec_pos;
                return true;
            }
            parent->pos = rec_pos;
            return false;
        }

        virtual void destroy() override {
            if (actor && actor->collider == (Amara::Node*)this) actor->collider = nullptr;
            Amara::Action::destroy();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Collider>("Collider",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
                "velocity", sol::property(
                    [](Collider& t) -> Vector2& { return t.velocity; },
                    [](Collider& t, sol::object v) { t.velocity = v; }
                ),
                "velocityX", sol::property([](Collider& t) { return t.velocity.x; }, [](Collider& t, float val) { t.velocity.x = val; }),
                "velocityY", sol::property([](Collider& t) { return t.velocity.y; }, [](Collider& t, float val) { t.velocity.y = val; }),
                "acceleration", sol::property(
                    [](Collider& t) -> Vector2& { return t.acceleration; },
                    [](Collider& t, sol::object v) { t.acceleration = v; }
                ),
                "accelerationX", sol::property([](Collider& t) { return t.acceleration.x; }, [](Collider& t, float val) { t.acceleration.x = val; }),
                "accelerationY", sol::property([](Collider& t) { return t.acceleration.y; }, [](Collider& t, float val) { t.acceleration.y = val; }),
                "damping", sol::property(
                    [](Collider& t) -> Vector2& { return t.damping; },
                    [](Collider& t, sol::object v) { t.damping = v; }
                ),
                "dampingX", sol::property([](Collider& t) { return t.damping.x; }, [](Collider& t, float val) { t.damping.x = val; }),
                "dampingY", sol::property([](Collider& t) { return t.damping.y; }, [](Collider& t, float val) { t.damping.y = val; }),
                "slopeDamping", &Collider::slopeDamping,
                "bounciness", sol::property(
                    [](Collider& t) -> Vector2& { return t.bounciness; },
                    [](Collider& t, sol::object v) { t.bounciness = v; }
                ),
                "shape", sol::property(
                    [](Collider& t) -> sol::object { return t.shape.get_lua_object(t.gameProps->lua); },
                    [](Collider& t, sol::object v) { t.shape = v; t.set_shape = true; }
                ),
                "maxChecks", &Collider::maxChecks,
                "targetAccuracy", &Collider::targetAccuracy,
                "correctionChecks", &Collider::correctionChecks,
                "collisionDirections", sol::readonly(&Collider::collisionDirections),
                "hasCollided", sol::overload(
                    sol::resolve<bool()>(&Collider::hasCollided),
                    sol::resolve<bool(Amara::Node*)>(&Collider::hasCollided),
                    sol::resolve<bool(Amara::Direction)>(&Collider::hasCollided)
                ),
                "selfCorrect", &Collider::selfCorrect,
                "addCollisionTarget", &Collider::addCollisionTarget,
                "removeCollisionTarget", &Collider::removeCollisionTarget,
                "target", sol::property(
                    [](Collider& t) -> sol::object {
                        if (t.collisionTargets.size() > 0) return t.collisionTargets[0]->get_lua_object();
                        return sol::nil;
                    },
                    [](Collider& t, sol::object v) {
                        t.collisionTargets.clear();
                        if (v.is<Amara::Node>()) {
                            t.addCollisionTarget(v.as<Amara::Node*>());
                        }
                        else if (v.is<sol::table>()) {
                            sol::table targets = v.as<sol::table>();
                            for (auto& pair: targets) {
                                sol::object obj = pair.second;
                                if (obj.is<Amara::Node>()) {
                                    t.addCollisionTarget(obj.as<Amara::Node*>());
                                }
                            }
                        }
                    }
                ),
                "targets", sol::property(
                    [](Collider& t) -> sol::object {
                        sol::table targets = t.gameProps->lua.create_table();
                        for (Amara::Node* target: t.collisionTargets) {
                            targets.add(target->get_lua_object());
                        }
                        return targets;
                    },
                    [](Collider& t, sol::object v) {
                        t.collisionTargets.clear();
                        if (v.is<Amara::Node>()) {
                            t.addCollisionTarget(v.as<Amara::Node*>());
                        }
                        else if (v.is<sol::table>()) {
                            sol::table targets = v.as<sol::table>();
                            for (auto& pair: targets) {
                                sol::object obj = pair.second;
                                if (obj.is<Amara::Node>()) {
                                    t.addCollisionTarget(obj.as<Amara::Node*>());
                                }
                            }
                        }
                    }
                )
            );
        }
    };
}
