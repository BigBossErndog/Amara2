namespace Amara {
    class Scene;

    class Entity {
    public:
        std::string id;
        std::string entityID;

        Amara::Entity* parent = nullptr;
        Amara::Scene* scene = nullptr;

        std::vector<Amara::Entity*> children;
        
        sol::table props;

        sol::function luaPreload;
        sol::function luaCreate;
        sol::function luaUpdate;
        
        MessageBox messages;

        FloatVector3 pos = { 0, 0, 0 };
        
        float depth = 0;
        bool lockDepthToY = false;

        Entity() {
            entityID = "entity";
        }

        virtual void init() {
            if (luaCreate.valid()) {
                try {
                    luaCreate(*this);
                }
                catch (const sol::error& e) {
                    SDL_Log("%s error: %s", entityID.c_str(), e.what());
                }
            }
        }

        virtual void configure(nlohmann::json config) {
            if (json_has(config, "x")) pos.x = config["x"];
            if (json_has(config, "y")) pos.y = config["y"];
            if (json_has(config, "z")) pos.z = config["z"];
        }

        void luaConfigure(sol::object config) {
            if (props["configure"].valid()) {
                sol::function f = props["configure"];
                try {
                    f(*this, config);
                }
                catch (const sol::error& e) {
                    SDL_Log("%s error: %s", entityID.c_str(), e.what());
                }
            }
            configure(lua_to_json(config));
        }

        virtual void preload() {
            if (luaPreload.valid()) {
                try {
                    luaPreload(*this);
                }
                catch (const sol::error& e) {
                    SDL_Log("%s error: %s", entityID.c_str(), e.what());
                }
            }
        }

        virtual void update() {}

        virtual void run() {
            messages.run();

            update();
            if (luaUpdate.valid()) luaUpdate(*this);

            if (lockDepthToY) depth = pos.y;
        }

        Amara::Entity* add(Amara::Entity* entity) {
            entity->scene = scene;
            entity->parent = this;
            children.push_back(entity);
            entity->init();
            return entity;
        }

        sol::object get(std::string key) {
            return props[key];
        }

        void printID() {
            SDL_Log("%s", id.c_str());
        }
        
        static void bindLua(sol::state& lua) {
            lua.new_usertype<Entity>("Entity",
                sol::constructors<Entity()>(),
                "pos", &Entity::pos,
                "id", &Entity::id,
                "entityID", &Entity::entityID,
                "parent", sol::readonly(&Entity::parent),
                "props", &Entity::props,
                "children", &Entity::children,
                "configure", &Entity::luaConfigure,
                "super_configure", &Entity::configure,
                "depth", &Entity::depth,
                "lockDepthToY", &Entity::lockDepthToY,
                "printID", &Entity::printID
            );
        }
    };
}