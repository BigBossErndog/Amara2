namespace Amara {
    class Transition: public Amara::Action {
    public:
        std::string next_key;
        Amara::Node* next_node = nullptr;

        bool destroy_past = true;
        bool just_deactivate = false;
        
        Transition(): Amara::Action() {
            set_base_node_id("Transition");
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "next")) {
                if (config["next"].is_string()) next_key = json_extract(config, "next");
            }
            if (json_has(config, "justDeactivate")) {
                just_deactivate = json_extract(config, "justDeactivate");
                destroy_past = !just_deactivate;
            }
            return Amara::Action::configure(config);
        }
        virtual sol::object luaConfigure(std::string key, sol::object val) override {
            if (val.is<Amara::Node>()) {
                if (String::equal(key, "next")) next_node = val.as<Amara::Node*>();
            }
            return Amara::Action::luaConfigure(key, val);
        }

        virtual void prepare() override {
            bringToFront();
            Amara::Action::prepare();
        }

        virtual void doTransition() {
            Amara::node* prev_parent = parent;

            if (!next_key.empty()) {
                if (parent && parent->parent) {
                    next_node = parent->parent->createChild(next_key);
                }
            }
            if (next_node) {
                next_node->activate();
                switchParent(next_node);
                bringToFront();
            }

            if (next_node != prev_parent) {
                if (destroy_past) prev_parent->destroy();
                else if (just_deactivate) prev_parent->deactivate();
            }
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::Transition>("Transition"
                sol::base_classes, sol::bases<Amara::Action>(),
                "doTransition", &Amara::Transition::doTransition
            );
        }
    };
}