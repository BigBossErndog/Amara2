namespace Amara {
    class Game;
    class EntityFactory;
    class SceneManager;
    class ScriptFactory;
    class FileManager;
    class MessageQueue;

    class GameProperties {
    public:
        static std::string context_path;
        static std::string lua_script_path;
        static std::string assets_path;
        static int lua_stack_size;

        static Game* game;

        static EntityFactory* factory;
        static SceneManager* scenes;
        static ScriptFactory* scripts;

        static FileManager* files;
        static MessageQueue* messages;
        static sol::state* lua;
    };
    Game* GameProperties::game = nullptr;
    int GameProperties::lua_stack_size = 100000;
    EntityFactory* GameProperties::factory = nullptr;
    SceneManager* GameProperties::scenes = nullptr;
    ScriptFactory* GameProperties::scripts = nullptr;
    FileManager* GameProperties::files = nullptr;
    MessageQueue* GameProperties::messages = nullptr;
    sol::state* GameProperties::lua = nullptr;

    #ifdef AMARA_FINAL_BUILD
        std::string GameProperties::context_path = "./";
    #else
        std::string GameProperties::context_path = "../";
    #endif
    std::string GameProperties::lua_script_path = "./lua";
    std::string GameProperties::assets_path = "./assets";
}