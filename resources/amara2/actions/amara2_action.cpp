namespace Amara {
    class Action: public Node {
    public:
        Amara::Node* actor = nullptr;

        sol::protected_function onPrepare;
        sol::protected_function onAct;
        sol::protected_function onComplete;

        bool completed = false;
        bool hasStarted = false;
        bool waitingForChildren = false;

        bool autoDestroy = false;
        
        bool locked = false;

        Action(): Node() {
            set_base_node_id("Action");
            depthSortSelfEnabled = false;
            is_action = true;
        }
        
        virtual void create() override {
            if (actor == nullptr) actor = parent;
            Action::Node::create();
        }

        virtual void prepare() {
            hasStarted = true;

            if (onPrepare.valid()) {
                try {
                    sol::protected_function_result result = onPrepare(actor->get_lua_object(), get_lua_object());
                    if (!result.valid()) {
                        sol::error err = result;
                        throw std::runtime_error("Lua Error: " + std::string(err.what()));  
                    }
                } catch (const std::exception& e) {
                    debug_log(e.what());
                    gameProps->breakWorld();
                }
            }
        }

        virtual void act(double deltaTime) {
            if (!hasStarted) {
                prepare();
            }
            if (hasStarted && !completed && onAct.valid()) {
                try {
                    sol::protected_function_result result = onAct(actor->get_lua_object(), get_lua_object(), deltaTime);
                    if (!result.valid()) {
                        sol::error err = result;
                        throw std::runtime_error("Lua Error: " + std::string(err.what()));
                    }
                } catch (const std::exception& e) {
                    debug_log(e.what());
                    gameProps->breakWorld();
                }
            }
        }

        virtual void update(double deltaTime) override {
            if (!destroyed && !locked && !completed) act(deltaTime);
        }

        virtual void run(double deltaTime) override {
            Amara::Node::run(deltaTime);
            if (completed && !destroyed && !has_running_child_actions()) {
                waitingForChildren = false;
                // Destroy self when done and children are done.
                destroy();
            }
        }

        Amara::Node* addChild(Amara::Node* node) override {
            Amara::Action* action = node->as<Amara::Action*>();
            if (action) {
                if (actor == nullptr) actor = parent;
                action->actor = actor;
                if (!completed) action->locked = true;
            }
            return Amara::Node::addChild(node);
        }

        sol::object on(Amara::Node* node) {
            actor = node;
            return get_lua_object();
        }

        virtual sol::object complete() {
            if (completed) return get_lua_object();
            completed = true;
            if (onComplete.valid()) {
                try {
                    sol::protected_function_result result = onComplete(actor->get_lua_object(), get_lua_object());
                    if (!result.valid()) {
                        sol::error err = result;
                        throw std::runtime_error("Lua Error: " + std::string(err.what()));
                    }
                } catch (const std::exception& e) {
                    debug_log(e.what());
                    gameProps->breakWorld();
                }
            }

            start_child_actions();
            waitingForChildren = true;

            return get_lua_object();
        }

        sol::object whenDone(sol::function func) {
            onComplete = func;
            return get_lua_object();
        }

        virtual sol::object luaConfigure(std::string key, sol::object val) override {
            if (val.is<sol::function>()) {
                sol::function func = val.as<sol::function>();
                if (String::equal("onPrepare", key)) onPrepare = func;
                else if (String::equal("onAct", key)) onAct = func;
                else if (String::equal("onComplete", key)) onComplete = func;
            }
            return Amara::Node::luaConfigure(key, val);
        }

        void start_child_actions() {
            children_copy_list = children;

            Amara::Node* node;
            Amara::Action* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                node = *it; 
                child = dynamic_cast<Amara::Action*>(node);

                if (node != nullptr && child == nullptr) node->destroy();
				
                if (child == nullptr || child->destroyed || child->parent != this) {
					++it;
					continue;
				}
                
                update_properties();
				child->locked = false;
                child->prepare();
				++it;
			}
        }

        bool has_running_child_actions() {
            children_copy_list = children;

            Amara::Action* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = dynamic_cast<Amara::Action*>(*it);
				if (child == nullptr || child->destroyed || child->parent != this) {
					++it;
					continue;
				}
				if (!child->completed || child->waitingForChildren) return true;
				++it;
			}
            return false;
        }

        sol::object chain(std::string key) {
            Amara::Action* action = nullptr;
            Amara::Node* child = nullptr;

            for (int i = children.size()-1; i >= 0; i++) {
                child = children[i];
                if (!child->destroyed && child->is_action) {
                    action = child->as<Amara::Action*>();
                    if (action) {
                        return action->chain(key);
                    }
                }
            }

            action = createChild(key)->as<Amara::Action*>();
            return action->get_lua_object();
        }

        bool chainCompleted() {
            if (!completed) return false;

            Amara::Action* action = nullptr;
            for (Amara::Node* child: children) {
                if (!child->is_action) continue;
                action = child->as<Amara::Action*>();
                if (action == nullptr) continue;
                if (!action->chainCompleted()) return false;
            }
            return true;
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::Action>("Action",
                sol::base_classes, sol::bases<Amara::Node>(),
                "onPrepare", &Action::onPrepare,
                "onAct", &Action::onAct,
                "onComplete", &Action::onComplete,
                "hasStarted", sol::readonly(&Action::hasStarted),
                "completed", sol::readonly(&Action::completed),
                "complete", &Action::complete,
                "addChild", &Action::addChild,
                "chain", &Action::chain,
                "whenDone", &Action::whenDone,
                "autoDestroy", &Action::autoDestroy,
                "on", &Action::on,
                "alongside", [](Amara::Node& e, std::string key) -> sol::object {
                    Amara::Action* action = nullptr;
                    if (e.parent) action = e.parent->createChild(key)->as<Amara::Action*>();
                    else action = e.createChild(key)->as<Amara::Action*>();
                    return action->get_lua_object();
                }
            );

            sol::usertype<Node> node_type = lua["Node"];
            node_type["act"] = [](Amara::Node& e, std::string key) -> sol::object {
                Amara::Action* action = e.createChild(key)->as<Amara::Action*>();
                return action->get_lua_object();
            };
            node_type["then"] = [](Amara::Node& e, std::string key) -> sol::object {
                // Chain to child actions.
                Amara::Action* action = nullptr;
                Amara::Node* child = nullptr;
                for (int i = e.children.size()-1; i >= 0; i++) {
                    child = e.children[i];
                    if (!child->destroyed && child->is_action) {
                        action = child->as<Amara::Action*>();
                        if (action) {
                            return action->chain(key);
                        }
                    }
                }
                if (action == nullptr) action = e.createChild(key)->as<Amara::Action*>();

                return action->get_lua_object();
            };
            node_type["stopActions"] = [](Amara::Node& e) -> sol::object {
                for (Amara::Node* child: e.children) {
                    if (!child->destroyed && child->is_action) {
                        child->as<Amara::Action*>()->destroy();
                    }
                }
                return e.get_lua_object();
            };
            node_type["isActing"] = sol::property([](Amara::Node& e) -> bool {
                if (e.is_action) return !e.destroyed;
                for (Amara::Node* child: e.children) {
                    if (!child->destroyed && child->is_action) {
                        Amara::Action* action = child->as<Amara::Action*>();
                        if (!action->chainCompleted()) return true;
                    }
                }
                return false;
            });
            node_type["finishedActing"] = sol::property([](Amara::Node& e) -> bool {
                if (e.is_action) return e.destroyed;
                Amara::Action* action = nullptr;
                for (Amara::Node* child: e.children) {
                    if (!child->destroyed && child->is_action) return false;
                    action = child->as<Amara::Action*>();
                    if (action && !action->chainCompleted()) return false;
                }
                return true;
            });
        }
    };
}