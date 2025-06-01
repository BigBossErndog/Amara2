namespace Amara {
    class Transition: public Amara::Action {
    public:
        std::string next_key;
        Amara::Node* next_node = nullptr;

        bool destroy_past = true;
        bool just_deactivate = false;

        double interim = 0;
        
        Transition(): Amara::Action() {
            set_base_node_id("Transition");
        }

        virtual void init() override {
            Amara::Action::init();
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "next")) {
                if (config["next"].is_string()) next_key = json_extract(config, "next");
            }
            if (json_has(config, "deactivate")) {
                just_deactivate = json_extract(config, "deactivate");
                destroy_past = !just_deactivate;
            }
            if (json_has(config, "interim")) {
                interim = json_extract(config, "interim");
            }
            return Amara::Action::configure(config);
        }
        virtual sol::object luaConfigure(std::string key, sol::object val) override {
            if (val.is<Amara::Node>()) {
                if (String::equal(key, "next")) next_node = val.as<Amara::Node*>();
            }
            return Amara::Action::luaConfigure(key, val);
        }
        
        virtual void doTransition() {
            Amara::Node* prev_parent = parent;

            if (!next_key.empty()) {
                if (parent && parent->parent) {
                    next_node = parent->parent->createChild(next_key);
                    next_node->deactivate();
                }
            }

            if (next_node) {
                if (interim == 0) {
                    next_node->activate();
                }
                else {
                    Amara::DelayedNode* interimNode = parent->parent->createChild("DelayedNode")->as<Amara::DelayedNode*>();
                    interimNode->interim = interim;
                    interimNode->setNode(next_node);
                }
                switchParent(next_node);
            }

            if (next_node != prev_parent) {
                if (destroy_past) prev_parent->destroy();
                else if (just_deactivate) prev_parent->deactivate();
            }
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::Transition>("Transition",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
                "doTransition", &Amara::Transition::doTransition,
                "interim", &Amara::Transition::interim
            );
        }
    };
}