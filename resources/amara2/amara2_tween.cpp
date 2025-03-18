namespace Amara {
    class Tween: public Amara::Action {
    public:
        nlohmann::json startData = nullptr;
        nlohmann::json targetData = nullptr;

        sol::table lua_actor_table;

        float duration = 0;
        double progress = 0;

        Amara::Ease easing = Amara::Ease::Linear;

        Tween(): Amara::Action() {
            baseEntityID = "Tween";
        }

        virtual void init() override {
            Amara::Action::init();
        }

        sol::object from(sol::table lua_data) {
            startData = lua_to_json(lua_data);
            return get_lua_object();
        }

        sol::object to(sol::table lua_data) {
            nlohmann::json data = lua_to_json(lua_data);
            targetData = nlohmann::json::object();
            return get_lua_object();
        }

        nlohmann::json clean_start_data() {
            std::vector<std::string> to_clean;
            for (auto it = startData.begin(); it != startData.end(); ++it) {
                if (!json_has(targetData, it.key())) {
                    to_clean.push_back(it.key());
                }
            }
            for (std::string& key: to_clean) {
                startData.erase(key);
            }
        }

        virtual void prepare() override {
            if (!targetData.is_null()) {
                if (!lua_actor_table.valid()) {
                    lua_actor_table = actor->get_lua_object().as<sol::table>();
                }
            
                if (startData.is_null()) {
                    clean_start_data();
                    for (auto it = startData.begin(); it != startData.end(); ++it) {
                        lua_actor_table[it.key()] = it.value();
                    }
                }
                else {
                    startData = nlohmann::json::object();
                    for (auto it = targetData.begin(); it != targetData.end(); ++it) {
                        startData = lua_to_json(lua_actor_table[it.key()]);
                    }
                }

                Amara::Action::prepare();
            }
        }

        virtual void act(double deltaTime) override {
            if (hasStarted) {
                progress += deltaTime/(double)duration;
                if (progress >= 1) {
                    progress = 1;
                    finish();
                }
                for (auto it = targetData.begin(); it != targetData.end(); ++it) {
                    lua_actor_table[it.key()] = ease(startData[it.key()], targetData[it.key()], progress, easing);
                }
            }
            Amara::Action::act(deltaTime);
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Tween>("Tween",
                sol::constructors<Tween()>(),
                sol::base_classes, sol::bases<Amara::Action>(),
                "duration", sol::readonly(&Tween::duration),
                "progress", sol::readonly(&Tween::progress),
                "from", &Tween::from,
                "to", &Tween::to
            );
        }
    };
}