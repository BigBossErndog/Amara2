namespace Amara {
    class FileManager;
    class MessageQueue;

    class GameProperties {
    public:
        static std::string lua_script_path;
        static std::string assets_path;

        static FileManager* files;
        static MessageQueue* messages;
        static sol::state* lua;
    };
    FileManager* GameProperties::files = nullptr;
    MessageQueue* GameProperties::messages = nullptr;
    sol::state* GameProperties::lua = nullptr;

    #ifdef AMARA_FINALBUILD
        std::string GameProperties::lua_script_path = "./lua/";
        std::string GameProperties::assets_path = "./assets/";
    #else
        std::string GameProperties::lua_script_path = "../../lua/";
        std::string GameProperties::assets_path = "../../assets/";
    #endif
}