namespace Amara {
    class Node;

    std::string node_to_string(sol::object);
    
    sol::object json_to_lua(sol::state& lua, nlohmann::json json) {
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
                arr[i + 1] = json_to_lua(lua, json[i]);
            }
            return sol::make_object(lua, arr);
        } else if (json.is_object()) {
            sol::table tbl = lua.create_table();
            for (auto& item : json.items()) {
                std::string key = item.key();
                tbl[key] = json_to_lua(lua, item.value());
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
        } else if (obj.is<int>()) {
            return obj.as<int>();
        } else if (obj.is<double>()) {
            return obj.as<double>();
        } else if (obj.get_type() == sol::type::string) {
            return obj.as<std::string>();
        } else if (obj.is<Amara::Color>()) {
            return obj.as<Amara::Color>().toJSON();
        } else if (obj.is<Vector2>()) {
            return obj.as<Vector2>().toJSON();
        } else if (obj.is<Vector3>()) {
            return obj.as<Vector3>().toJSON();
        } else if (obj.is<Vector4>()) {
            return obj.as<Vector4>().toJSON();
        } else if (obj.is<Rectangle>()) {
            return obj.as<Rectangle>().toJSON();
        } else if (obj.is<Circle>()) {
            return obj.as<Circle>().toJSON();
        } else if (is_node(obj)) {
            return node_to_string(obj);
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

    bool lua_object_is_table_array(const sol::object& obj) {
        if (!obj.is<sol::table>()) {
            return false;
        }

        sol::table table = obj.as<sol::table>();
        int expected_key = 1;

        for (const auto& pair : table) {
            if (!pair.first.is<int>() || pair.first.as<int>() != expected_key) {
                return false;
            }
            expected_key++;
        }

        return true;
    }

    std::string lua_to_string(sol::object obj) {
        if (obj.is<sol::nil_t>()) return "nil";
        if (obj.is<std::string>()) return obj.as<std::string>();
        if (obj.is<sol::function>()) return "(function)";

        if (obj.is<int>()) return std::to_string(obj.as<int>());
        if (obj.is<double>()) {
            double val = obj.as<double>();
            if (floor(val) == val) return std::to_string((int)val);
            else return std::to_string(val);
        }

        if (obj.is<Rectangle>()) return std::string(obj.as<Rectangle>());
        if (obj.is<Vector3>()) return std::string(obj.as<Vector3>());
        if (obj.is<Vector2>()) return std::string(obj.as<Vector2>());

        if (obj.is<Color>()) return std::string(obj.as<Color>());

        if (is_node(obj)) return node_to_string(obj);

        if (obj.is<sol::table>()) {
            nlohmann::json j = lua_to_json(obj);
            return j.dump(4);
        }

        return lua_to_json(obj).dump(4);
    }

    sol::object string_to_lua_object(sol::state& lua, const std::string& luaString) {
        std::string luaCode = "return " + luaString;
        sol::protected_function_result result = lua.do_string(luaCode);
        
        if (result.valid()) {
            sol::object obj = result;
            return obj;
        } 
        else {
            return sol::nil;
        }
    }
    
    template <typename T>
    sol::table vector_to_lua(sol::state& lua, const std::vector<T>& vec) {
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

    const char* lua_table_to_string = R"(
        function(val, name, skipnewlines, depth)
            skipnewlines = skipnewlines or false
            depth = depth or 0

            local tmp = string.rep(" ", depth)

            if name then tmp = tmp .. name .. " = " end

            if type(val) == "table" then
                tmp = tmp .. "{" .. (not skipnewlines and "\n" or "")

                for k, v in pairs(val) do
                    tmp =  tmp .. table.to_string(v, k, skipnewlines, depth + 1) .. "," .. (not skipnewlines and "\n" or "")
                end

                tmp = tmp .. string.rep(" ", depth) .. "}"
            elseif type(val) == "number" then
                tmp = tmp .. tostring(val)
            elseif type(val) == "string" then
                tmp = tmp .. string.format("%q", val)
            elseif type(val) == "boolean" then
                tmp = tmp .. (val and "true" or "false")
            else
                tmp = tmp .. "\"[inserializeable datatype:" .. type(val) .. "]\""
            end

            return tmp
        end
    )";
    
    void lua_debug_log(sol::variadic_args args) {
        std::ostringstream ss;
        for (auto arg : args) {
            ss << lua_to_string(arg);
        }
        debug_log(ss.str());
    }
    void c_style_log(const char* format, ...) {
        va_list args;
        va_start(args, format);

        int size = vsnprintf(nullptr, 0, format, args);
        va_end(args);
        
        if (size <= 0) return;

        std::vector<char> buffer(size + 1);
    
        va_start(args, format);
        vsnprintf(buffer.data(), buffer.size(), format, args);
        va_end(args);
        
        debug_log(std::string(buffer.data()));
    }

    Vector2& Vector2::operator= (sol::object obj) {
        if (obj.is<Vector2>()) *this = obj.as<Vector2>();
        else *this = lua_to_json(obj);
        return *this;
    }
    Vector3& Vector3::operator= (sol::object obj) {
        if (obj.is<Vector3>()) *this = obj.as<Vector3>();
        else if (obj.is<Vector2>()) *this = obj.as<Vector2>();
        else *this = lua_to_json(obj);
        return *this;
    }
    Vector4& Vector4::operator= (sol::object obj) {
        if (obj.is<Vector4>()) *this = obj.as<Vector4>();
        else if (obj.is<Vector3>()) *this = obj.as<Vector3>();
        else if (obj.is<Vector2>()) *this = obj.as<Vector2>();
        else *this = lua_to_json(obj);
        return *this;
    }
    Rectangle& Rectangle::operator= (sol::object obj) {
        if (obj.is<Rectangle>()) *this = obj.as<Rectangle>();
        else *this = lua_to_json(obj);
        return *this;
    }
    Circle& Circle::operator= (sol::object obj) {
        if (obj.is<Circle>()) *this = obj.as<Circle>();
        else if (obj.is<Vector2>()) *this = Circle(obj.as<Vector2>());
        else *this = lua_to_json(obj);
        return *this;
    }

    Amara::Color& Amara::Color::operator= (sol::object obj) {
        if (obj.is<Amara::Color>()) *this = obj.as<Amara::Color>();
        else *this = lua_to_json(obj);
        return *this; 
    }

    void bind_lua_LuaUtilityFunctions(sol::state& lua) {
        lua.set_function("debug_log", &Amara::lua_debug_log);

        lua["lua_print"] = lua["print"];
        lua.set_function("print", [](sol::variadic_args args) {
            debug_log(lua_string_sep_concat(" ", args));
        });

        lua.set_function("object_to_string", &Amara::lua_to_string);
        
        sol::table string_metatable = lua["string"];
        string_metatable.set_function("starts_with", [](std::string self, std::string check) -> bool {
            return String::startsWith(self, check);
        });
        string_metatable.set_function("ends_with", [](std::string self, std::string check) -> bool {
            return String::endsWith(self, check);
        });
        string_metatable.set_function("contains", [](std::string self, std::string check) -> bool {
            return String::contains(self, check);
        });
        string_metatable.set_function("concat", &Amara::lua_string_concat);
        string_metatable.set_function("sep_concat", &Amara::lua_string_sep_concat);
        string_metatable.set_function("json_string", [](sol::object obj) {
            return lua_to_json(obj).dump();
        });

        sol::table math_metatable = lua["math"];
        math_metatable.set_function("round", [](double num) -> int {
            return std::round(num);
        });
        math_metatable.set_function("hypotenuse", [](double a, double b) -> int {
            return std::sqrt(a * a + b * b);
        });


        sol::table table_metatable = lua["table"];
        table_metatable.set_function("to_string", string_to_lua_object(lua, lua_table_to_string));
    }
}
