namespace Amara {
    sol::object json_to_lua(nlohmann::json json) {
        sol::state& lua = Properties::lua();
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

    std::string lua_to_string(sol::object obj) {
        if (obj.is<std::string>()) return obj.as<std::string>();
        if (obj.is<Vector3>()) return std::string(obj.as<Vector3>());
        if (obj.is<Vector2>()) return std::string(obj.as<Vector2>());

        nlohmann::json j = lua_to_json(obj);
        return j.dump();
    }

    template <typename T>
    sol::table vector_to_lua(const std::vector<T>& vec) {
        sol::state& lua = Properties::lua();
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
    
    template<typename... Args>
    void log(Args... args) {
        std::ostringstream ss;
        (ss << ... << args);
        std::cout << ss.str().c_str() << std::endl;
    }
    void lua_log(sol::variadic_args args) {
        std::ostringstream ss;
        for (auto arg : args) {
            ss << lua_to_string(arg);
        }
        log(ss.str());
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
        
        log(std::string(buffer.data()));
    }

    void bindLuaUtilityFunctions(sol::state& lua) {
        lua.set_function("log", &Amara::lua_log);
        lua.set_function("object_to_string", &Amara::lua_to_string);
        
        sol::table string_metatable = lua["string"];
        string_metatable.set_function("starts_with", [](std::string self, std::string check) -> bool {
            return string_startsWith(self, check);
        });
        string_metatable.set_function("ends_with", [](std::string self, std::string check) -> bool {
            return string_endsWith(self, check);
        });
        string_metatable.set_function("concat", &Amara::lua_string_concat);
    }
}
