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
        Vector2 real_pos;

        GeneralPointer() = default;
        GeneralPointer(Amara::GameProps* _gameProps) {
            gameProps = _gameProps;
        }

        bool isDown();
        bool justPressed();
        bool justReleased();

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
            
            mouse = Amara::Mouse(gameProps);
            mouse.active = true;
            mouse.id = 0;

            generalPointer = GeneralPointer(_gameProps);

            mouse.luaobject = sol::make_object(gameProps->lua, &mouse);
            touch.luaobject = sol::make_object(gameProps->lua, &touch);
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
                generalPointer.real_pos = mouse.real_pos;
            }
            else if (touch.isDown()) {
                generalPointer.pos = *touch.lastFinger;
                generalPointer.real_pos = touch.lastFinger->real_pos;
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

    bool GeneralPointer::isDown() {
        InputManager* input = gameProps->inputManager;
        return input->mouse.left.isDown || input->touch.isDown();
    }

    bool GeneralPointer::justPressed() {
        InputManager* input = gameProps->inputManager;
        return input->mouse.left.justPressed || input->touch.justPressed();
    }

    bool GeneralPointer::justReleased() {
        InputManager* input = gameProps->inputManager;
        return input->mouse.left.justReleased || input->touch.justReleased();
    }

    void GeneralPointer::bind_lua(sol::state& lua) {
        lua.new_usertype<GeneralPointer>("GeneralPointer",
            "isDown", sol::property([](Amara::GeneralPointer& p) {
                return p.isDown();
            }),
            "justPressed", sol::property([](Amara::GeneralPointer& p) {
                return p.justPressed();
            }),
            "justReleased", sol::property([](Amara::GeneralPointer& p) {
                return p.justReleased();
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