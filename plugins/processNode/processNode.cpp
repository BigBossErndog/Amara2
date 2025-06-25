class ProcessNode: public Amara::Action {
public:
    SDL_Process* process = nullptr;
    SDL_IOStream* io = nullptr;
    
    char buffer[256] = {0};
    std::string partial_line;
    
    std::vector<nlohmann::json> output;

    std::vector<std::string> args;

    bool finished = false;
    int exitCode = 0;
    
    ProcessNode(): Amara::Action() {
        set_base_node_id("ProcessNode");
    }

    virtual Amara::Node* configure(nlohmann::json config) override {
        if (Amara::json_has(config, "arguments")) {
            nlohmann::json arg_config = config["arguments"];

            if (arg_config.is_array()) {
                for (auto& arg : arg_config) {
                    args.push_back(arg.get<std::string>());
                }
            }
            else {
                args.push_back(arg_config.get<std::string>());
            }
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

        process = SDL_CreateProcess(c_args.data(), true);
        
        if (process) {
            io = SDL_GetProcessOutput(process);
        }
        else {
            Amara::debug_log("Error: Failed to creeate process.");
            Amara::debug_log("Arguments: ", nlohmann::json(args));
        }
    }

    virtual void act(double deltaTime) override {
        Amara::Action::act(deltaTime);

        if (hasStarted) {
            if (io) {
                size_t bytes_read = 0;
                do {
                    bytes_read = SDL_ReadIO(io, buffer, sizeof(buffer) - 1);
                    if (bytes_read > 0) {
                        buffer[bytes_read] = '\0';
                        partial_line.append(buffer, bytes_read);

                        size_t newline_pos;
                        while ((newline_pos = partial_line.find('\n')) != std::string::npos) {
                            logOutput(partial_line.substr(0, newline_pos));
                            partial_line.erase(0, newline_pos + 1);
                        }
                    }
                } while (bytes_read > 0);

                if (SDL_WaitProcess(process, false, &exitCode)) {
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
            else {
                complete();
            }
        }
    }

    void logOutput(std::string msg) {
        output.push_back(msg);

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
        if (!partial_line.empty()) {
            output.push_back(partial_line);
            partial_line.clear();
        }
        Amara::Node::destroy();
    }

    static void bind_lua(sol::state& lua) {
        lua.new_usertype<ProcessNode>("ProcessNode",
            sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
            "output", sol::property([](const ProcessNode& p) -> sol::object {
                if (p.output.size() == 0) return sol::nil;
                return Amara::json_to_lua(p.gameProps->lua, p.output);
            }),
            "finished", sol::readonly(&ProcessNode::finished),
            "exitCode", sol::readonly(&ProcessNode::exitCode)
        );
    }
};