namespace Amara {
    class NodeInput;
    class World;

    struct InputDef {
        NodeInput* input = nullptr;
        Shape shape;
        Rectangle viewport;
        nlohmann::json data;
        Pointer* lastPointer = nullptr;
    };

    class GeneralPointer {
    public:
        Amara::GameProps* gameProps = nullptr;
        
        Vector2 pos;

        GeneralPointer() = default;
        GeneralPointer(Amara::GameProps* _gameProps) {
            gameProps = _gameProps;
        }

        static void bind_lua(sol::state& lua);
    };
    
    class InputManager {
    public:
        Amara::GameProps* gameProps = nullptr;
        Amara::World* world = nullptr;
        
        std::vector<Amara::InputDef> queue;

        Mouse mouse;
        TouchHandler touch;

        GeneralPointer generalPointer;

        bool any_hovered = false;

        bool force_release_pointer = false;

        Amara::InputMode pointerMode = Amara::InputMode::None;

        void init(Amara::GameProps* _gameProps, Amara::World* _world) {
            gameProps = _gameProps;
            world = _world;
            
            mouse.active = true;
            mouse.id = 0;

            generalPointer = GeneralPointer(_gameProps);
        }

        void queueInput(const Amara::InputDef& input) {
            queue.push_back(input);
        }

        void clearQueue() {
            queue.clear();
        }

        void pre_update(double deltaTime) {
            if (mouse.moved) {
                generalPointer.pos = mouse;
            }
            else if (touch.isDown()) {
                generalPointer.pos = *touch.lastFinger;
            }
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

    void GeneralPointer::bind_lua(sol::state& lua) {
        lua.new_usertype<GeneralPointer>("GeneralPointer",
            "isDown", sol::property([](Amara::GeneralPointer& p) {
                InputManager* input = p.gameProps->inputManager;
                return input->mouse.left.isDown || input->touch.isDown();
            }),
            "justPressed", sol::property([](Amara::GeneralPointer& p) {
                InputManager* input = p.gameProps->inputManager;
                return input->mouse.left.justPressed || input->touch.justPressed();
            }),
            "justReleased", sol::property([](Amara::GeneralPointer& p) {
                InputManager* input = p.gameProps->inputManager;
                return input->mouse.left.justReleased || input->touch.justReleased();
            }),
            "x", sol::property([](Amara::GeneralPointer& p) {
                return p.pos.x;
            }),
            "y", sol::property([](Amara::GeneralPointer& p) {
                return p.pos.y;
            }),
            "pos", sol::readonly(&GeneralPointer::pos)
        );
    }
}