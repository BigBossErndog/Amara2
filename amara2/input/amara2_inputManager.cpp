namespace Amara {
    class NodeInput;
    class World;
    
    class InputManager {
    public:
        Amara::GameProps* gameProps = nullptr;
        Amara::World* world = nullptr;
        
        std::vector<Amara::NodeInput*> queue;

        Mouse mouse;
        TouchHandler touch;

        bool any_hovered = false;

        bool force_release_pointer = false;

        Amara::InputMode pointerMode = Amara::InputMode::None;

        void init(Amara::GameProps* _gameProps, Amara::World* _world) {
            gameProps = _gameProps;
            world = _world;
            
            mouse.active = true;
            mouse.id = 0;
        }

        void queueInput(Amara::NodeInput* input) {
            queue.push_back(input);
        }

        void clearQueue() {
            queue.clear();
        }

        void update(double deltaTime) {
            if (force_release_pointer) forceReleasePointer();

            mouse.update(deltaTime);
            touch.update(deltaTime);
        }

        bool checkPointerHover(const Vector2& pos);

        void forceReleasePointer();

        void handleMouseMovement(const Amara::Vector2& pos);
        void handleMouseDown(const Amara::Vector2& point);
        void handleMouseUp(const Amara::Vector2& point);

        void handleFingerEvent(const Amara::Vector2& pos, Pointer* finger, SDL_EventType eventType);
    };
}