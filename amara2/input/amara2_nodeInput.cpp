namespace Amara {
    class Node;

    class NodeInput: public MessageBox {
    public:
        Amara::Shape shape;

        Button hover;
        bool rec_hovered = false;

        void queueInput(const Amara::Shape::ShapeVariant& _shape) {
            shape = _shape;
            gameProps->inputManager->queueInput(this);
        }

        void update() {
            if (rec_hovered) {
                rec_hovered = false;
            }
            else {
                if (hover.isDown) {
                    hover.release();
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