namespace Amara {
    class RepeatAction : public Amara::Action {
    public:
        double counter = 0;
        double interim = 1.0;

        int repeats = -1;

        RepeatAction() : Amara::Action() {
            set_base_node_id("RepeatAction");
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "interim")) interim = config["interim"];
            if (json_has(config, "repeats")) repeats = config["repeats"];
            if (json_has(config, "delay")) {
                nlohmann::json& delay = config["delay"];
                if (delay.is_boolean()) {
                    if (delay.get<bool>()) counter = interim;
                }
                else if (delay.is_number()) {
                    counter = delay.get<double>();
                }
            }

            return Amara::Action::configure(config);
        }

        virtual void prepare() override {
            Amara::Action::prepare();
        }

        virtual void act(double deltaTime) override {
            if (!has_started) {
                prepare();
            }
            if (has_started && !completed) {
                counter -= deltaTime;
                while (counter <= 0) {
                    counter += interim;
                    if (funcs.hasFunction("onAct")) funcs.callFunction(actor, "onAct", get_lua_object(), deltaTime);
                    if (repeats > 0) repeats -= 1;
                    if (repeats == 0) {
                        complete();
                        break;
                    }
                }
            }
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<RepeatAction>("RepeatAction",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
                "interim", &RepeatAction::interim,
                "repeats", &RepeatAction::repeats
            );
        }
    };
}