namespace Amara {
    class Node;
    class World;
    class Scene;
    class GameManager;
    class NodeFactory;
    class ScriptFactory;
    class FileManager;
    class AssetManager;
    class MessageQueue;
    class GarbageCollector;
    class Asset;

    struct PassOnProps {
        float rotation = 0;
        Vector3 anchor = { 0, 0, 0 };
        Vector2 scale = { 1, 1 };
        
        Vector2 scroll = { 0, 0 };
        Vector2 zoom = { 1, 1 };
        
        float cameraRotation = 0;
    };

    class Props {
    public:
        static std::string context_path;
        static std::string lua_script_path;
        static std::string assets_path;
        static int lua_stack_size;

        static std::string platform;

        static bool integrate_new_windows;

        static double deltaTime;
        
        static Amara::World* render_origin;
        static Rectangle master_viewport;
        static Rectangle display;

        static PassOnProps passOn;

        static GraphicsEnum graphics;

        static SDL_GPUDevice* gpuDevice;
        static SDL_GLContext glContext;
        static SDL_Renderer* renderer;

        static bool glFunctionsLoaded;

        static SDL_Window* current_window;
        static SDL_DisplayID displayID;

        static GameManager* game;
        static World* world;
        static Scene* scene;

        static NodeFactory* factory;
        static ScriptFactory* scripts;
        static FileManager* files;
        static AssetManager* assets;
        static MessageQueue* messages;
        static GarbageCollector* garbageCollector;

        static bool lua_exception_thrown;

        static void queue_garbage(Amara::Node*, double);
        static void queue_garbage(Amara::Node* node) {
            Props::queue_garbage(node, 1);
        }
        static void queue_asset_garbage(Amara::Asset* asset, double expiration);
        static void queue_asset_garbage(Amara::Asset* asset) {
            Props::queue_asset_garbage(asset, 1);
        }
        static void queue_texture_garbage(GLuint textureID);

        static sol::state* lua_state;

        static std::vector<std::string> debug_log_buffer;
        static std::vector<std::string> debug_log_queue;

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

    Amara::World* Props::render_origin = nullptr;
    Rectangle Props::master_viewport;
    Rectangle Props::display;
    
    PassOnProps Props::passOn;

    SDL_Window* Props::current_window = nullptr;
    SDL_DisplayID Props::displayID = 0;

    GraphicsEnum Props::graphics = GraphicsEnum::None;

    SDL_GLContext Props::glContext = NULL;
    SDL_Renderer* Props::renderer = nullptr;
    SDL_GPUDevice* Props::gpuDevice = nullptr;

    bool Props::glFunctionsLoaded = false;

    int Props::lua_stack_size = 5000000;
    
    GameManager* Props::game = nullptr;
    World* Props::world = nullptr;
    Scene* Props::scene = nullptr;
    NodeFactory* Props::factory = nullptr;
    ScriptFactory* Props::scripts = nullptr;
    FileManager* Props::files = nullptr;
    AssetManager* Props::assets = nullptr;
    MessageQueue* Props::messages = nullptr;
    GarbageCollector* Props::garbageCollector = nullptr;

    bool Props::lua_exception_thrown = false;
    
    sol::state* Props::lua_state = nullptr;
}