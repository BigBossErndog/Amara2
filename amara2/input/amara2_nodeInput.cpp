namespace Amara {
    class Node;

    class NodeInput: public MessageBox {
    public:
        Amara::Shape shape;

        Button hover;
        bool rec_hovered = false;
        bool hover_by_mouse = false;

        void queueInput(const Amara::Shape::ShapeVariant& _shape) {
            shape = _shape;
            gameProps->inputManager->queueInput(this);
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
                        if (hover.isDown && String::equal(msg.key, "onMouseMove")) {
                            Vector2 pos = msg.data.as<Vector2>();
                            if (!shape.collidesWith(pos)) {
                                hover.release();

                                if (hover_by_mouse) handleMessage({ nullptr, "onMouseExit", sol::nil });
                                handleMessage({ nullptr, "onPointerExit", sol::nil });
                                
                                hover_by_mouse = false;
                            }
                        }
                    }
                    if (msg.sender == this) {
                        it = messages->queue.erase(it);
                        continue;
                    }
                    ++it;
                }
            }
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<NodeInput>("NodeInput",
                sol::base_classes, sol::bases<MessageBox>(),
                "mouse", sol::property([](Amara::NodeInput& n) { return n.gameProps->inputManager->mouse; }),
                "hovered", sol::property([](Amara::NodeInput& n) { return n.hover.isDown; })
            );
        }
    };
}