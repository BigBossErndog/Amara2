namespace Amara {
    class Tween: public Amara::Action {
    public:
        nlohmann::json start_data = nullptr;
        nlohmann::json target_data = nullptr;

        sol::table lua_actor_table;

        float tween_duration = 0;
        double progress = 0;

        Amara::Ease easing = Amara::Ease::Linear;

        bool yoyo = false;
        bool waitingYoyo = false;

        int repeats = 0;

        sol::protected_function onUpdate;

        Tween(): Amara::Action() {
            set_base_node_id("Tween");
        }

        sol::object from(sol::table lua_data) {
            if (!target_data.is_null()) {
                Amara::Tween* tween = createChild("Tween")->as<Amara::Tween*>();
                return tween->from(lua_data);
            }
            start_data = lua_to_json(lua_data);
            return get_lua_object();
        }
        
        sol::object to(sol::table lua_data) {
            if (!target_data.is_null()) {
                Amara::Tween* tween = createChild("Tween")->as<Amara::Tween*>();
                return tween->to(lua_data);
            }

            target_data = nlohmann::json::object();

            if (lua_data["onComplete"].valid()) {
                onComplete = lua_data["onComplete"];
                lua_data["onComplete"] = sol::nil;
            }
            if (lua_data["onUpdate"].valid()) {
                onUpdate = lua_data["onUpdate"];
                lua_data["onUpdate"] = sol::nil;
            }
            if (lua_data["duration"].valid()) {
                tween_duration = lua_data["duration"];
                lua_data["duration"] = sol::nil;
            }
            if (lua_data["ease"].valid()) {
                easing = lua_data["ease"].get<Amara::Ease>();
                lua_data["ease"] = sol::nil;
            }
            if (lua_data["yoyo"].valid()) {
                yoyo = lua_data["yoyo"].get<bool>();
                waitingYoyo = yoyo;
            }
            if (lua_data["repeats"].valid()) {
                repeats = lua_data["repeats"].get<int>();
            }
            if (lua_data["color"].valid()) {
                Amara::Color color = lua_to_json(lua_data["color"]);
                target_data["color"] = color.toJSON();
                lua_data["color"] = sol::nil;
            }
            if (lua_data["tint"].valid()) {
                Amara::Color color = lua_to_json(lua_data["tint"]);
                target_data["tint"] = color.toJSON();
                lua_data["tint"] = sol::nil;
            }

            nlohmann::json data = lua_to_json(lua_data);
            
            for (auto it = data.begin(); it != data.end(); ++it) {
                if (it.value().is_null()) continue;
                target_data[it.key()] = it.value();
            }

            return get_lua_object();
        }

        void clean_data() {
            std::vector<std::string> to_clean;
            for (auto it = start_data.begin(); it != start_data.end(); ++it) {
                if (!json_has(target_data, it.key()) || !(it.value().is_number() || it.value().is_object())) {
                    to_clean.push_back(it.key());
                }
            }
            for (std::string& key: to_clean) {
                json_erase(start_data, key);
                json_erase(target_data, key);
            }
        }

        virtual void prepare() override {
            if (!target_data.is_null()) {
                progress = 0;
                
                if (!lua_actor_table.valid()) {
                    lua_actor_table = actor->get_lua_object().as<sol::table>();
                }
            
                if (start_data.is_null()) {
                    start_data = nlohmann::json::object();
                    if (lua_actor_table.valid()) {
                        for (auto it = target_data.begin(); it != target_data.end(); ++it) {
                            start_data[it.key()] = lua_to_json(lua_actor_table[it.key()]);
                        }
                    }
                }
                else if (lua_actor_table.valid()) {
                    for (auto it = start_data.begin(); it != start_data.end(); ++it) {
                        lua_actor_table.set(it.key(), json_to_lua(gameProps->lua, it.value()));
                    }
                }
                clean_data();

                Amara::Action::prepare();
            }
        }

        virtual void act(double deltaTime) override {
            Amara::Action::act(deltaTime);
            
            if (hasStarted) {
                progress += deltaTime/tween_duration;
                if (progress >= 1) {
                    progress = 1;
                }

                if (lua_actor_table.valid()) {
                    for (auto it = target_data.begin(); it != target_data.end(); ++it) {
                        if (it.value().is_number()) {
                            lua_actor_table.set(it.key(), ease((double)start_data[it.key()], (double)target_data[it.key()], progress, easing));
                        }
                        else if (string_equal(it.key(), "color") || string_equal(it.key(), "tint")) {
                            Amara::Color start_color = start_data[it.key()];
                            Amara::Color target_color = target_data[it.key()];
                            lua_actor_table.set(it.key(), ease(start_color, target_color, progress, easing));
                        }
                    }
                }

                if (onUpdate.valid()) {
                    try {
                        sol::protected_function_result result = onUpdate(actor, deltaTime, get_lua_object());
                        if (!result.valid()) {
                            sol::error err = result;
                            throw std::runtime_error("Lua Error: " + std::string(err.what()));  
                        }
                    } catch (const std::exception& e) {
                        debug_log(e.what());
                        gameProps->breakWorld();
                    }
                }

                if (progress == 1) {
                    if (waitingYoyo) {
                        progress = 0;
                        waitingYoyo = false;

                        nlohmann::json rec_target = target_data;
                        target_data = start_data;
                        start_data = rec_target;
                    }
                    else if (repeats != 0) {
                        repeats -= 1;
                        progress = 0;
                        if (yoyo) {
                            nlohmann::json rec_target = target_data;
                            target_data = start_data;
                            start_data = rec_target;
                        }
                        waitingYoyo = yoyo;
                    }
                    else complete();
                }
            }
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Tween>("Tween",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
                "progress", sol::readonly(&Tween::progress),
                "duration", sol::readonly(&Tween::tween_duration),
                "from", &Tween::from,
                "to", &Tween::to,
                "set", &Tween::from
            );

            sol::usertype<Amara::Node> node_type = lua["Node"];
            node_type["tween"] = sol::property([](Amara::Node& e) -> sol::object {
                Amara::Tween* tween = e.createChild("Tween")->as<Amara::Tween*>();
                return tween->get_lua_object();
            });
        }
    };
}