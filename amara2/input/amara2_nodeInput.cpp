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

        Vector2 drag = Vector2(0, 0);

        Vector2 rec_interact_pos;

        CursorEnum cursor = CursorEnum::Default;

        InputDef lastInteraction;

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
                    if (config["active"]) activate();
                    else deactivate();
                }
                if (json_has(config, "draggable")) {
                    draggable = config["draggable"];
                }
                if (json_has(config, "cursor")) {
                    cursor = config["cursor"];
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

        void queueInput(const Amara::Shape::ShapeVariant& _shape, Rectangle _viewport, nlohmann::json _data) {
            gameProps->inputManager->queueInput({ this, _shape, _viewport, _data });
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
                if (lastPointer == nullptr || !lastPointer->active || !lastInteraction.shape.collidesWith(lastPointer->real_pos)) {
                    hover.release();

                    if (hover_by_mouse) handleMessage({ nullptr, "onMouseExit", sol::nil });
                    handleMessage({ nullptr, "onPointerExit", sol::nil });
                    
                    hover_by_mouse = false;
                }
                else {
                    handleMessage({ nullptr, "whilePointerHover", sol::nil });
                }
            }

            if (held) {
                if (lastPointer == nullptr || !lastPointer->active || !lastPointer->state.isDown) {
                    held = false;
                }
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
            lua.new_usertype<NodeInput>("NodeInput",
                sol::base_classes, sol::bases<Amara::MessageBox>(),
                "mouse", sol::property([](Amara::NodeInput& n) { return n.gameProps->inputManager->mouse; }),
                "hovered", sol::property([](Amara::NodeInput& n) { return n.hover.isDown; }),
                "held", sol::readonly(&NodeInput::held),
                "timeHeld", sol::readonly(&NodeInput::timeHeld),
                "draggable", &NodeInput::draggable,
                "configure", sol::resolve<void(sol::object)>(&NodeInput::configure),
                "cursor", &NodeInput::cursor
            );
        }
    };
}