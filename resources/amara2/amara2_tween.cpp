namespace Amara {
    class EntityTween: public Amara::Action {
    public:
        nlohmann::json startData = nullptr;
        nlohmann::json targetData = nullptr;

        sol::object lua_actor;

        float duration = 0;
        double progress = 0;

        Amara::Ease easing = Amara::Ease::Linear;

        EntityTween(): Amara::Action() {
            baseEntityID = "EntityTween";
        }

        virtual void init() override {
            Amara::Action::init();
            lua_actor = actor->get_lua_object();
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
            if (startData.is_null()) {
                clean_start_data();
                actor->configure(startData);
            }
            else {
                startData = actor->toJSON();
                clean_start_data();
            }

            Amara::Action::prepare();
        }

        virtual void act(double deltaTime) override {
            progress += deltaTime;
            if (progress >= 1) {
                progress = 1;
                finish();
            }
            for (auto it = targetData.begin(); it != targetData.end(); ++it) {
                actor->configure(it.key(), ease(startData[it.key()], targetData[it.key()], progress, easing));
            }
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<EntityTween>("EntityTween",
                sol::base_classes, sol::bases<Amara::Entity, Amara::Action>(),
                "from", &EntityTween::from,
                "to", &EntityTween::to
            );
        }
    };
}