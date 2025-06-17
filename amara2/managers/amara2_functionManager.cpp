namespace Amara {
    class Node;

    class FunctionManager {
    public:
        Amara::GameProps* gameProps = nullptr;
        Amara::Node* node = nullptr;

        std::unordered_map<std::string, sol::table> funcTable;
        std::string lastClass;

        std::unordered_map<std::string, std::string> inheritance_map;
        std::string lastRegisteredClass;

        FunctionManager() = default;

        void init(Amara::GameProps* _gameProps, Amara::Node* _node) {
            gameProps = _gameProps;
            node = _node;
        }

        void registerClass(std::string className) {
            inheritance_map[className] = lastRegisteredClass;
            lastRegisteredClass = className;
        }

        sol::object get_node_lua_object();

        sol::function _create_wrapped_function(sol::function original_callback) {
            return sol::make_object(this->gameProps->lua, [this, original_callback](sol::variadic_args va) -> sol::object {
                std::vector<sol::object> actual_args_vector;
                if (va.size() > 0) {
                    for (auto it = va.begin() + 1; it != va.end(); ++it) {
                        actual_args_vector.push_back(*it);
                    }
                }
                return original_callback(this->get_node_lua_object(), sol::as_args(actual_args_vector));
            });
        }

        void setFunction(std::string className, std::string funcName, sol::function func) {
            if (funcTable.find(className) == funcTable.end()) createTable(className);
            funcTable[className][funcName] = _create_wrapped_function(func);
        }

        void createTable(std::string className) {
            sol::table new_class_tbl = gameProps->lua.create_table();
            
            if (!lastClass.empty() && funcTable.find(lastClass) != funcTable.end()) {
                sol::table last_class_tbl = funcTable[lastClass];
                for (const auto& pair : last_class_tbl) {
                    if (pair.second.is<sol::function>()) {
                        new_class_tbl[pair.first] = pair.second;
                    }
                }
                new_class_tbl["super"] = last_class_tbl;
            }

            sol::table meta = gameProps->lua.create_table();

            meta["__newindex"] = [this](sol::table tbl, sol::object key, sol::object value) {
                if (value.is<sol::function>()) {
                    sol::function callback = value.as<sol::function>();
                    sol::function func = sol::make_object(this->gameProps->lua, [this, key, callback](sol::variadic_args va)->sol::object {
                        std::vector<sol::object> remaining_args_vector;

                        if (va.size() > 0) {
                            for (auto it = va.begin() + 1; it != va.end(); ++it) {
                                remaining_args_vector.push_back(*it);
                            }
                        }

                        return callback(this->get_node_lua_object(), sol::as_args(remaining_args_vector));
                    });
                    tbl.raw_set(key, func);
                }
                else debug_log("Error: Cannot set non-functions to FunctionManager.");
            };

            new_class_tbl[sol::metatable_key] = meta;
            funcTable[className] = new_class_tbl;
            lastClass = className;
        }

        bool hasTable(std::string className) {
            return funcTable.find(className) != funcTable.end();
        }

        sol::object getClassTable(std::string className) {
            if (funcTable.find(className) != funcTable.end()) {
                return funcTable[className];
            }
            if (inheritance_map.find(className) != inheritance_map.end()) {
                return getClassTable(inheritance_map[className]);
            }
            return sol::nil;
        }

        template<typename... CallArgs>
        sol::function getFunction(std::string className, std::string funcName) {
            auto class_iter = funcTable.find(className);
            if (class_iter != funcTable.end()) {
                sol::table class_tbl = class_iter->second;
                sol::object func_obj = class_tbl[funcName];

                if (func_obj.is<sol::function>()) {
                    return func_obj;
                } 
            }
            else if (inheritance_map.find(className) != inheritance_map.end()) {
                return getFunction(inheritance_map[className], funcName);
            }
            return sol::nil;
        }

        template<typename... CallArgs>
        sol::object callFunction(std::string className, std::string funcName, CallArgs&&... args) {
            auto class_iter = funcTable.find(className);
            if (class_iter != funcTable.end()) {
                sol::table class_tbl = class_iter->second;
                sol::object func_obj = class_tbl[funcName];

                if (func_obj.is<sol::function>()) {
                    sol::function func = func_obj.as<sol::function>();
                    try {
                        sol::protected_function_result result = func.call(std::forward<CallArgs>(args)...);
                        if (result.valid()) {
                            return result;
                        }
                        else {
                            sol::error err = result;
                            throw std::runtime_error("Lua Error: " + std::string(err.what()));
                        }
                    }
                    catch (const sol::error& e) {
                        debug_log(e.what());
                        gameProps->breakWorld();
                    }
                } 
                else debug_log("Error: Class ", className, " does not have function table.");
            }
            else if (inheritance_map.find(className) != inheritance_map.end()) {
                return callFunction(inheritance_map[className], funcName, std::forward<CallArgs>(args)...);
            }
            else debug_log("Error: Class ", className, " does not have function table.");
            return sol::nil;
        }

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
                }
            );
        }
    };
}