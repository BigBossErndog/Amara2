namespace Amara {
    class SceneManager {
    public:
        std::vector<Scene*> scenes;
        std::unordered_map<std::string, Amara::Scene*> sceneMap;

        Amara::Scene* addSceneViaScript(std::string key, std::string path);

        Amara::Scene* add(std::string key, Amara::Scene* scene);

        Amara::Scene* create() {
            return new Amara::Scene();
        }

        static void bindLua(sol::state& lua) {
            Amara::Scene::bindLua(lua);

            lua.new_usertype<SceneManager>("SceneManager",
                "add", &SceneManager::addSceneViaScript,
                "create", &SceneManager::create
            );
        }
    };
}