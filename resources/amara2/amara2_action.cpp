namespace Amara {
    class Action: public Entity, public StateManager {
    public:
        Amara::Entity* actor = nullptr;

        sol::function onPrepare;
        sol::function onAct;
        sol::function onComplete;

        bool isCompleted = false;
        bool hasStarted = false;
        bool isWaitingForChildren = false;

        bool locked = false;

        Action(): Entity() {
            set_base_entity_id("Action");
            depthSortEnabled = true;
            is_action = true;
        }

        virtual void init() override {
            Amara::Entity::init();
            if (actor == nullptr) actor = parent;
        }

        virtual void prepare() {
            hasStarted = true;

            if (onPrepare.valid()) {
                try {
                    onPrepare(actor);
                }
                catch (const sol::error& e) {
                    debug_log("Error: On ", *this, "\" while executing onPrepare().");
                }
            }
        }

        virtual void act(double deltaTime) {
            if (!hasStarted) {
                prepare();
            }
            if (hasStarted && onAct.valid()) {
                try {
                    onAct(actor, deltaTime);
                }
                catch (const sol::error& e) {
                    debug_log("Error: On ", *this, "\" while executing onAct().");
                }
            }
        }

        virtual void update(double deltaTime) override {
            if (!isDestroyed && !locked && !isCompleted) act(deltaTime);
        }

        virtual void run(double deltaTime) override {
            Amara::Entity::run(deltaTime);
            if (isCompleted && !isDestroyed && !has_running_child_actions()) {
                isWaitingForChildren = false;
                // Destroy self when done and children are done.
                destroy();
            }
        }

        Amara::Entity* addChild(Amara::Entity* entity) {
            Amara::Action* action = entity->as<Amara::Action*>();
            if (action) {
                action->actor = actor;
                action->locked = true;
            }
            return Amara::Entity::addChild(entity);
        }

        void complete() {
            if (isCompleted) return;
            isCompleted = true;
            if (onComplete.valid()) {
                try {
                    onComplete(actor);
                }
                catch (const sol::error& e) {
                    debug_log("Error: On ", *this, "\" while executing onComplete().");
                }
            }

            start_child_actions();
            isWaitingForChildren = true;
        }

        bool completeEvt() {
            if (once()) {
                complete();
                return true;
            }
            return false;
        }

        virtual sol::object luaConfigure(std::string key, sol::object val) override {
            if (val.is<sol::function>()) {
                sol::function func = val.as<sol::function>();
                if (string_equal("onPrepare", key)) onPrepare = func;
                else if (string_equal("onAct", key)) onAct = func;
                else if (string_equal("onComplete", key)) onComplete = func;
            }
            return luaConfigure(key, val);
        }

        void start_child_actions() {
            children_copy_list = children;

            Amara::Entity* entity;
            Amara::Action* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                entity = *it; 
                child = dynamic_cast<Amara::Action*>(entity);

                if (entity != nullptr && child == nullptr) entity->destroy();
				
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

        sol::object whenDone(sol::function func) {
            onComplete = func;
            return get_lua_object();
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
            lua.new_usertype<Action>("Action",
                sol::base_classes, sol::bases<Entity>(),
                "onPrepare", &Action::onPrepare,
                "onAct", &Action::onAct,
                "onComplete", &Action::onComplete,
                "hasStarted", sol::readonly(&Action::hasStarted),
                "isCompleted", sol::readonly(&Action::isCompleted),
                "complete", &Action::complete,
                "completeEvt", &Action::completeEvt,
                "addChild", &Action::addChild,
                "chain", &Action::chain,
                "whenDone", &Action::whenDone
            );

            sol::usertype<Entity> entity_type = lua["Entity"];
            entity_type["act"] = sol::property([](Amara::Entity& e, std::string key) -> sol::object {
                Amara::Action* action = e.createChild(key)->as<Amara::Action*>();
                return action->get_lua_object();
            });
            entity_type["isActing"] = sol::property([](Amara::Entity& e) -> bool {
                for (Amara::Entity* ent: e.children) {
                    if (!ent->isDestroyed && ent->is_action) return true;
                }
                return false;
            });
            entity_type["finishedActing"] = sol::property([](Amara::Entity& e) -> bool {
                for (Amara::Entity* ent: e.children) {
                    if (!ent->isDestroyed && ent->is_action) return false;
                }
                return true;
            });
        }
    };
}