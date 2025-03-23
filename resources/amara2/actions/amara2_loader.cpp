namespace Amara {
    struct LoadTask {
        AssetEnum type = AssetEnum::None;
        std::string key;
        std::string path;
        bool replace = false;

        int failAttempts = 0;
    };

    class Loader: public Action {
    public:
        std::vector<LoadTask> tasks;

        int loadRate = -1;

        bool processTask(const LoadTask& task) {
            switch (task.type) {
                case AssetEnum::Image:
                    break;
            }
            return true;
        }

        virtual void act(double deltaTime) override {
            Amara::Action::act(deltaTime);

            if (hasStarted) {
                int processedTasks = 0;

                for (auto it = tasks.begin(); it != tasks.end();) {
                    if (processTask(*it)) {
                        ++it;
                    }
                    else {
                        it = tasks.erase(it);
                    }

                    processedTasks += 1;
                    if (loadRate > 0 && processedTasks >= loadRate) {
                        break;
                    }
                }

                if (tasks.size() == 0) complete();
            }
        }
        
        sol::object queueTask(const LoadTask& task) {
            if (loadRate > 0) tasks.push_back(task);
            else processTask(task);
            return get_lua_object();
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Loader>("Loader"
                
            );
        }
    };
}