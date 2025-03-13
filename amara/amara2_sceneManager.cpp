namespace Amara {
    class SceneManager {
    public:
        sol::state* lua;

        std::vector<Scene*> scenes;
        std::unordered_map<std::string, Amara::Scene*> sceneMap;

        void init(sol::state& gLua) {
            lua = &gLua;
            luaBind(gLua);
        }

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

        void luaBind(sol::state& lua) {
            lua.new_usertype<SceneManager>("SceneManager",
                "add", &SceneManager::addSceneViaScript
            );

            Scene::luaBind(lua);
        }
    };
}