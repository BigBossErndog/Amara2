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
        
        virtual void create() override {
            if (parent) parent->transition = this;
            Amara::Action::create();
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
                    next_config = sol::make_object(gameProps->lua, sol::nil);
                }
            }
            return Amara::Action::configure(config);
        }
        virtual sol::object luaConfigure(std::string key, sol::object val) override {
            if (String::equal(key, "next")) {
                if (val.is<Amara::Node>()) {
                    next_node = val.as<Amara::Node*>();
                    next_key.clear();
                }
                else if (val.is<sol::table>()) {
                    sol::table t = val.as<sol::table>();
                    if (t["node"].valid()) {
                        sol::object node = t["node"];
                        if (node.is<std::string>()) {
                            next_key = node.as<std::string>();
                            next_node = nullptr;
                            t["node"] = sol::nil;
                        }
                        else if (node.is<Amara::Node>()) {
                            next_node = node.as<Amara::Node*>();
                            next_key.clear();
                            t["node"] = sol::nil;
                        }
                    }
                    next_config = t;
                }
            }
            return Amara::Action::luaConfigure(key, val);
        }

        bool checkWorld(Amara::Node* node);
        
        virtual void doTransition() {
            Amara::Node* prev_parent = parent;
            
            if (!next_key.empty()) {
                if (!next_node) {
                    if (parent && parent->parent) {
                        sol::object lua_node = parent->parent->luaCreateChild(next_key, next_config);
                        if (lua_node.is<Amara::Node>()) {
                            next_node = lua_node.as<Amara::Node*>();
                            next_node->deactivate();
                        }
                    }
                }
                else {
                    next_key.clear();
                }
            }
                
            if (funcs.hasFunction("onTransition")) {
                if (next_node) {
                    funcs.callFunction(actor, "onTransition", get_lua_object(), next_node->get_lua_object());
                }
                else {
                    funcs.callFunction(actor, "onTransition", get_lua_object());
                }
            }

            if (next_node) {
                next_node->transition = this;
                if (interim == 0) {
                    next_node->activate();
                    switchParent(next_node);
                }
                else {
                    Amara::DelayNode* interimNode = parent->parent->createChild("DelayNode")->as<Amara::DelayNode*>();
                    interimNode->actor = this;
                    interimNode->interim = interim;
                    interimNode->transition = this;
                    interimNode->passChildren = true;
                    interimNode->setNode(next_node);
                    switchParent(interimNode);
                }
            }

            if (prev_parent && next_node != prev_parent) {
                prev_parent->transition = nullptr;
                if (!checkWorld(prev_parent)) {
                    if (destroy_past) prev_parent->destroy();
                    else if (just_deactivate) prev_parent->deactivate();
                }
            }
        }

        bool transitionFinished() {
            return parent == next_node && (next_node == nullptr || next_node->actuated);
        }

        virtual sol::object complete() override {
            if (completed) return get_lua_object();
            if (next_node) {
                next_node->transition = nullptr;
            }
            return Amara::Action::complete();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::Transition>("Transition",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
                "doTransition", &Amara::Transition::doTransition,
                "transitionFinished", sol::property([](Amara::Transition& self) {
                    return self.transitionFinished();
                }),
                "interim", &Amara::Transition::interim,
                "complete", &Amara::Transition::complete,
                "next", sol::property(
                    [](Amara::Transition& t) -> sol::object {
                        if (t.next_node) {
                            return t.next_node->get_lua_object();
                        }
                        else if (!t.next_key.empty()) {
                            return sol::make_object(t.gameProps->lua, t.next_key);
                        }
                        return sol::nil;
                    },
                    [](Amara::Transition& tr, sol::object val) {
                        if (val.is<Amara::Node>()) {
                            tr.next_node = val.as<Amara::Node*>();
                        }
                        else if (val.is<sol::table>()) {
                            sol::table t = val.as<sol::table>();
                            if (t["node"].valid()) {
                                sol::object node = t["node"];
                                if (node.is<std::string>()) {
                                    tr.next_key = node.as<std::string>();
                                    t["node"] = sol::nil;
                                }
                            }
                            tr.next_config = t;
                        }
                    }
                )
            );

            sol::usertype<Amara::Node> node_type = lua["Node"];
            node_type["transition"] = sol::readonly(&Amara::Node::transition);
        }
    };
}