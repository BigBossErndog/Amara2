namespace Amara {
    class EntityFactory {
    public:
        std::unordered_map<std::string, std::function<Entity*()>> factory;
        std::unordered_map<std::string, std::string> entityScripts;

        sol::table props;

        void add(std::string key, std::string script) {
            entityScripts[key] = script;
        }
        static Amara::Entity invalidEntity;
        Amara::Entity* create(std::string key) {
            auto it = factory.find(key);
            if (it != factory.end() && it->second) return (it->second());
            
            if (entityScripts.find(key) == entityScripts.end()) {
                SDL_Log("Error: Entity \"%s\" has not been defined.", key.c_str());
                return nullptr;
            }
            Amara::Entity* entity = nullptr;
            try {
                return (GameProperties::files->run(entityScripts[key]).as<Amara::Entity*>());
            }
            catch (const sol::error& e) {
                SDL_Log("%s error: %s", key.c_str(), e.what());
            }
            return nullptr;
        }

        void prepareEntities() {
            invalidEntity.id = "invalid";

            factory["entity"] = []() -> Entity* { return new Entity(); };
        }

        static void bindLua(sol::state& lua) {
            Entity::bindLua(lua);

            lua.new_usertype<EntityFactory>("EntityFactory",
                "add", &EntityFactory::add,
                "props", &EntityFactory::props,
                "create", &EntityFactory::create
            );
        }
    };
    Entity EntityFactory::invalidEntity = Amara::Entity();
}