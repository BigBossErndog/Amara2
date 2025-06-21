namespace Amara {
    class Node;

    class FunctionMap {
    public:
        GameProps* gameProps = nullptr;

        sol::table func_table;
        std::unordered_map<std::string, sol::protected_function> func_map;
        
        FunctionMap() = default;
        FunctionMap(GameProps* _gameProps) {
            gameProps = _gameProps;
        }

        void createTable() {
            func_table = gameProps->lua.create_table();
        }

        bool hasFunction(std::string funcName) {
            return func_map.find(funcName) != func_map.end();
        }

        void setFunction(std::string funcName, sol::function func, sol::function wrapped_func) {
            func_map[funcName] = func;
            func_table[funcName] = wrapped_func;
        }

        sol::function getFunction(std::string funcName) {
            return func_map[funcName];
        }

        template<typename... CallArgs>
        sol::object callFunction(Amara::Node* _node, std::string funcName, CallArgs&&... args) {
            try {
                sol::protected_function_result result = func_map[funcName](get_lua_object(_node), std::forward<CallArgs>(args)...);
                if (result.valid()) {
                    return result;
                }
                else {
                    sol::error err = result;
                    throw std::runtime_error("Lua Error: " + std::string(err.what()));
                }
            }
            catch (const std::exception& e) {
                debug_log(e.what());
                gameProps->breakWorld();
            }

            return sol::nil;
        }

        sol::object get_lua_object(Amara::Node* node);
    };
    
    class FunctionManager {
    public:
        Amara::GameProps* gameProps = nullptr;
        Amara::Node* owner_node = nullptr;
        Amara::Node* node = nullptr;

        std::unordered_map<std::string, FunctionMap> funcMap;
        std::string lastClass;

        std::unordered_map<std::string, std::string> inheritance_map;
        std::string lastRegisteredClass;

        FunctionManager() = default;

        void init(Amara::GameProps* _gameProps, Amara::Node* _node) {
            gameProps = _gameProps;
            node = _node;
            owner_node = _node;
        }

        void registerClass(std::string className) {
            if (String::equal(className, lastRegisteredClass)) return;
            inheritance_map[className] = lastRegisteredClass;
            lastRegisteredClass = className;
        }

        std::string owner_node_string();

        sol::function _create_wrapped_function(std::string class_name, std::string func_name) {
            return sol::make_object(this->gameProps->lua, [this, class_name, func_name](sol::variadic_args va) -> sol::object {
                std::vector<sol::object> remaining_args_vector;

                if (va.size() > 0) {
                    for (auto it = va.begin() + 1; it != va.end(); ++it) {
                        remaining_args_vector.push_back(*it);
                    }
                }
                return this->callFunction(class_name, func_name, sol::as_args(remaining_args_vector));
            });
        }

        void setFunction(std::string className, std::string funcName, sol::function func) {
            if (funcMap.find(className) == funcMap.end()) createTable(className);
            funcMap[className].setFunction(funcName, func, _create_wrapped_function(className, funcName));
        }

        void createTable(std::string className) {
            funcMap[className] = FunctionMap(gameProps);
            FunctionMap& new_map = funcMap[className];

            new_map.createTable();

            sol::table& new_class_tbl = funcMap[className].func_table;

            if (!lastClass.empty() && funcMap.find(lastClass) != funcMap.end()) {
                sol::table last_class_tbl = funcMap[lastClass].func_table;
                for (const auto& pair : last_class_tbl) {
                    if (pair.second.is<sol::function>()) {
                        new_class_tbl[pair.first] = pair.second;
                    }
                }
                new_class_tbl["func"] = last_class_tbl;
            }

            lastClass = className;
        }

        bool hasTable(std::string className) {
            return funcMap.find(className) != funcMap.end();
        }

        sol::object getClassTable(std::string className) {
            if (funcMap.find(className) != funcMap.end()) {
                return funcMap[className].func_table;
            }
            if (inheritance_map.find(className) != inheritance_map.end()) {
                return getClassTable(inheritance_map[className]);
            }
            return sol::nil;
        }

        bool hasFunction(std::string className, std::string funcName) {
            if (funcMap.find(className) != funcMap.end() && funcMap[className].hasFunction(funcName)) {
                return true;
            }
            if (inheritance_map.find(className) != inheritance_map.end()) {
                return hasFunction(inheritance_map[className], funcName);
            }
            return false;
        }
        bool hasFunction(std::string funcName) {
            return hasFunction(lastClass, funcName);
        }

        template<typename... CallArgs>
        sol::object callFunction(std::string className, std::string funcName, CallArgs&&... args) {
            if (funcMap.find(className) != funcMap.end()) {
                FunctionMap& found_map = funcMap[className];
                if (found_map.hasFunction(funcName)) {
                    return found_map.callFunction(node, funcName, std::forward<CallArgs>(args)...);
                }
            }
            if (inheritance_map.find(className) != inheritance_map.end()) {
                return callFunction(inheritance_map[className], funcName, std::forward<CallArgs>(args)...);
            }
            else debug_log("Error: ", owner_node_string(), " does not have the function \"", funcName, "\".");
            return sol::nil;
        }
        
        template<typename... CallArgs>
        sol::object callFunction(std::string funcName, CallArgs&&... args) {
            return callFunction(lastClass, funcName, std::forward<CallArgs>(args)...);
        }

        template<typename... CallArgs>
        sol::object callFunction(Amara::Node* _node, std::string funcName, CallArgs&&... args) {
            Amara::Node* rec = node;
            node = _node;
            sol::object ret = callFunction(lastClass, funcName, std::forward<CallArgs>(args)...);
            node = rec;
            return ret;
        }

        sol::object get_lua_object();

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<FunctionManager>("FunctionManager",
                sol::meta_function::index, [](FunctionManager& fm, sol::object key_obj) -> sol::object {
                    if (key_obj.is<std::string>()) {
                        std::string key_str = key_obj.as<std::string>();
                        sol::object class_table_obj = fm.getClassTable(key_str);
                        if (class_table_obj != sol::nil) {
                            return class_table_obj;
                        }
                    }
                    return sol::nil;
                },
                sol::meta_function::new_index, [](FunctionManager& fm, sol::object key_obj, sol::object value_obj) {
                    if (key_obj.is<std::string>()) {
                        std::string key_str = key_obj.as<std::string>();
                        if (value_obj.is<sol::function>()) {
                            sol::function func = value_obj.as<sol::function>();
                            fm.setFunction(fm.lastClass, key_str, func);
                        }
                    }
                    return value_obj;
                }
            );
        }
    };
}