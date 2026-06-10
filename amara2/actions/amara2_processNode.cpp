namespace Amara {
    class ProcessNode: public Amara::Action {
    public:
        SDL_Process* process = nullptr;
        SDL_IOStream* io = nullptr;
        
        char buffer[4096] = {0};
        std::string partial_line;
        std::string delimiter = "\n";
        
        std::vector<nlohmann::json> output;
        std::vector<std::string> args;

        bool finished = false;
        int exitCode = 0;
        
        bool change_environment = false;
        std::string environment_path;
        
        ProcessNode(): Amara::Action() {
            set_base_node_id("ProcessNode");
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "delimiter")) {
                delimiter = json_get<std::string>(config, "delimiter");
            }
            if (json_has(config, "arguments")) {
                nlohmann::json arg_config = config["arguments"];

                if (arg_config.is_array()) {
                    for (auto& arg : arg_config) {
                        if (arg.is_string()) args.push_back(arg.get<std::string>());
                        else args.push_back(arg.dump());
                    }
                }
                else {
                    if (arg_config.is_string()) args.push_back(arg_config.get<std::string>());
                    else args.push_back(arg_config.dump());
                }
            }
            if (json_has(config, "environment")) {
                change_environment = true;
                environment_path = json_get<std::string>(config, "environment");
            }
            return Amara::Node::configure(config);
        }

        virtual void prepare() override {
            Amara::Action::prepare();

            std::vector<const char*> c_args;
            for (const auto& arg : args) {
                c_args.push_back(arg.c_str());
            }
            c_args.push_back(nullptr);

            if (change_environment) {
                SDL_PropertiesID props = SDL_CreateProperties();
                SDL_SetPointerProperty(props, SDL_PROP_PROCESS_CREATE_ARGS_POINTER, c_args.data());
                SDL_SetStringProperty(props, SDL_PROP_PROCESS_CREATE_WORKING_DIRECTORY_STRING, environment_path.c_str());
                
                SDL_SetNumberProperty(props, SDL_PROP_PROCESS_CREATE_STDIN_NUMBER, SDL_PROCESS_STDIO_NULL);
                
                SDL_SetNumberProperty(props, SDL_PROP_PROCESS_CREATE_STDOUT_NUMBER, SDL_PROCESS_STDIO_APP);
                SDL_SetNumberProperty(props, SDL_PROP_PROCESS_CREATE_STDERR_NUMBER, SDL_PROCESS_STDIO_APP);
                
                process = SDL_CreateProcessWithProperties(props);
                SDL_DestroyProperties(props);
            }
            else {
                process = SDL_CreateProcess(c_args.data(), true);
            }
            
            if (process) {
                io = SDL_GetProcessOutput(process);
            }
            else {
                Amara::debug_log("Error: Failed to create process.");
                Amara::debug_log("Arguments: ", nlohmann::json(args));
                destroy();
                if (funcs.hasFunction("onExit")) {
                    std::string msg = String::concat("Error: ", SDL_GetError());
                    funcs.callFunction(this, "onExit", -1, msg);
                }
            }
        }

        virtual void act(double deltaTime) override {
            Amara::Action::act(deltaTime);
            
            if (has_started && process) {
                bool found_output = false;

                if (io) {
                    Sint64 available = SDL_GetIOSize(io);
                    if (available > 0) {
                        size_t bytes_to_read = std::min((size_t)available, sizeof(buffer) - 1);
                        size_t bytes_read = SDL_ReadIO(io, buffer, bytes_to_read);
                        
                        if (bytes_read > 0) {
                            buffer[bytes_read] = '\0';
                            partial_line.append(buffer, bytes_read);

                            if (delimiter.empty()) {
                                logOutput(partial_line);
                                partial_line.clear();
                            } else {
                                size_t delim_pos;
                                while ((delim_pos = partial_line.find(delimiter)) != std::string::npos) {
                                    logOutput(partial_line.substr(0, delim_pos));
                                    partial_line.erase(0, delim_pos + delimiter.length());
                                }
                            }
                            found_output = true;
                        }
                    }
                }

                if (!found_output && SDL_WaitProcess(process, false, &exitCode)) {
                    finished = true;
                    if (!partial_line.empty()) {
                        logOutput(partial_line);
                        partial_line.clear();
                    }
                    io = nullptr;

                    SDL_DestroyProcess(process);
                    process = nullptr;

                    complete();

                    if (funcs.hasFunction("onExit")) {
                        funcs.callFunction(this, "onExit", exitCode);
                    }
                }
            }
            else if (has_started && !process) {
                complete();
            }
        }
        
        void logOutput(std::string msg) {
            output.push_back(msg);
            Amara::debug_log("Received: ", msg);
            if (funcs.hasFunction("onOutput")) {
                funcs.callFunction(this, "onOutput", msg);
            }
        }

        virtual void destroy() override {
            if (process) {
                SDL_KillProcess(process, false);
                SDL_DestroyProcess(process);
                process = nullptr;
            }
            io = nullptr;
            if (!partial_line.empty()) {
                output.push_back(partial_line);
                partial_line.clear();
            }
            Amara::Node::destroy();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::ProcessNode>("ProcessNode",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
                "output", sol::property([](const ProcessNode& p) -> sol::object {
                    if (p.output.size() == 0) return sol::nil;
                    return Amara::json_to_lua(p.gameProps->lua, p.output);
                }),
                "delimiter", &ProcessNode::delimiter,
                "finished", sol::readonly(&ProcessNode::finished),
                "exitCode", sol::readonly(&ProcessNode::exitCode)
            );
        }
    };
}