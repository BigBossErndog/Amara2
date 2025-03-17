namespace Amara {
    class World;
    class Scene;
    class GameManager;
    class EntityFactory;
    class ScriptFactory;
    class FileManager;
    class MessageQueue;

    class Properties {
    public:
        static std::string context_path;
        static std::string lua_script_path;
        static std::string assets_path;
        static int lua_stack_size;

        static std::string platform;

        static GameManager* game;
        static World* world;
        static Scene* scene;

        static EntityFactory* factory;
        static ScriptFactory* scripts;

        static FileManager* files;
        static MessageQueue* messages;
        static sol::state* lua_state;

        static sol::state& lua() {
            return *lua_state;
        };
        static void set_lua(sol::state& _lua) {
            lua_state = &_lua;
        }
    };
    int Properties::lua_stack_size = 100000;
    GameManager* Properties::game = nullptr;
    World* Properties::world = nullptr;
    Scene* Properties::scene = nullptr;
    EntityFactory* Properties::factory = nullptr;
    ScriptFactory* Properties::scripts = nullptr;
    FileManager* Properties::files = nullptr;
    MessageQueue* Properties::messages = nullptr;
    sol::state* Properties::lua_state = nullptr;

    #ifdef AMARA_FINAL_BUILD
        std::string Properties::context_path = "./";
    #else
        std::string Properties::context_path = "../";
    #endif
    std::string Properties::lua_script_path = "./lua_scripts";
    std::string Properties::assets_path = "./assets";
    std::string Properties::platform = "unknown";
}