namespace Amara {
    class SceneManager {
    public:
        std::vector<Scene*> scenes;
        std::unordered_map<std::string, Amara::Scene*> sceneMap;

        Amara::Scene* addSceneViaScript(std::string key, std::string path) {
            Scene* scene = GameProperties::files->run(path).as<Amara::Scene*>();
            sceneMap[key] = scene;
            scenes.push_back(scene);
            return scene;
        }

        Amara::Scene* add(std::string key, Amara::Scene* scene) {
            sceneMap[key] = scene;
            scenes.push_back(scene);
            return scene;
        }

        Amara::Scene* create() {
            return new Amara::Scene();
        }

        static void bindLua(sol::state& lua) {
            Scene::bindLua(lua);

            lua.new_usertype<SceneManager>("SceneManager",
                "add", &SceneManager::addSceneViaScript,
                "create", &SceneManager::create
            );
        }
    };
}