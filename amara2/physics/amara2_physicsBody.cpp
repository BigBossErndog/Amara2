namespace Amara {
    class PhysicsBody: public Amara::Action {
    public:
        cpBody* body = nullptr;
        cpShape* body_shape = nullptr;

        Amara::PhysicsSpace* spaceNode = nullptr;

        Vector2 velocity = Vector2(0, 0);

        Vector2 rec_position = Vector2(0, 0);
        Vector2 rec_velocity = Vector2(0, 0);

        Amara::Shape shape;
        bool shape_set = false;

        double friction = 0;
        double elasticity = 0;

        PhysicsBody() : Amara::Action() {
            set_base_node_id("PhysicsBody");
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "velocity")) {
                velocity = config["velocity"];
            }
            if (json_has(config, "shape")) {
                nlohmann::json shapeConfig = config["shape"];
                if (!shapeConfig.is_null()) setShape(shapeConfig);
            }
            if (json_has(config, "friction")) {
                setFriction(config["friction"]);
            }
            if (json_has(config, "elasticity")) {
                setElasticity(config["elasticity"]);
            }
            return Amara::Action::configure(config);
        }

        virtual sol::object luaConfigure(std::string key, sol::object val) override {
            if (String::equal(key, "space")) {
                if (val.is<Amara::PhysicsSpace>()) {
                    spaceNode = val.as<Amara::PhysicsSpace*>();
                    if (spaceNode && spaceNode->space) {
                        if (body) cpSpaceAddBody(spaceNode->space, body);
                    }
                }
            }
            return Amara::Action::luaConfigure(key, val);
        }

        void setShape(sol::object shape_obj) {
            if (shape_obj.is<Rectangle>()) {
                shape = shape_obj.as<Rectangle>();
                processShape();
            }
            else if (shape_obj.is<Circle>()) {
                shape = shape_obj.as<Circle>();
                processShape();
            }
            else if (shape_obj.is<Vector2>()) {
                shape = shape_obj.as<Vector2>();
                processShape();
            }
            else if (shape_obj.is<sol::table>()) {
                nlohmann::json shapeConfig = lua_to_json(shape_obj);
                setShape(shapeConfig);
            }
        }

        void setShape(nlohmann::json shapeConfig) {
            if (shapeConfig.is_object()) {
                if (json_has(shapeConfig, "x", "y")) {
                    if (json_has(shapeConfig, "w", "h") || json_has(shapeConfig, "width", "height")) {
                        // Rectangle
                        Rectangle rect = shapeConfig;
                        shape = rect;
                        processShape();
                    }
                    else if (json_has(shapeConfig, "r") || json_has(shapeConfig, "radius")) {
                        // Circle
                        Circle circle = shapeConfig;
                        shape = circle;
                        processShape();
                    }
                }
                else if (json_has(shapeConfig, "p1", "p2", "p3")) {
                    if (json_has(shapeConfig, "p4")) {
                        // Quad
                    }
                    else {
                        // Triangle
                    }
                }
            }
            else if (shapeConfig.is_array()) {
                if (shapeConfig.size() == 4) {
                    if (shapeConfig[0].is_number() && shapeConfig[1].is_number() &&
                        shapeConfig[2].is_number() && shapeConfig[3].is_number()) {
                        // Rectangle
                        Rectangle rect = shapeConfig;
                        shape = rect;
                        processShape();
                    }
                    else {
                        // Quad
                    }
                }
                else if (shapeConfig.size() == 3) {
                    if (shapeConfig[0].is_number() && shapeConfig[1].is_number() &&
                        shapeConfig[2].is_number()) {
                        // Circle
                        Circle circle = shapeConfig;
                        shape = circle;
                        processShape();
                    }
                    else {
                        // Triangle
                    }
                }
                else if (shapeConfig.size() == 2) {
                    if (shapeConfig[0].is_number() && shapeConfig[1].is_number()) {
                        // Line
                    }
                }
            }
        }

        void processShape() {
            if (!body) return;

            if (body_shape && spaceNode && spaceNode->space) {
                cpSpaceRemoveShape(spaceNode->space, body_shape);
                cpShapeFree(body_shape);
                body_shape = nullptr;
            }

            if (shape.is<Rectangle>()) {
                Rectangle rect = shape.as<Rectangle>();
                body_shape = cpBoxShapeNew2(body, cpBBNew(rect.x, rect.y, rect.w, rect.h), 0.0);
            }
            else if (shape.is<Circle>()) {
                Circle circle = shape.as<Circle>();
                body_shape = cpCircleShapeNew(body, circle.radius, cpv(0, 0));
            }
            else {
                debug_log("Error: Unsupported shape type for PhysicsBody.");
            }

            if (body_shape) {
                cpSpaceAddShape(spaceNode->space, body_shape);

                cpShapeSetFriction(body_shape, friction);
                cpShapeSetElasticity(body_shape, elasticity);
            }
        }

        void setFriction(double frictionValue) {
            friction = frictionValue;
            if (body_shape) {
                cpShapeSetFriction(body_shape, friction);
            }
        }
        void setElasticity(double elasticityValue) {
            elasticity = elasticityValue;
            if (body_shape) {
                cpShapeSetElasticity(body_shape, elasticity);
            }
        }

        virtual void prepare() override {
            Amara::Action::prepare();
            
            body = cpBodyNew(1.0, 1.0);
            if (!body) {
                debug_log("Error: Failed to instantiate PhysicsBody.");
                gameProps->breakWorld();
                return;
            }
            if (actor) {
                cpVect initialPos = cpv(actor->pos.x, actor->pos.y);
                cpBodySetPosition(body, initialPos);
                rec_position.x = actor->pos.x;
                rec_position.y = actor->pos.y;
            }
            cpBodySetVelocity(body, cpv(velocity.x, velocity.y));
            rec_velocity = velocity;

            if (spaceNode && spaceNode->space) {
                cpSpaceAddBody(spaceNode->space, body);
            }
     

            processShape();
        }
        
        virtual void act(double deltaTime) override {
            Amara::Action::act(deltaTime);

            if (hasStarted) {
                if (spaceNode && spaceNode->destroyed) {
                    destroy();
                    return;
                }

                if (body && actor) {
                    // Sync from actor to body if actor was moved externally
                    if (actor->pos.x != rec_position.x || actor->pos.y != rec_position.y) {
                        cpBodySetPosition(body, cpv(actor->pos.x, actor->pos.y));
                    }

                    // Sync velocity if changed externally
                    if (velocity.x != rec_velocity.x || velocity.y != rec_velocity.y) {
                        cpBodySetVelocity(body, cpv(velocity.x, velocity.y));
                    }

                    // Sync from body to actor
                    cpVect pos = cpBodyGetPosition(body);
                    actor->pos.x = pos.x;
                    actor->pos.y = pos.y;

                    cpVect vel = cpBodyGetVelocity(body);
                    velocity.x = vel.x;
                    velocity.y = vel.y;

                    // Update recorded values
                    rec_position.x = actor->pos.x;
                    rec_position.y = actor->pos.y;
                    rec_velocity.x = velocity.x;
                    rec_velocity.y = velocity.y;
                }
            }
        }

        virtual void destroy() override {
            if (body) {
                if (spaceNode && spaceNode->space) {
                    cpSpaceRemoveBody(spaceNode->space, body);
                }
                cpBodyFree(body);
                body = nullptr;
            }
            Amara::Node::destroy();
        }

        sol::object getPhysicsShape() {
            if (shape.is<Rectangle>()) {
                return sol::make_object(gameProps->lua, shape.as<Rectangle>());
            }
            else if (shape.is<Circle>()) {
                return sol::make_object(gameProps->lua, shape.as<Circle>());
            }
            else if (shape.is<Vector2>()) {
                return sol::make_object(gameProps->lua, shape.as<Vector2>());
            }
            else {
                sol::nil;
            }
        }

        void addToSpace(sol::object _obj) {
            if (_obj.is<Amara::PhysicsSpace>()) {
                Amara::PhysicsSpace* space = _obj.as<Amara::PhysicsSpace*>();
                if (space && space->space) {
                    if (body) cpSpaceAddBody(space->space, body);
                }
            }
            else {
                debug_log("Error: Expected a PhysicsSpace object.");
            }
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::PhysicsBody>("PhysicsBody",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
                "velocity", &Amara::PhysicsBody::velocity,
                "shape", sol::property(&Amara::PhysicsBody::getShape, sol::resolve<void(sol::object)>(&Amara::PhysicsBody::setShape)),
                "setFriction", &Amara::PhysicsBody::setFriction,
                "setElasticity", &Amara::PhysicsBody::setElasticity,
                "friction", sol::property(&Amara::PhysicsBody::friction, &Amara::PhysicsBody::setFriction),
                "elasticity", sol::property(&Amara::PhysicsBody::elasticity, &Amara::PhysicsBody::setElasticity),
                "addToSpace", &Amara::PhysicsBody::addToSpace
            );
        }
    };

    void Amara::PhysicsSpace::addBody(sol::object body_obj) {
        if (body_obj.is<Amara::PhysicsBody>()) {
            Amara::PhysicsBody* bodyNode = body_obj.as<Amara::PhysicsBody*>();
            if (space && bodyNode && bodyNode->body) {
                cpSpaceAddBody(space, bodyNode->body);
                bodyNode->spaceNode = this;
            }
            else {
                debug_log("Error: Physics space or body is not initialized.");
            }
        }
        else if (body_obj.is<Amara::Node>()) {
            auto& node_children = body_obj.as<Amara::Node*>()->children; 
            for (auto& child : node_children) {
                if (child->is<Amara::PhysicsBody>()) {
                    cpSpaceAddBody(space, child->as<Amara::PhysicsBody*>()->body);
                    child->as<Amara::PhysicsBody*>()->spaceNode = this;
                }
            }
        }
    }
}