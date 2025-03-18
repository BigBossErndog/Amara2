namespace Amara {
    class Tween: public Amara::Action {
    public:
        nlohmann::json start_data = nullptr;
        nlohmann::json target_data = nullptr;

        sol::table lua_actor_table;

        float tween_duration = 0;
        double progress = 0;

        Amara::Ease easing = Amara::Ease::Linear;

        Tween(): Amara::Action() {
            baseEntityID = "Tween";
        }

        sol::object on(sol::table _t) {
            lua_actor_table = _t;
            return get_lua_object();
        }

        sol::object from(sol::table lua_data) {
            start_data = lua_to_json(lua_data);
            return get_lua_object();
        }

        sol::object to(sol::table lua_data) {
            if (!target_data.is_null()) {
                Amara::Tween* tween = addChild(new Tween())->as<Amara::Tween*>();
                return tween->to(lua_data);
            }

            if (lua_data["onFinish"].valid()) {
                onFinish = lua_data["onFinish"];
            }

            nlohmann::json data = lua_to_json(lua_data);
            target_data = nlohmann::json::object();

            for (auto it = data.begin(); it != data.end(); ++it) {
                if (it.value().is_null()) continue;
                
                if (string_equal("duration", it.key())) tween_duration = data["duration"];
                else if (string_equal("ease", it.key())) easing = data["easing"];
                else target_data[it.key()] = it.value();
            }

            return get_lua_object();
        }

        nlohmann::json clean_start_data() {
            std::vector<std::string> to_clean;
            for (auto it = start_data.begin(); it != start_data.end(); ++it) {
                if (!json_has(target_data, it.key())) {
                    to_clean.push_back(it.key());
                }
            }
            for (std::string& key: to_clean) {
                start_data.erase(key);
            }
        }

        virtual void prepare() override {
            if (!target_data.is_null()) {
                if (!lua_actor_table.valid()) {
                    std::cout << actor->id << std::endl;
                    lua_actor_table = actor->get_lua_object().as<sol::table>();
                }
            
                if (start_data.is_null()) {
                    start_data = nlohmann::json::object();
                    for (auto it = target_data.begin(); it != target_data.end(); ++it) {
                        start_data[it.key()] = lua_to_json(lua_actor_table[it.key()]);
                    }
                }
                else {
                    clean_start_data();
                    for (auto it = start_data.begin(); it != start_data.end(); ++it) {
                        lua_actor_table[it.key()] = it.value();
                    }
                }

                Amara::Action::prepare();
            }
        }

        virtual void act(double deltaTime) override {
            if (hasStarted) {
                progress += deltaTime/tween_duration;
                if (progress >= 1) {
                    progress = 1;
                    finish();
                }

                for (auto it = target_data.begin(); it != target_data.end(); ++it) {
                    if (lua_actor_table.valid()) {
                        lua_actor_table[it.key()] = ease(start_data[it.key()], target_data[it.key()], progress, easing);
                    }
                }
            }
            Amara::Action::act(deltaTime);
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Tween>("Tween",
                sol::base_classes, sol::bases<Amara::Action, Amara::Entity>(),
                "progress", sol::readonly(&Tween::progress),
                "from", &Tween::from,
                "to", &Tween::to,
                "on", &Tween::on
            );
        }
    };
}