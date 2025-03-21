namespace Amara {
    class World;
    class Scene;
    class GameManager;
    class EntityFactory;
    class ScriptFactory;
    class FileManager;
    class MessageQueue;

    class Props {
    public:
        static std::string context_path;
        static std::string lua_script_path;
        static std::string assets_path;
        static int lua_stack_size;

        static std::string platform;

        static double deltaTime;

        static GameManager* game;
        static World* world;
        static Scene* scene;

        static EntityFactory* factory;
        static ScriptFactory* scripts;

        static FileManager* files;
        static MessageQueue* messages;
        static sol::state* lua_state;

        static Rectangle viewport;

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
    double Props::deltaTime = 1;

    int Props::lua_stack_size = 1000000;
    
    GameManager* Props::game = nullptr;
    World* Props::world = nullptr;
    Scene* Props::scene = nullptr;
    EntityFactory* Props::factory = nullptr;
    ScriptFactory* Props::scripts = nullptr;
    FileManager* Props::files = nullptr;
    MessageQueue* Props::messages = nullptr;
    sol::state* Props::lua_state = nullptr;

    Rectangle Props::viewport;

}