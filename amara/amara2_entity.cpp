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
            entityID = "Entity";
            
            props = GameProperties::lua->create_table();

            sol::table props_meta = GameProperties::lua->create_table();
            props_meta["__newindex"] = [this](sol::table tbl, sol::object key, sol::object value) {
                if (value.is<sol::function>()) {
                    sol::function callback = value.as<sol::function>();
                    sol::function func = sol::make_object(*GameProperties::lua, [this, callback](sol::variadic_args va)->sol::object {
                        return callback(this, sol::as_args(va));
                    });
                    tbl.raw_set(key, func);
                }
                else tbl.raw_set(key, value);
            };
            props[sol::metatable_key] = props_meta;
        }

        virtual void init() {
            if (luaCreate.valid()) {
                try {
                    luaCreate(*this);
                }
                catch (const sol::error& e) {
                    c_style_log("%s: \"%s\" error on create().", entityID.c_str(), id.c_str());
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
                    c_style_log("%s: \"%s\" error on configure().", entityID.c_str(), id.c_str());
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
                    c_style_log("%s: \"%s\" error on preload().", entityID.c_str(), id.c_str());
                }
            }
        }

        virtual void update() {}

        virtual void run() {
            messages.run();

            update();
            if (luaUpdate.valid()) {
                try {
                    luaUpdate(*this);
                }
                catch (const sol::error& e) {
                    c_style_log("%s: \"%s\" error on update().", entityID.c_str(), id.c_str());
                }
            }

            if (lockDepthToY) depth = pos.y;
        }

        Amara::Entity* add(Amara::Entity* entity) {
            entity->scene = scene;
            entity->parent = this;
            children.push_back(entity);
            entity->init();
            return entity;
        }
        sol::object luaAdd(std::string);

        sol::object get(std::string key) {
            if (props[key].valid() && props[key].is<sol::function>()) {
                sol::function func = props[key];
                return sol::make_object(*GameProperties::lua, [this, &func](sol::table table, sol::variadic_args va, sol::this_state s) -> sol::object {
                    return func(this, sol::as_args(va));
                });
            }
            return props[key];
        }
        void set(std::string key, sol::object obj) {
            props[key] = obj;
        }

        sol::object as(std::string);

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Entity>("Entity",
                sol::constructors<Entity()>(),
                "pos", &Entity::pos,
                "id", &Entity::id,
                "entityID", sol::readonly(&Entity::entityID),
                "parent", sol::readonly(&Entity::parent),
                "props", &Entity::props,
                "children", &Entity::children,
                "configure", &Entity::luaConfigure,
                "super_configure", &Entity::configure,
                "depth", &Entity::depth,
                "lockDepthToY", &Entity::lockDepthToY,
                "onPreload", &Entity::luaPreload,
                "onCreate", &Entity::luaCreate,
                "onUpdate", &Entity::luaUpdate,
                "createChild", &Entity::luaAdd,
                "addChild", &Entity::add,
                "get", &Entity::get,
                "set", &Entity::set
            );
        }
    };
}