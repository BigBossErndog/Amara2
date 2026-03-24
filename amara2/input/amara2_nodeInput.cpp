namespace Amara {
    class Node;
    class NodeInput;

    class NodeInput: public MessageBox {
    public:
        Button hover;
        bool rec_hovered = false;
        bool hover_by_mouse = false;
        
        bool held = false;
        double timeHeld = false;
        bool draggable = false;
        bool dragging = false;

        Vector2 drag = Vector2(0, 0);

        Vector2 rec_interact_pos;

        CursorEnum cursor = CursorEnum::Default;
        GeneralPointer pointer;

        InputDef lastInteraction;
        
        Amara::Button state;
        
        bool activeClickZone = true;

        NodeInput() = default;

        void configure(nlohmann::json config) {
            if (config.is_boolean()) {
                if (config.get<bool>()) {
                    activate();
                }
                else {
                    deactivate();
                }
            }
            else if (config.is_object()) {
                if (json_has(config, "active")) {
                    if (json_get<bool>(config, "active")) activate();
                    else deactivate();
                }
                if (json_has(config, "draggable")) {
                    draggable = json_get<bool>(config, "draggable");
                }
                if (json_has(config, "cursor")) {
                    cursor = static_cast<CursorEnum>(json_get<int>(config, "cursor"));
                }
                if (json_has(config, "activeClickZone")) {
                    activeClickZone = json_get<bool>(config, "activeClickZone");
                }
            }
        }
        void configure(sol::object config) {
            configure(lua_to_json(config));
            
            if (config.is<sol::table>()) {
                sol::table tbl = config.as<sol::table>();
                for (const auto& it: tbl) {
                    sol::object val = it.second;
                    if (val.is<sol::function>()) {
                        std::string key = it.first.as<std::string>();
                        sol::function func = val.as<sol::function>();
                        
                        listen(key, func);
                    }
                }
            }
        }

        void queueInput(Amara::Shape _shape, Rectangle _viewport, nlohmann::json _data) {
            if (!activeClickZone) return;
            
            gameProps->inputManager->queueInput({
                this,
                _shape.scale(gameProps->passOn.input_scale),
                _viewport*gameProps->passOn.input_scale,
                _data
            });
        }
        
        void run(double deltaTime) {
            hover.update(deltaTime);
            
            if (!messageBox.empty()) {
                MessageQueue* messages = gameProps->messages;
                
                for (
                    auto it = messages->begin();
                    it != messages->end();
                ) {
                    Message& msg = *it;
                    if (msg.active) {
                        handleMessage(msg);
                    }
                    if (msg.sender == this) {
                        it = messages->queue.erase(it);
                        continue;
                    }
                    ++it;
                }
            }

            Amara::Pointer* lastPointer = lastInteraction.lastPointer;
            if (hover.isDown) {
                if (!dragging && (lastPointer == nullptr || !lastPointer->active || !lastInteraction.shape.collidesWith(lastPointer->real_pos))) {
                    hover.release();
                    
                    if (hover_by_mouse) handleMessage({ nullptr, "onMouseExit", sol::nil });
                    handleMessage({ nullptr, "onPointerExit", sol::nil });
                    held = false;
                    
                    hover_by_mouse = false;
                }
                else {
                    handleMessage({ nullptr, "whilePointerHover", sol::nil });

                    if (hover_by_mouse) {
                        handleMessage({ nullptr, "whileMouseHover", sol::nil });
                    }
                }
            }

            if (held) {
                if (lastPointer == nullptr || !lastPointer->active || !lastPointer->state.isDown) {
                    held = false;
                }
            }

            dragging = false;
        }
        
        void post_run(double deltaTime) {
            state.update(deltaTime);
            if (state.isDown && !hover.isDown) {
                state.release();
            }
        }

        virtual void deactivate() override {
            hover.release();
            hover_by_mouse = false;
            held = false;
            timeHeld = 0;
            MessageBox::deactivate();
        }

        static void bind_lua(sol::state& lua) {
            Amara::GeneralPointer::bind_lua(lua);

            lua.new_usertype<NodeInput>("NodeInput",
                sol::base_classes, sol::bases<Amara::MessageBox>(),
                "mouse", sol::property([](Amara::NodeInput& n) { return n.gameProps->inputManager->mouse; }),
                "hovered", sol::property([](Amara::NodeInput& n) { return n.hover.isDown; }),
                "held", sol::readonly(&NodeInput::held),
                "timeHeld", sol::readonly(&NodeInput::timeHeld),
                "draggable", &NodeInput::draggable,
                "drag", sol::readonly(&NodeInput::drag),
                "dragging", sol::readonly(&NodeInput::dragging),
                "configure", sol::resolve<void(sol::object)>(&NodeInput::configure),
                "cursor", &NodeInput::cursor,
                "pointer", sol::property([](Amara::NodeInput& n) {
                    return n.gameProps->inputManager->generalPointer;
                }),
                "state", &Amara::NodeInput::state,
                "configure", sol::resolve<void(sol::object)>(&NodeInput::configure),
                "activeClickZone", &NodeInput::activeClickZone
            );
        }
    };
}