namespace Amara {
    class Entity;
    class World;
    class Scene;
    class GameManager;
    class EntityFactory;
    class ScriptFactory;
    class FileManager;
    class MessageQueue;
    class GarbageCollector;

    class Props {
    public:
        static std::string context_path;
        static std::string lua_script_path;
        static std::string assets_path;
        static int lua_stack_size;
        
        static std::string platform;

        static bool integrate_new_windows;

        static double deltaTime;

        static Amara::GraphicsEnum graphics;
        static Amara::World* render_origin;
        static Rectangle viewport;
        static Rectangle master_viewport;
        static Rectangle display;

        static SDL_GPUDevice* gpuDevice;
        static SDL_GLContext glContext;
        static SDL_Renderer* renderer;

        static SDL_Window* current_window;
        static SDL_DisplayID displayID;

        static GameManager* game;
        static World* world;
        static Scene* scene;

        static EntityFactory* factory;
        static ScriptFactory* scripts;
        static FileManager* files;
        static MessageQueue* messages;
        static GarbageCollector* garbageCollector;

        static bool lua_exception_thrown;

        static void queue_garbage(Amara::Entity*, double);
        static void queue_garbage(Amara::Entity* entity) {
            Props::queue_garbage(entity, 1);
        }

        static sol::state* lua_state;

        static sol::state& lua() {
            return *lua_state;
        };
        static void set_lua(sol::state& _lua) {
            lua_state = &_lua;
        }
    };

    #ifdef AMARA_FINAL_BUILD
        std::string Props::context_path = "./";
    #else
        std::string Props::context_path = "../";
    #endif
    std::string Props::lua_script_path = "./lua_scripts";
    std::string Props::assets_path = "./assets";
    std::string Props::platform = "unknown";

    bool Props::integrate_new_windows = false;

    double Props::deltaTime = 1;

    Amara::GraphicsEnum Props::graphics = GraphicsEnum::None;
    Amara::World* Props::render_origin = nullptr;
    Rectangle Props::viewport;
    Rectangle Props::master_viewport;
    Rectangle Props::display;

    SDL_Window* Props::current_window = nullptr;
    SDL_DisplayID Props::displayID = 0;

    SDL_GLContext Props::glContext = NULL;
    SDL_GPUDevice* Props::gpuDevice = nullptr;
    SDL_Renderer* Props::renderer = nullptr;

    int Props::lua_stack_size = 5000000;
    
    GameManager* Props::game = nullptr;
    World* Props::world = nullptr;
    Scene* Props::scene = nullptr;
    EntityFactory* Props::factory = nullptr;
    ScriptFactory* Props::scripts = nullptr;
    FileManager* Props::files = nullptr;
    MessageQueue* Props::messages = nullptr;
    GarbageCollector* Props::garbageCollector = nullptr;

    bool Props::lua_exception_thrown = false;
    
    sol::state* Props::lua_state = nullptr;
}