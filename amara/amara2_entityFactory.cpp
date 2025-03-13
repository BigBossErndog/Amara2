namespace Amara {
    class EntityFactory {
    public:
        std::unordered_map<std::string, std::function<Entity*()>> factory;

        std::unordered_map<std::string, std::string> entityScripts;

        void add(std::string key, std::string script) {
            entityScripts[key] = script;
        }

        Amara::Entity* create(std::string key) {
            auto it = factory.find(key);
            if (it != factory.end() && it->second) return it->second();

            if (entityScripts.find(key) == entityScripts.end()) {
                SDL_Log("");
                return nullptr;
            }
            Amara::Entity* entity = nullptr;
            try {
                std::string path = entityScripts[key];
                entity = GameProperties::files->run(string_concat(std::vector<std::string>({
                    GameProperties::lua_script_path,
                    entityScripts[key]
                }))).as<Amara::Entity*>();
            }
            catch (const sol::error& e) {
                SDL_Log("%s error: %s", key.c_str(), e.what());
                entity = nullptr;
            }
            return entity;
        }

        Entity* entity() { return new Entity(); }

        void luaBind(sol::state& lua) {
            lua.new_usertype<EntityFactory>("EntityFactory",
                "add", &EntityFactory::add,
                "create", &EntityFactory::create,
                "entity", &EntityFactory::entity
            );

            Entity::luaBind(lua);
        }
    };
}