namespace Amara {
    class TextInput: public Amara::Action {
    public:
        std::string text;

        bool recording = false;
        int selectIndex = 0;

        TextInput(): Amara::Action() {
            set_base_node_id("TextInput");
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "text")) setText(config["text"].get<std::string>());
            if (json_is(config, "startInput")) startInput();
            if (json_is(config, "stopInput")) stopInput();
            if (json_is(config, "clear")) clear();
            
            return Amara::Node::configure(config);
        }
        
        virtual void act(double deltaTime) override {
            Amara::Action::act(deltaTime);

            if (has_started && recording) {
                switch (gameProps->text_input_type) {
                    case TextInputEnum::Text: {
                        text.insert(selectIndex, gameProps->text_input);
                        selectIndex += gameProps->text_input.size();
                        break;
                    }
                    case TextInputEnum::Backspace: {
                        backspace();
                        break;
                    }
                }
                if (gameProps->text_input_type != TextInputEnum::None) {
                    if (funcs.hasFunction("onInput")) {
                        funcs.callFunction(this, "onInput", text);
                    }
                }
            }
        }

        sol::object setText(std::string _text) {
            text = _text;
            selectIndex = text.size();
            return get_lua_object();
        }

        sol::object startInput() {
            if (!recording) {
                recording = true;
                if (gameProps->text_input_count == 0) {
                    SDL_StartTextInput(gameProps->current_window);
                }
                gameProps->text_input_count++;
            }
            return get_lua_object();
        }

        sol::object stopInput() {
            if (recording) {
                recording = false;
                gameProps->text_input_count--;
                if (gameProps->text_input_count == 0) {
                    SDL_StopTextInput(gameProps->current_window);
                }
            }
            return get_lua_object();
        }

        sol::object backspace() {
            if (selectIndex > 0) {
                text.erase(selectIndex - 1, 1);
                selectIndex--;
            }
            return get_lua_object();
        }

        sol::object clear() {
            text = "";
            selectIndex = 0;
            return get_lua_object();
        }

        sol::object split() {
            std::string part1 = text.substr(0, selectIndex);
            std::string part2 = text.substr(selectIndex);
            return json_to_lua(gameProps->lua, nlohmann::json::array({part1, part2}));
        }

        virtual void destroy() {
            if (!destroyed) stopInput();
            Amara::Node::destroy();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<TextInput>("TextInput",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
                "text", sol::property([](TextInput& t) -> std::string { return t.text; }, &TextInput::setText),
                "setText", &TextInput::setText,
                "recording", sol::readonly(&TextInput::recording),
                "startInput", &TextInput::startInput,
                "stopInput", &TextInput::stopInput,
                "backspace", &TextInput::backspace,
                "clear", &TextInput::clear,
                "split", &TextInput::split
            );
        }
    };
}