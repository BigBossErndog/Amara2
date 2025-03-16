namespace Amara {
    class World;
    class EntityFactory;
    class SceneManager;
    class ScriptFactory;
    class FileManager;
    class MessageQueue;

    class WorldProperties {
    public:
        static std::string context_path;
        static std::string lua_script_path;
        static std::string assets_path;
        static int lua_stack_size;

        static World* world;

        static EntityFactory* factory;
        static SceneManager* scenes;
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
    World* WorldProperties::world = nullptr;
    int WorldProperties::lua_stack_size = 100000;
    EntityFactory* WorldProperties::factory = nullptr;
    SceneManager* WorldProperties::scenes = nullptr;
    ScriptFactory* WorldProperties::scripts = nullptr;
    FileManager* WorldProperties::files = nullptr;
    MessageQueue* WorldProperties::messages = nullptr;
    sol::state* WorldProperties::lua_state = nullptr;

    #ifdef AMARA_FINAL_BUILD
        std::string WorldProperties::context_path = "./";
    #else
        std::string WorldProperties::context_path = "../";
    #endif
    std::string WorldProperties::lua_script_path = "./lua_scripts";
    std::string WorldProperties::assets_path = "./assets";
}