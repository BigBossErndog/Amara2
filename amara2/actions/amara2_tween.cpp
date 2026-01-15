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
        
        bool start_delay = false;
        double delay = 0;
        double interim = 0;

        Tween(): Amara::Action() {
            set_base_node_id("Tween");
        }

        sol::object from(sol::table lua_data) {
            if (!target_data.is_null()) {
                Amara::Tween* tween = createChild("Tween")->as<Amara::Tween*>();
                return tween->from(lua_data);
            }

            if (lua_data["target"].valid()) {
                sol::object otarget = lua_data["target"];
                lua_actor_table = otarget.as<sol::table>();
                lua_data["target"] = sol::nil;
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

            if (lua_data["target"].valid()) {
                sol::object otarget = lua_data["target"];
                lua_actor_table = otarget.as<sol::table>();
                lua_data["target"] = sol::nil;
            }
            if (lua_data["onComplete"].valid()) {
                funcs.setFunction(nodeID, "onComplete", lua_data["onComplete"]);
                lua_data["onComplete"] = sol::nil;
            }
            if (lua_data["onStart"].valid()) {
                funcs.setFunction(nodeID, "onStart", lua_data["onStart"]);
                lua_data["onStart"] = sol::nil;
            }
            if (lua_data["onProgress"].valid()) {
                funcs.setFunction(nodeID, "onProgress", lua_data["onProgress"]);
                lua_data["onProgress"] = sol::nil;
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
                lua_data["yoyo"] = sol::nil;
            }
            if (lua_data["repeats"].valid()) {
                repeats = lua_data["repeats"].get<int>();
                lua_data["repeats"] = sol::nil;
            }
            if (lua_data["delay"].valid()) {
                delay = lua_data["delay"].get<double>();
                lua_data["delay"] = sol::nil;
                start_delay = true;
            }
            if (lua_data["interim"].valid()) {
                interim = lua_data["interim"].get<double>();
                lua_data["interim"] = sol::nil;
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
                if (!json_has(target_data, it.key()) || !(it.value().is_number() || it.value().is_object() || it.value().is_boolean())) {
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
                }
                if (lua_actor_table.valid()) {
                    nlohmann::json start_prop_data = json_extract(start_data, "props");
                    for (auto it = start_data.begin(); it != start_data.end(); ++it) {
                        lua_actor_table.set(it.key(), json_to_lua(gameProps->lua, it.value()));
                    }
                    if (!start_prop_data.is_null()) {
                        start_data["props"] = start_prop_data;
                        sol::table prop_table = lua_actor_table["props"];
                        for (auto it = start_prop_data.begin(); it != start_prop_data.end(); ++it) {
                            prop_table.set(it.key(), json_to_lua(gameProps->lua, it.value()));
                        }
                    }

                    nlohmann::json prop_data = json_extract(target_data, "props");
                    for (auto it = target_data.begin(); it != target_data.end(); ++it) {
                        if (!json_has(start_data, it.key())) start_data[it.key()] = lua_to_json(lua_actor_table[it.key()]);
                    }
                    if (!prop_data.is_null()) {
                        target_data["props"] = prop_data;
                        if (!json_has(start_data, "props")) start_data["props"] = nlohmann::json::object();
                        nlohmann::json& start_prop_data = start_data["props"];
                        sol::table prop_table = lua_actor_table["props"];
                        for (auto it = prop_data.begin(); it != prop_data.end(); ++it) {
                            if (!json_has(start_prop_data, it.key())) start_prop_data[it.key()] = lua_to_json(prop_table[it.key()]);
                        }
                    }
                }
                clean_data();

                Amara::Action::prepare();

                if (funcs.hasFunction("onStart")) funcs.callFunction(actor, "onStart", get_lua_object());
            }
        }

        void completeProperties() {
            if (lua_actor_table.valid()) {
                double end_progress = (yoyo) ? 0 : 1;

                for (auto it = target_data.begin(); it != target_data.end(); ++it) {
                    tweenValue(lua_actor_table, it.key(), start_data[it.key()], target_data[it.key()], end_progress);
                }
            }
        }

        void tweenValue(sol::table target, const std::string& key,  const nlohmann::json& val1, const nlohmann::json& val2, double progress) {
            if (target[key].is<int>()) {
                target.set(key, std::round(ease((double)val1, (double)val2, progress, easing)));
            }
            else if (target[key].is<double>()) {
                target.set(key, ease((double)val1, (double)val2, progress, easing));
            }
            else if (target[key].is<Amara::Vector2>()) {
                Amara::Vector2 start_vec = val1;
                Amara::Vector2 target_vec = val2;
                target.set(key, Vector2(
                    ease(start_vec.x, target_vec.x, progress, easing),
                    ease(start_vec.y, target_vec.y, progress, easing)
                ));
            }
            else if (target[key].is<Amara::Rectangle>()) {
                Amara::Rectangle start_rect = val1;
                Amara::Rectangle target_rect = val2;
                target.set(key, Rectangle(
                    ease(start_rect.x, target_rect.x, progress, easing),
                    ease(start_rect.y, target_rect.y, progress, easing),
                    ease(start_rect.w, target_rect.w, progress, easing),
                    ease(start_rect.h, target_rect.h, progress, easing)
                ));
            }
            else if (target[key].is<Amara::Color>()) {
                Amara::Color start_color = val1;
                Amara::Color target_color = val2;
                target.set(key, ease(start_color, target_color, progress, easing));
            }
            else if ((String::equal(key, "props") || String::equal(key, "get")) && target[key].is<sol::table>()) {
                for (auto it = val1.begin(); it != val1.end(); ++it) {
                    tweenValue(target[key], it.key(), val1[it.key()], val2[it.key()], progress);
                }
            }
        }

        void finishTween() {
            completeProperties();
            complete();
        }

        virtual void act(double deltaTime) override {
            Amara::Action::act(deltaTime);
            
            if (has_started) {
                if (delay > 0) {
                    delay -= deltaTime;
                    if (delay <= 0 && start_delay) {
                        start_delay = false;
                        if (funcs.hasFunction("onStart")) {
                            funcs.callFunction(actor, "onStart", get_lua_object());
                        }
                    }
                }
                else {
                    progress += deltaTime/tween_duration;
                    if (progress >= 1) {
                        if (waitingYoyo) {
                            progress -= 1;
                            waitingYoyo = false;
                        }
                        else if (repeats != 0) {
                            repeats -= 1;
                            progress -= 1;
                            waitingYoyo = yoyo;
                        }
                        else {
                            completeProperties();
                            double real_progress = (yoyo && !waitingYoyo) ? 1 - progress : progress;
                            if (funcs.hasFunction("onProgress")) funcs.callFunction(actor, "onProgress", get_lua_object(), real_progress, deltaTime);
                            
                            complete();
                        }
                        if (interim > 0) {
                            delay = interim;
                        }
                    }

                    if (!completed) {
                        double real_progress = (yoyo && !waitingYoyo) ? 1 - progress : progress;
                        real_progress = std::clamp(real_progress, 0.0, 1.0);
                        if (lua_actor_table.valid()) {
                            for (auto it = target_data.begin(); it != target_data.end(); ++it) {
                                tweenValue(lua_actor_table, it.key(), start_data[it.key()], target_data[it.key()], real_progress);
                            }
                        }

                        if (funcs.hasFunction("onProgress")) funcs.callFunction(actor, "onProgress", get_lua_object(),real_progress, deltaTime);
                    }
                }
            }
        }
        
        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::Tween>("Tween",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
                "progress", sol::readonly(&Tween::progress),
                "duration", sol::readonly(&Tween::tween_duration),
                "interim", &Tween::interim,
                "delay", &Tween::delay,
                "repeats", &Tween::repeats,
                "from", &Tween::from,
                "to", &Tween::to,
                "set", &Tween::from,
                "finishTween", &Tween::finishTween
            );

            sol::usertype<Amara::Node> node_type = lua["Node"];
            node_type["tween"] = sol::property([](Amara::Node& e) -> sol::object {
                Amara::Tween* tween = e.createChild("Tween")->as<Amara::Tween*>();
                return tween->get_lua_object();
            });
        }
    };
}