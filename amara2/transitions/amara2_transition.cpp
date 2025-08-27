namespace Amara {
    class Transition: public Amara::Action {
    public:
        std::string next_key;
        Amara::Node* next_node = nullptr;
        sol::object next_config = sol::nil;

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
            if (json_has(config, "deactivate")) {
                just_deactivate = json_extract(config, "deactivate");
                destroy_past = !just_deactivate;
            }
            if (json_has(config, "interim")) {
                interim = json_extract(config, "interim");
            }
            if (json_has(config, "next")) {
                if (config["next"].is_string()) {
                    next_key = json_extract(config, "next");
                }
            }
            return Amara::Action::configure(config);
        }
        virtual sol::object luaConfigure(std::string key, sol::object val) override {
            if (String::equal(key, "next")) {
                if (val.is<sol::table>()) {
                    sol::table t = val.as<sol::table>();
                    if (t["node"].valid()) {
                        sol::object node = t["node"];
                        if (node.is<std::string>()) {
                            next_key = node.as<std::string>();
                            t["node"] = sol::nil;
                        }
                    }
                    next_config = t;
                }
                else if (val.is<Amara::Node>()) {
                    next_node = val.as<Amara::Node*>();
                }
            }
            return Amara::Action::luaConfigure(key, val);
        }
        
        virtual void doTransition() {
            Amara::Node* prev_parent = parent;
            
            if (!next_key.empty()) {
                if (parent && parent->parent) {
                    sol::object lua_node = parent->parent->luaCreateChild(next_key, next_config);
                    if (lua_node.is<Amara::Node>()) {
                        next_node = lua_node.as<Amara::Node*>();
                        next_node->deactivate();
                    }
                }
            }

            if (next_node) {
                if (interim == 0) {
                    next_node->activate();
                }
                else {
                    Amara::DelayNode* interimNode = parent->parent->createChild("DelayNode")->as<Amara::DelayNode*>();
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