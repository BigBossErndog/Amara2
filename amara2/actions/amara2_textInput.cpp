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
            
            return Amara::Node::configure(config);
        }
        
        virtual void act(double deltaTime) override {
            Amara::Action::act(deltaTime);

            if (hasStarted && recording) {
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

        virtual void destroy() {
            if (!destroyed) stopInput();
            Amara::Node::destroy();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<TextInput>("TextInput",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
                "text", &TextInput::text,
                "setText", &TextInput::setText,
                "recording", sol::readonly(&TextInput::recording),
                "startInput", &TextInput::startInput,
                "stopInput", &TextInput::stopInput
            );
        }
    };
}