namespace Amara {
    class NodeInput;
    
    class InputManager {
    public:
        std::vector<Amara::NodeInput*> queue;

        Mouse mouse;

        void queueInput(Amara::NodeInput* input) {
            queue.push_back(input);
        }

        void clearQueue() {
            queue.clear();
        }
        void handleMouseMovement(const Amara::Vector2& pos);
        void handleMouseDown(const Amara::Vector2& point);
        void handleMouseUp(const Amara::Vector2& point);
    };
}