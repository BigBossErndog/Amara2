namespace Amara {
    class AutoProgress: public Amara::Action {
    public:
        Amara::Text* text = nullptr;

        int startIndex = -1;
        double timer = 0;
        double speed = 1; // speed = characters per second

        int until = -1;

        AutoProgress(): Amara::Action() {
            set_base_node_id("AutoProgress");
            is_autoprogress = true;
        }

        Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "speed")) speed = config["speed"];
            if (json_has(config, "start")) startIndex = config["start"];
            if (json_has(config, "progressUntil")) until = config["progressUntil"];

            return Action::configure(config);
        }
        
        virtual sol::object luaConfigure(std::string key, sol::object val) override {
            if (val.is<sol::function>()) {
                if (String::equal(key, "skipCondition")) {
                    funcs.setFunction(nodeID, "skipCondition", val);
                }
            }
            return Amara::Action::luaConfigure(key, val);
        }

        virtual void prepare() override {
            Amara::Action::prepare();

            if (text == nullptr) text = actor->as<Amara::Text*>();
            if (text == nullptr) complete();
            else {
                if (startIndex >= 0) text->progress = startIndex;
            }

            if (until < 0) until = text->layout.size();
        }

        virtual void act(double deltaTime) override {
            Amara::Action::act(deltaTime);

            if (has_started) {
                timer += deltaTime * speed;
                while (timer > 1.0) {
                    timer -= 1.0;
                    if (text->progressText(1) || text->progress >= until) {
                        complete();
                        break;
                    }
                }
                if (!completed && funcs.hasFunction("skipCondition")) {
                    if (funcs.callFunction(actor, "skipCondition").as<bool>()) {
                        skip();
                    }
                }
            }
        }

        sol::object skip() {
            if (!completed) {
                if (text) text->progress = until;
                complete();
            }
            return get_lua_object();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<AutoProgress>("AutoProgress",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
                "skip", &AutoProgress::skip
            );
        }
    };

    sol::object Amara::Text::autoProgress(sol::table sol_config) {
        nlohmann::json config = lua_to_json(sol_config);

        if (json_has(config, "start")) progress = config["start"];
        else progress = 0;
        
        for (Amara::Node* node: children) {
            if (node->is_autoprogress && !node->destroyed) {
                Amara::AutoProgress* action = node->as<Amara::AutoProgress*>();
                if (action) action->destroy();
            }
        }

        Amara::Node* action = createChild("AutoProgress");
        action->luaConfigure(sol_config);

        return action->get_lua_object();
    }
    sol::object Amara::Text::autoProgress(double speed) { // speed = characters per second
        nlohmann::json config = nlohmann::json::object();
        config["speed"] = speed;

        return autoProgress(json_to_lua(gameProps->lua, config));
    }
    sol::object Amara::Text::skipProgress() {
        for (Amara::Node* node: children) {
            if (node->is_autoprogress && !node->destroyed) {
                Amara::AutoProgress* action = node->as<Amara::AutoProgress*>();
                if (action) action->skip();
            }
        }

        return get_lua_object();
    }
}