namespace Amara {
    class Node;

    class NodeInput: public MessageBox {
    public:
        Amara::Shape shape;

        Button hover;
        bool rec_hovered = false;
        bool hover_by_mouse = false;

        Amara::Pointer* lastPointer = nullptr;
        
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
                    }
                    if (msg.sender == this) {
                        it = messages->queue.erase(it);
                        continue;
                    }
                    ++it;
                }
            }

            if (hover.isDown) {
                if (lastPointer == nullptr || !lastPointer->active || !shape.collidesWith(lastPointer->realPos)) {
                    hover.release();

                    if (hover_by_mouse) handleMessage({ nullptr, "onMouseExit", sol::nil });
                    handleMessage({ nullptr, "onPointerExit", sol::nil });
                    
                    hover_by_mouse = false;
                }
            }
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<NodeInput>("NodeInput",
                sol::base_classes, sol::bases<Amara::MessageBox>(),
                "mouse", sol::property([](Amara::NodeInput& n) { return n.gameProps->inputManager->mouse; }),
                "hovered", sol::property([](Amara::NodeInput& n) { return n.hover.isDown; }),
                "lastPointer", sol::readonly(&NodeInput::lastPointer)
            );
        }
    };
}