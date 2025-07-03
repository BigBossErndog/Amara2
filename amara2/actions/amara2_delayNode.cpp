namespace Amara {
    class DelayNode: public Amara::Action {
    public:
        double interim = 0;
        double progress = 0;

        Amara::Node* next_node = nullptr;

        DelayNode(): Amara::Action() {
            set_base_node_id("DelayNode");
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "interim")) interim = config["interim"];
            return Amara::Action::configure(config);
        }

        virtual sol::object luaConfigure(std::string key, sol::object val) override {
            if (val.is<Amara::Node>()) {
                if (String::equal(key, "node")) setNode(val.as<Amara::Node*>());
            }
            return Amara::Action::luaConfigure(key, val);
        }
        
        virtual void create() override {
            Amara::Action::create();
        }

        void setNode(Amara::Node* node) {
            next_node = node;
            next_node->deactivate();
        }

        virtual void prepare() override {
            Amara::Action::prepare();
            progress = 0;
        }

        virtual void act(double deltaTime) {
            Amara::Action::act(deltaTime);
            progress += deltaTime/interim;
            if (progress >= 1) {
                next_node->activate();
                complete();
            }
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::DelayNode>("DelayNode",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
                "interim", &Amara::DelayNode::interim,
                "node", sol::property([](Amara::DelayNode& node) -> Amara::Node* { return node.next_node; }, [](Amara::DelayNode& node, Amara::Node* n) { node.setNode(n); })
            );
        }
    };
}