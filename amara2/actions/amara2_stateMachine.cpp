namespace Amara {
    class Script;

    struct StateRecord {
        std::string name;
        int event = 0;
        std::string jumpFlag;
    };

    class StateMachine: public Amara::Action {
    public:
        std::string currentState;
        std::string lastState;

        std::vector<StateRecord> stateRecords;

        int currentEvent = 1;
        int eventLooker = 0;

        double waitCounter = 0;
        double waitTimeElapsed = 0;
        
        bool skipEvent = false;
        bool debug = false;

        std::string jumpFlag;

        std::unordered_map<std::string, sol::function> state_map;

        StateMachine(): Amara::Action() {
            set_base_node_id("StateMachine");
            reset();
        }

        virtual void act(double deltaTime) override {
            Amara::Action::act(deltaTime);

            if (hasStarted) {
                if (state_map.find(currentState) != state_map.end()) {
                    sol::protected_function func = state_map[currentState];
                    eventLooker = 0;
                    try {
                        sol::protected_function_result result = func(actor->get_lua_object(), get_lua_object(), deltaTime);
                        if (!result.valid()) {
                            sol::error err = result;
                            throw std::runtime_error(std::string(err.what()));  
                        }
                    } catch (const std::exception& e) {
                        debug_log(e.what());
                        gameProps->breakWorld();
                    }
                }
            }
        }

        sol::object addState(std::string key, sol::function func) {
            state_map[key] = func;
            return get_lua_object();
        }

        void clearStateRecord() {
            stateRecords.clear();
        }

        void reset() {
            currentState.clear();
            lastState.clear();
            currentEvent = 1;
            jumpFlag.clear();
            stateRecords.clear();
        }

        bool resetEvent() {
            if (event()) {
                reset();
                return true;
            }
            return false;
        }

        bool inState(std::string key) {
            if (currentState.compare(key) == 0) {
                return true;
            }
            return false;
        }

        void start(std::string key) {
            switchState(key);
        }

        bool start() {
            if (currentState.empty()) {
                eventLooker = 0;
                return true;
            }
            return false;
        }

        bool state(std::string key) {
            if (currentState.empty()) {
                currentState = key;
            }
            if (String::equal(key, currentState)) {
                eventLooker = 0;
                return true;
            }
            return false;
        }

        void switchState(std::string key) {
            if (!currentState.empty()) {
                lastState = currentState;
                Amara::StateRecord record = { currentState, currentEvent, jumpFlag };
                stateRecords.push_back(record);
            }

            currentState = key;
            currentEvent = 1;

            if (debug) {
                debug_log(*this, ": switch state \"", key, "\".");
            }
        }

        bool switchStateEvent(std::string key) {
            if (once()) {
                switchState(key);
                return true;
            }
            return false;
        }

        void returnState() {
            if (stateRecords.empty()) {
                reset();
            }
            else {
                lastState = currentState;
                
                Amara::StateRecord record = stateRecords.back();
                currentState = record.name;
                currentEvent = record.event;
                jumpFlag = record.jumpFlag;
                stateRecords.pop_back();

                if (debug) {
                    debug_log(*this, ": return state \"", currentState, "\".");
                }
            }
        }

        bool returnStateEvent() {
            if (event()) {
                returnState();
                return true;
            }
            return false;
        }
        
        void restartState() {
            currentEvent = 1;
            if (debug) {
                debug_log(*this, ": restart state \"", currentState, "\".");
            }
        }

        bool restartStateEvent() {
            if (event()) {
                restartState();
                return true;
            }
            return false;
        }

        bool event() {
            eventLooker += 1;
            if (currentEvent == eventLooker) {
                if (skipEvent) {
                    skipEvent = false;
                    return false;
                }

                return true;
            }
            return false;
        }

        bool once() {
            if (event()) {
                nextEvent();
                return true;
            }
            return false;
        }

        void nextEvent() {
            currentEvent += 1;
            skipEvent = true;
        }
        
        bool nextEventOn(bool cond) {
            if (cond) {
                nextEvent();
                return true;
            }
            return false;
        }

        bool wait(double time, bool skip) {
            bool ret = false;

            if (once()) {
                waitCounter = 0;
                waitTimeElapsed = 0;
                ret = true;
            }

            if (event()) {
                double t = time / gameProps->deltaTime;

                waitCounter += 1;
                if (waitCounter >= t || skip) {
                    waitCounter = 0;
                    nextEvent();
                }

                ret = true;
            }

            return ret;
        }
        bool wait(float time) {
            return wait(time, false);
        }

        bool waitUntil(bool condition) {
            if (event()) {
                if (condition) nextEvent();
                return true;
            }
            return false;
        }

        bool repeat(int num) {
            bool ret = false;
            for (int i = 0; i < num; i++) {
                if (once()) {
                    ret = true;
                }
            }
            return ret;
        }

        bool bookmark(std::string flag) {
            bool toReturn = false;

            if (once()) {}
            else {
                if (jumpFlag.compare(flag) == 0) {
                    jumpFlag.clear();
                    currentEvent = eventLooker;
                    nextEvent();
                    toReturn = true;
                }
            }

            return toReturn;
        }

        void jump(std::string flag) {
            jumpFlag = flag;
        }

        bool jumpEvent(std::string flag) {
            if (event()) {
                jump(flag);
                return true;
            }
            return false;
        }

        virtual sol::object complete() override {
            if (parent && parent->stateMachine == this) {
                parent->stateMachine = nullptr;
            }
            if (actor && actor->stateMachine == this) {
                actor->stateMachine = nullptr;
            }

            return Amara::Action::complete();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<StateMachine>("StateMachine",
                sol::constructors<StateMachine()>(),
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>(),
                "start", sol::overload(
                    sol::resolve<void(std::string)>(&StateMachine::start),
                    sol::resolve<bool()>(&StateMachine::start)
                ),
                "event", &StateMachine::event,
                "once", &StateMachine::once,
                "nextEvent", &StateMachine::nextEvent,
                "nextEventOn", &StateMachine::nextEventOn,
                "wait", sol::overload(
                    sol::resolve<bool(double, bool)>(&StateMachine::wait),
                    sol::resolve<bool(float)>(&StateMachine::wait)
                ),
                "waitUntil", &StateMachine::waitUntil,
                "repeat", &StateMachine::repeat,
                "bookmark", &StateMachine::bookmark,
                "jump", &StateMachine::jump,
                "jumpEvent", &StateMachine::jumpEvent,
                "switchState", &StateMachine::switchState,
                "switchStateEvent", &StateMachine::switchStateEvent,
                "returnState", &StateMachine::returnState,
                "returnStateEvent", &StateMachine::returnStateEvent,
                "restartState", &StateMachine::restartState,
                "restartStateEvent", &StateMachine::restartStateEvent,
                "addState", &StateMachine::addState,
                "reset", &StateMachine::reset,
                "resetEvent", &StateMachine::resetEvent,
                "inState", &StateMachine::inState,
                "state", &StateMachine::state
            );

            sol::usertype<Node> node_type = lua["Node"];
            node_type["state"] = sol::property([](Amara::Node& node) -> sol::object {
                if (node.stateMachine == nullptr) {
                    node.stateMachine = node.createChild("StateMachine")->as<Amara::StateMachine*>();
                    node.stateMachine->is_action = false;
                }
                return node.stateMachine->get_lua_object();
            });
        }
    };
}