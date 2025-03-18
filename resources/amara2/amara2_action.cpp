namespace Amara {
    class Action: public Entity, public StateManager {
    public:
        Amara::Entity* actor = nullptr;

        sol::function onPrepare;
        sol::function onAct;
        sol::function onFinish;

        bool isFinished = false;
        bool hasStarted = false;

        bool locked = false;

        Action(): Entity() {
            baseEntityID = "Action";
            do_not_depth_sort = true;
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
                    log("Error: On ", *this, "\" while executing onPrepare().");
                }
            }
        }

        virtual void act(double deltaTime) {
            if (!hasStarted) {
                hasStarted = true;
                prepare();
            }
            if (onAct.valid()) {
                try {
                    onAct(actor, deltaTime);
                }
                catch (const sol::error& e) {
                    log("Error: On ", *this, "\" while executing onAct().");
                }
            }
        }

        virtual void update(double deltaTime) override {
            if (!isDestroyed && !locked && !isFinished) act(deltaTime);
        }

        virtual void run(double deltaTime) override {
            Amara::Entity::run(deltaTime);
            if (isFinished && !isDestroyed && !has_running_child_actions()) {
                // Destroy self when done and children are done.
                destroy();
            }
        }

        virtual Amara::Entity* addChild(Amara::Entity* entity) override {
            Amara::Action* action = entity->as<Amara::Action*>();
            if (action) {
                action->actor = actor;
                action->locked = true;
            }
            Amara::Entity::addChild(entity);
        }

        void finish() {
            if (isFinished) return;
            isFinished = true;
            if (onFinish.valid()) {
                try {
                    onFinish(actor);
                }
                catch (const sol::error& e) {
                    log("Error: On ", *this, "\" while executing onFinish().");
                }
            }
        }

        bool finishEvt() {
            if (once()) {
                finish();
                return true;
            }
            return false;
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

        bool has_running_child_actions() {
            children_copy_list = children;

            Amara::Action* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = dynamic_cast<Amara::Action*>(*it);
				if (child == nullptr || child->isDestroyed || child->parent != this) {
					++it;
					continue;
				}
				if (!child->isFinished) return true;
				++it;
			}
            return false;
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Action>("Action",
                sol::base_classes, sol::bases<Amara::Entity, Amara::StateManager>(),
                "onPrepare", &Action::onPrepare,
                "onAct", &Action::onAct,
                "hasStarted", sol::readonly(&Action::hasStarted),
                "isFinished", sol::readonly(&Action::isFinished),
                "finish", &Action::finish,
                "finishEvt", &Action::finishEvt
            );
        }
    };
}