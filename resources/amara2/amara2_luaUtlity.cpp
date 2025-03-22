namespace Amara {
    class Entity;

    sol::object json_to_lua(nlohmann::json json) {
        sol::state& lua = Props::lua();
        if (json.is_null()) {
            return sol::make_object(lua, sol::nil);
        } else if (json.is_boolean()) {
            return sol::make_object(lua, json.get<bool>());
        } else if (json.is_number()) {
            return sol::make_object(lua, json.get<double>());
        } else if (json.is_string()) {
            return sol::make_object(lua, json.get<std::string>());
        } else if (json.is_array()) {
            sol::table arr = lua.create_table();
            for (size_t i = 0; i < json.size(); ++i) {
                arr[i + 1] = json_to_lua(json[i]);
            }
            return sol::make_object(lua, arr);
        } else if (json.is_object()) {
            sol::table tbl = lua.create_table();
            for (auto& item : json.items()) {
                std::string key = item.key();
                tbl[key] = json_to_lua(item.value());
            }
            return sol::make_object(lua, tbl);
        }

        return sol::make_object(lua, sol::nil);
    }

    nlohmann::json lua_to_json(sol::object obj) {
        if (!obj.valid() || obj.get_type() == sol::type::nil) {
            return nullptr;
        } else if (obj.get_type() == sol::type::boolean) {
            return obj.as<bool>();
        } else if (obj.get_type() == sol::type::number) {
            return obj.as<double>();
        } else if (obj.get_type() == sol::type::string) {
            return obj.as<std::string>();
        } else if (obj.get_type() == sol::type::table) {
            sol::table tbl = obj.as<sol::table>();
            nlohmann::json json;
    
            bool isArray = true;
            size_t index = 1;
            for (auto& pair : tbl) {
                sol::object key = pair.first;
                sol::object value = pair.second;
    
                if (key.get_type() == sol::type::number && key.as<size_t>() == index) {
                    json.push_back(lua_to_json(value));
                    ++index;
                } else {
                    isArray = false;
                    break;
                }
            }
    
            if (isArray) {
                return json;
            } else {
                for (auto& pair : tbl) {
                    sol::object key = pair.first;
                    sol::object value = pair.second;
    
                    if (key.get_type() == sol::type::string) {
                        json[key.as<std::string>()] = lua_to_json(value);
                    }
                }
                return json;
            }
        }
    
        return nullptr;
    }
    
    std::string entity_to_string(sol::object);
    std::string lua_to_string(sol::object obj) {
        if (obj.is<int>()) return std::to_string(obj.as<int>());
        if (obj.is<double>()) {
            double val = obj.as<double>();
            if (floor(val) == val) return std::to_string((int)val);
            else return std::to_string(val);
        }
        if (obj.is<sol::nil_t>()) return "nil";
        if (obj.is<std::string>()) return obj.as<std::string>();
        if (is_entity(obj)) return entity_to_string(obj);
        if (obj.is<Vector3>()) return std::string(obj.as<Vector3>());
        if (obj.is<Vector2>()) return std::string(obj.as<Vector2>());

        nlohmann::json j = lua_to_json(obj);
        return j.dump();
    }

    template <typename T>
    sol::table vector_to_lua(const std::vector<T>& vec) {
        sol::state& lua = Props::lua();
        sol::table lua_table = lua.create_table();
    
        for (size_t i = 0; i < vec.size(); ++i) {
            lua_table[i + 1] = vec[i];
        }
    
        return lua_table;
    }

    std::string lua_string_concat(sol::variadic_args args) {
        std::ostringstream ss;
        for (auto arg : args) {
            ss << lua_to_string(arg);
        }
        return ss.str();
    }

    std::string lua_string_sep_concat(const std::string& separator, sol::variadic_args args) {
        std::ostringstream ss;
        bool first = true;
        
        for (auto arg : args) {
            ss << (first ? "" : separator) << lua_to_string(arg);
            first = false;
        }
    
        return ss.str();
    }
    
    template<typename... Args>
    void debug_log(Args... args) {
        std::ostringstream ss;
        (ss << ... << args);
        std::cout << ss.str().c_str() << std::endl;
    }
    void lua_debug_log(sol::variadic_args args) {
        std::ostringstream ss;
        for (auto arg : args) {
            ss << lua_to_string(arg);
        }
        debug_log(ss.str());
    }
    std::string c_style_log(const char* format, ...) {
        va_list args;
        va_start(args, format);

        int size = vsnprintf(nullptr, 0, format, args);
        va_end(args);
        
        if (size <= 0) return "";

        std::vector<char> buffer(size + 1);
    
        va_start(args, format);
        vsnprintf(buffer.data(), buffer.size(), format, args);
        va_end(args);
        
        debug_log(std::string(buffer.data()));
    }

    void bindLua_LuaUtilityFunctions(sol::state& lua) {
        lua.set_function("debug_log", &Amara::lua_debug_log);
        lua.set_function("print", [](sol::variadic_args args) {
            debug_log(lua_string_sep_concat(" ", args));
        });

        lua.set_function("object_to_string", &Amara::lua_to_string);
        
        sol::table string_metatable = lua["string"];
        string_metatable.set_function("starts_with", [](std::string self, std::string check) -> bool {
            return string_startsWith(self, check);
        });
        string_metatable.set_function("ends_with", [](std::string self, std::string check) -> bool {
            return string_endsWith(self, check);
        });
        string_metatable.set_function("concat", &Amara::lua_string_concat);
        string_metatable.set_function("sep_concat", &Amara::lua_string_sep_concat);

        sol::table math_metatable = lua["math"];
        math_metatable.set_function("round", [](double num) -> int {
            return std::round(num);
        });

        sol::table table_metatable = lua["table"];
    }
}
