namespace Amara {
    class PeriodicAction : public Amara::Action {
    public:
        double counter = 0;
        double period = 1.0;

        bool startWithAct = true;

        PeriodicAction() : Amara::Action() {
            set_base_node_id("PeriodicAction");
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "period")) period = config["period"];
            if (json_has(config, "startWithAct")) startWithAct = config["startWithAct"];

            return Amara::Action::configure(config);
        }

        virtual void prepare() override {
            if (startWithAct) counter = period;
            else counter = 0;
            
            Amara::Action::prepare();
        }

        virtual void act(double deltaTime) override {
            if (!hasStarted) {
                prepare();
            }
            if (hasStarted && !completed) {
                counter += deltaTime;
                if (counter >= period) {
                    counter -= period;
                    if (funcs.hasFunction("onAct")) funcs.callFunction(actor, "onAct", deltaTime);
                }
            }
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<PeriodicAction>("PeriodicAction",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
                "period", &PeriodicAction::period
            );
        }
    };
}