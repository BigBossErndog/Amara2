namespace Amara {
    class WaitAction: public Action {
    public:
        float duration = 0;
        double progress = 0;

        WaitAction(): Action() {
            set_base_node_id("WaitAction");
        }

        virtual void prepare() override {
            progress = 0;
            Action::prepare();
        }

        virtual void act(double deltaTime) override {
            Action::act(deltaTime);
            progress += deltaTime/duration;
            if (progress >= 1) {
                progress = 1;
                complete();
            }
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<WaitAction>("WaitAction",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
                "progress", sol::readonly(&WaitAction::progress),
                "duration", sol::readonly(&WaitAction::duration)
            );
            sol::usertype<Amara::Node> node_type = lua["Node"];
            node_type["wait"] = [](Amara::Node& e, float duration) -> sol::object {
                Amara::WaitAction* action = e.addChild(new WaitAction())->as<Amara::WaitAction*>();
                action->duration = duration;
                return action->get_lua_object();
            };
        }
    };
}