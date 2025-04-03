namespace Amara {
    class Action: public Node {
    public:
        Amara::Node* actor = nullptr;

        sol::protected_function onPrepare;
        sol::protected_function onAct;
        sol::protected_function onComplete;

        bool isCompleted = false;
        bool hasStarted = false;
        bool isWaitingForChildren = false;
        
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
                    sol::protected_function_result result = onPrepare(actor);
                    if (!result.valid()) {
                        sol::error err = result;
                        throw std::runtime_error("Lua Error: " + std::string(err.what()));  
                    }
                } catch (const std::exception& e) {
                    debug_log(e.what());
                    Props::breakWorld();
                }
            }
        }

        virtual void act(double deltaTime) {
            if (!hasStarted) {
                prepare();
            }
            if (hasStarted && !isCompleted && onAct.valid()) {
                try {
                    sol::protected_function_result result = onAct(actor, deltaTime);
                    if (!result.valid()) {
                        sol::error err = result;
                        throw std::runtime_error("Lua Error: " + std::string(err.what()));  
                    }
                } catch (const std::exception& e) {
                    debug_log(e.what());
                    Props::breakWorld();
                }
            }
        }

        virtual void update(double deltaTime) override {
            if (!isDestroyed && !locked && !isCompleted) act(deltaTime);
        }

        virtual void run(double deltaTime) override {
            Amara::Node::run(deltaTime);
            if (isCompleted && !isDestroyed && !has_running_child_actions()) {
                isWaitingForChildren = false;
                // Destroy self when done and children are done.
                destroy();
            }
        }

        Amara::Node* addChild(Amara::Node* node) override {
            Amara::Action* action = node->as<Amara::Action*>();
            if (action) {
                if (actor == nullptr) actor = parent;
                action->actor = actor;
                if (!isCompleted) action->locked = true;
            }
            return Amara::Node::addChild(node);
        }

        sol::object on(Amara::Node* node) {
            actor = node;
            return get_lua_object();
        }

        virtual sol::object complete() {
            if (isCompleted) return get_lua_object();
            isCompleted = true;
            if (onComplete.valid()) {
                try {
                    sol::protected_function_result result = onComplete(actor);
                    if (!result.valid()) {
                        sol::error err = result;
                        throw std::runtime_error("Lua Error: " + std::string(err.what()));  
                    }
                } catch (const std::exception& e) {
                    debug_log(e.what());
                    Props::breakWorld();
                }
            }

            start_child_actions();
            isWaitingForChildren = true;

            return get_lua_object();
        }

        sol::object whenDone(sol::function func) {
            onComplete = func;
            return get_lua_object();
        }

        virtual sol::object luaConfigure(std::string key, sol::object val) override {
            if (val.is<sol::function>()) {
                sol::function func = val.as<sol::function>();
                if (string_equal("onPrepare", key)) onPrepare = func;
                else if (string_equal("onAct", key)) onAct = func;
                else if (string_equal("onComplete", key)) onComplete = func;
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
				
                if (child == nullptr || child->isDestroyed || child->parent != this) {
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
				if (child == nullptr || child->isDestroyed || child->parent != this) {
					++it;
					continue;
				}
				if (!child->isCompleted || child->isWaitingForChildren) return true;
				++it;
			}
            return false;
        }

        sol::object chain(std::string key) {
            Amara::Action* action = createChild(key)->as<Amara::Action*>();
            return action->get_lua_object();
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Amara::Action>("Action",
                sol::base_classes, sol::bases<Amara::Node>(),
                "onPrepare", &Action::onPrepare,
                "onAct", &Action::onAct,
                "onComplete", &Action::onComplete,
                "hasStarted", sol::readonly(&Action::hasStarted),
                "isCompleted", sol::readonly(&Action::isCompleted),
                "complete", &Action::complete,
                "addChild", &Action::addChild,
                "chain", &Action::chain,
                "whenDone", &Action::whenDone,
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
                    if (!child->isDestroyed && child->is_action) {
                        action = child->createChild(key)->as<Amara::Action*>();
                        break;
                    }
                }
                if (action == nullptr) action = e.createChild(key)->as<Amara::Action*>();

                return action->get_lua_object();
            };
            node_type["isActing"] = sol::property([](Amara::Node& e) -> bool {
                if (e.is_action) return !e.isDestroyed;
                for (Amara::Node* child: e.children) {
                    if (!child->isDestroyed && child->is_action) return true;
                }
                return false;
            });
            node_type["finishedActing"] = sol::property([](Amara::Node& e) -> bool {
                if (e.is_action) return e.isDestroyed;
                for (Amara::Node* child: e.children) {
                    if (!child->isDestroyed && child->is_action) return false;
                }
                return true;
            });
        }
    };
}