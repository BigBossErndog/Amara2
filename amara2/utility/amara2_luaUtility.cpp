namespace Amara {
    class Node;

    std::string node_to_string(sol::object);
    std::string node_to_short_string(sol::object);
    
    sol::object json_to_lua(sol::state& lua, nlohmann::json json) {
        if (json.is_null()) {
            return sol::make_object(lua, sol::nil);
        } else if (json.is_boolean()) {
            return sol::make_object(lua, json.get<bool>());
        } else if (json.is_number_integer()) {
            return sol::make_object(lua, json.get<int>());
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
        } else if (obj.get_type() == sol::type::number) {
            if (obj.is<int>()) {
                return obj.as<int>();
            }
            return obj.as<double>();
        } else if (obj.get_type() == sol::type::string) {
            return obj.as<std::string>();
        } else if (obj.is<Amara::Color>()) {
            return obj.as<Amara::Color>().toJSON();
        } else if (obj.is<Rectangle>()) {
            return obj.as<Rectangle>().toJSON();
        } else if (obj.is<Circle>()) {
            return obj.as<Circle>().toJSON();
        } else if (obj.is<Triangle>()) {
            return obj.as<Triangle>().toJSON();
        } else if (obj.is<Line>()) {
            return obj.as<Line>().toJSON();
        } else if (obj.is<Quad>()) {
            return obj.as<Quad>().toJSON();
        } else if (obj.is<Vector4>()) {
            return obj.as<Vector4>().toJSON();
        } else if (obj.is<Vector3>()) {
            return obj.as<Vector3>().toJSON();
        } else if (obj.is<Vector2>()) {
            return obj.as<Vector2>().toJSON();
        } else if (is_node(obj)) {
            return node_to_short_string(obj);
        } else if (obj.get_type() == sol::type::table) {
            sol::table tbl = obj.as<sol::table>();
            nlohmann::json json = nlohmann::json::array();
    
            bool isArray = true;

            size_t index = 1;
            for (auto& pair : tbl) {
                sol::object key = pair.first;
                sol::object value = pair.second;
    
                if (key.get_type() == sol::type::number && key.as<size_t>() == index) {
                    if (isArray) {
                        json.push_back(lua_to_json(value));
                        ++index;
                    }
                }
                else {
                    isArray = false;
                    break;
                }
            }
    
            if (isArray) {
                if (json.size() > 0) return json;
                return nlohmann::json::object();
            }
            else {
                json = nlohmann::json::object();
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
        if (!obj.is<sol::table>() || obj.is<sol::userdata>()) {
            return false;
        }

        sol::table table = obj.as<sol::table>();
        if (table.size() == 0) return false;
        
        int expected_key = 1;
        
        for (const auto& pair : table) {
            if (!pair.first.is<int>() || pair.first.as<int>() != expected_key) {
                return false;
            }
            expected_key++;
        }

        return true;
    }
    
    std::string lua_to_string(sol::object obj, bool printing, const std::string& indent, const std::string& current_string) {
        if (obj.is<sol::lua_nil_t>()) return "nil";
        if (obj.is<bool>()) {
            return obj.as<bool>() ? "true" : "false";
        }
        if (obj.is<std::string>()) {
            if (printing) {
                return "\"" + obj.as<std::string>() + "\"";
            }
            else {
                return obj.as<std::string>();
            }
        }
        if (obj.is<sol::function>()) return "(function)";

        if (obj.is<double>()) {
            double val = obj.as<double>();
            if (floor(val) == val) return std::to_string((int)floor(val));
            else return std::to_string(val);
        }
        if (obj.is<int>()) return std::to_string(obj.as<int>());

        if (obj.is<Rectangle>()) return std::string(obj.as<Rectangle>());
        if (obj.is<Circle>()) return std::string(obj.as<Circle>());
        if (obj.is<Triangle>()) return std::string(obj.as<Triangle>());
        if (obj.is<Line>()) return std::string(obj.as<Line>());
        if (obj.is<Vector3>()) return std::string(obj.as<Vector3>());
        if (obj.is<Vector2>()) return std::string(obj.as<Vector2>());
        
        if (obj.is<Color>()) return std::string(obj.as<Color>());

        if (is_node(obj)) return node_to_string(obj);

        if (obj.is<sol::table>()) {
            sol::table tbl = obj.as<sol::table>();
            if (lua_object_is_table_array(obj)) {
                std::vector<std::string> items;
                int width = 0;
                
                std::string result = std::string("{");
                for (auto& item : tbl) {
                    std::string item_str = lua_to_string(item.second, printing, indent + std::string("  "), String::last_line(result));
                    items.push_back(item_str);
                    width += String::longest_string(item_str) + 2;
                }
                
                width += indent.size() + current_string.size() + 2;
                bool oversized = width > 32;
                if (!oversized) result += std::string(" ");
                else result += std::string("\n");
                
                for (int i = 0; i < items.size(); i++) {
                    if (oversized) result += indent + std::string("  ") + items[i];
                    else result += items[i];
                    if (i < items.size() - 1) {
                        if (oversized) result += std::string(",\n");
                        else result += std::string(", ");
                    }
                }
                
                if (oversized) result += std::string("\n") + indent + std::string("}");
                else result += std::string(" }");
                
                return result;
            }
            else {
                std::vector<std::string> items;
                int width = 0;
                
                std::string result = "{";
                for (auto& item : tbl) {
                    std::string item_str = item.first.as<std::string>() + std::string(" = ");
                    item_str += lua_to_string(item.second, printing, indent + std::string("  "), String::last_line(result + item_str));
                    items.push_back(item_str);
                    width += String::longest_string(item_str) + 2;
                }
                
                width += indent.size() + current_string.size() + 2;
                bool oversized = width > 32;
                if (!oversized) result += std::string(" ");
                else result += std::string("\n");
                
                for (int i = 0; i < items.size(); i++) {
                    if (oversized) result += indent + std::string("  ") + items[i];
                    else result += items[i];
                    if (i < items.size() - 1) {
                        if (oversized) result += std::string(",\n");
                        else result += std::string(", ");
                    }
                }
                if (oversized) result += std::string("\n") + indent + std::string("}");
                else result += std::string(" }");
                
                return result;
            }
        }
        
        return "UNRECOGNIZED_OBJECT";
    }
    std::string lua_to_string(sol::object obj) {
        return lua_to_string(obj, false, "", "");
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

    std::string lua_string_concat(sol::variadic_args args, bool printing) {
        std::ostringstream ss;
        for (auto arg : args) {
            ss << lua_to_string(arg, printing, "", "");
        }
        return ss.str();
    }
    std::string lua_string_concat(sol::variadic_args args) {
        return lua_string_concat(args, false);
    }
    
    std::string lua_string_sep_concat(const std::string& separator, sol::variadic_args args, bool printing) {
        std::ostringstream ss;
        bool first = true;
        
        for (auto arg : args) {
            ss << (first ? "" : separator) << lua_to_string(arg, printing, "", "");
            first = false;
        }
    
        return ss.str();
    }
    std::string lua_string_sep_concat(const std::string& separator, sol::variadic_args args) {
        return lua_string_sep_concat(separator, args, false);
    }

    sol::table lua_shallow_copy(sol::state& lua, sol::table tbl) {
        if (tbl.is<sol::userdata>()) return tbl;
        if (!tbl.is<sol::table>()) return tbl;
        
        sol::table copy = lua.create_table();
        for (auto& pair : tbl) {
            copy[pair.first] = pair.second;
        }
        return copy;
    }

    sol::table lua_deep_copy(sol::state& lua, sol::table src) {
        if (src.is<sol::userdata>()) return src;
        if (!src.is<sol::table>()) return src;
        
        sol::table dst(lua, sol::create);

        for (auto& kv : src) {
            sol::object key = kv.first;
            sol::object value = kv.second;

            if (value.is<sol::table>() && !value.is<sol::userdata>()) {
                dst[key] = lua_deep_copy(lua, value.as<sol::table>());
            } else {
                dst[key] = value;
            }
        }
        return dst;
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
            ss << lua_to_string(arg, true, "", "");
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
    Quad& Quad::operator= (sol::object obj) {
        if (obj.is<Quad>()) *this = obj.as<Quad>();
        else *this = lua_to_json(obj);
        return *this;
    }
    Circle& Circle::operator= (sol::object obj) {
        if (obj.is<Circle>()) *this = obj.as<Circle>();
        else if (obj.is<Vector2>()) *this = Circle(obj.as<Vector2>());
        else *this = lua_to_json(obj);
        return *this;
    }
    Triangle& Triangle::operator= (sol::object obj) {
        if (obj.is<Triangle>()) *this = obj.as<Triangle>();
        else *this = lua_to_json(obj);
        return *this;
    }
    Line& Line::operator= (sol::object obj) {
        if (obj.is<Line>()) *this = obj.as<Line>();
        else *this = lua_to_json(obj);
        return *this;
    }

    Amara::Color& Amara::Color::operator= (sol::object obj) {
        if (obj.is<Amara::Color>()) *this = obj.as<Amara::Color>();
        else *this = lua_to_json(obj);
        return *this; 
    }

    double lua_random(sol::state& lua) {
        return lua["math"]["random"]();
    }
    double lua_random(sol::state& lua, double min, double max) {
        return min + (max - min) * lua_random(lua);
    }
    
    bool lua_is_truthy(const sol::object& obj) {
        return obj.valid() &&
               obj.get_type() != sol::type::nil &&
               !(obj.get_type() == sol::type::boolean && !obj.as<bool>());
    }

    void bind_lua_LuaUtilityFunctions(sol::state& lua) {
        lua.set_function("debug_log", &Amara::lua_debug_log);

        lua["lua_print"] = lua["print"];
        lua.set_function("print", [](sol::variadic_args args) {
            debug_log(lua_string_sep_concat(" ", args, true));
        });

        lua.set_function("object_to_string", sol::resolve<std::string(sol::object)>(&Amara::lua_to_string));
        
        sol::table string_metatable = lua["string"];
        string_metatable.set_function("starts_with", [](sol::object self, sol::object check) -> bool {
            if (!self.is<std::string>() || !check.is<std::string>()) {
                fatal_error("Error: string.starts_with() expected 2 string arguments.");
            }
            return String::startsWith(self.as<std::string>(), check.as<std::string>());
        });
        string_metatable.set_function("ends_with", [](sol::object self, sol::object check) -> bool {
            if (!self.is<std::string>() || !check.is<std::string>()) {
                fatal_error("Error: string.ends_with() expected 2 string arguments.");
            }
            return String::endsWith(self.as<std::string>(), check.as<std::string>());
        });
        string_metatable.set_function("contains", [](std::string self, std::string check, sol::object caseSensitive) -> bool {
            if (caseSensitive.is<bool>()) {
                return String::contains(self, check, caseSensitive.as<bool>());
            }
            return String::contains(self, check);
        });
        string_metatable.set_function("concat", sol::resolve<std::string(sol::variadic_args)>(Amara::lua_string_concat));
        string_metatable.set_function("sep_concat", sol::resolve<std::string(const std::string&, sol::variadic_args)>(Amara::lua_string_sep_concat));
        string_metatable.set_function("json_string", [](sol::object obj) {
            return lua_to_json(obj).dump();
        });
        string_metatable.set_function("random", [&lua](sol::object num_obj) {
            if (!num_obj.is<int>()) {
                fatal_error("Error: string.random() expected an integer argument.");
            }
            int num = num_obj.as<int>();
            
            std::string result = "";
            for (int i = 0; i < num; i++) {
                result += 'a' + floor(lua_random(lua) * ('z' - 'a' + 1));
            }
            
            return result;
        });

        sol::table math_metatable = lua["math"];
        math_metatable.set_function("round", [](sol::object num) -> int {
            if (!num.is<double>() && !num.is<int>()) {
                fatal_error("Error: math.round() expected a number argument.");
            }
            return std::round(num.as<double>());
        });
        math_metatable.set_function("hypotenuse", [](sol::object oa, sol::object ob) {
            if ((!oa.is<double>() && !oa.is<int>()) || (!ob.is<double>() && !ob.is<int>())) {
                fatal_error("Error: math.hypotenuse() expected 2 number arguments.");
            }
            double a = oa.as<double>();
            double b = ob.as<double>();
            return std::sqrt(a * a + b * b);
        });
        math_metatable.set_function("hash", [](sol::object input, sol::object seed) -> double {
            if (seed.is<std::string>()) {
                if (input.is<std::string>()) return Amara::hash(input.as<std::string>(), seed.as<std::string>());
                if (input.is<double>()) return Amara::hash(std::to_string(input.as<double>()), seed.as<std::string>());
                if (input.is<int>()) return Amara::hash(std::to_string(input.as<int>()), seed.as<std::string>());
                if (input.is<bool>()) return Amara::hash(std::to_string(input.as<bool>()), seed.as<std::string>());
            }
            else {
                if (input.is<std::string>()) return Amara::hash(input.as<std::string>());
                if (input.is<double>()) return Amara::hash(std::to_string(input.as<double>()));
                if (input.is<int>()) return Amara::hash(std::to_string(input.as<int>()));
                if (input.is<bool>()) return Amara::hash(std::to_string(input.as<bool>()));
            }
            return 0;
        });
        math_metatable.set_function("ease", sol::overload(
            sol::resolve<double(double, double, double, Amara::Ease)>(&Amara::ease),
            sol::resolve<double(double, double, double)>(&Amara::ease),
            sol::resolve<Amara::Color(const Amara::Color&, const Amara::Color&, double, Amara::Ease)>(&Amara::ease),
            sol::resolve<Amara::Color(const Amara::Color&, const Amara::Color&, double)>(&Amara::ease),
            sol::resolve<Amara::Vector2(const Amara::Vector2&, const Amara::Vector2&, double, Amara::Ease)>(&Amara::ease),
            sol::resolve<Amara::Vector2(const Amara::Vector2&, const Amara::Vector2&, double)>(&Amara::ease)
        ));
        
        sol::table table_metatable = lua["table"];
        table_metatable.set_function("size", [](sol::object tbl) -> int {
            if (!tbl.is<sol::table>()) {
                fatal_error("Error: table.size() expected a table argument.");
            }
            sol::table t = tbl.as<sol::table>();

            if (lua_object_is_table_array(tbl)) {
                return static_cast<int>(t.size());
            }

            int count = 0;
            for (auto& pair : t) {
                ++count;
            }
            return count;
        });
        table_metatable.set_function("to_string", string_to_lua_object(lua, lua_table_to_string));
        table_metatable.set_function("shallow_copy", [&lua](sol::object tbl) -> sol::table {
            if (!tbl.is<sol::table>()) {
                fatal_error("Error: table.shallow_copy() expected a table argument.");
            }
            return lua_shallow_copy(lua, tbl.as<sol::table>());
        });
        table_metatable.set_function("deep_copy", [&lua](sol::object tbl) -> sol::table {
            if (!tbl.is<sol::table>()) {
                fatal_error("Error: table.deep_copy() expected a table argument.");
            }
            return lua_deep_copy(lua, tbl.as<sol::table>());
        });
        table_metatable.set_function("append", [](sol::object obj1, sol::object obj2) {
            if (!obj1.is<sol::table>() || !obj2.is<sol::table>()) {
                fatal_error("Error: table.append() expected 2 table arguments.");
            }
            
            sol::table tbl1 = obj1.as<sol::table>();
            sol::table tbl2 = obj2.as<sol::table>();
            
            int len = tbl1.size();
            for (auto& pair : tbl2) {
                tbl1[len + 1] = pair.second;
            }
            return tbl1;
        });
        table_metatable.set_function("merge", [&lua](sol::object obj1, sol::object obj2) {
            if (!obj1.is<sol::table>() || !obj2.is<sol::table>()) {
                fatal_error("Error: table.merge() expected 2 table arguments.");
            }
            
            sol::table t1 = obj1.as<sol::table>();
            sol::table t2 = obj2.as<sol::table>();
            
            sol::table new_table = lua.create_table();
            for (auto& it: t1) {
                new_table[it.first] = it.second;
            }
            for (auto& it: t2) {
                new_table[it.first] = it.second;
            }
            return new_table;
        });
        table_metatable.set_function("update", [](sol::table t1, sol::table t2) {
            if (!t1.is<sol::table>() || !t2.is<sol::table>()) {
                fatal_error("Error: table.update() expected 2 table arguments.");
            }
            for (auto& it: t2) {
                t1[it.first] = it.second;
            }
            return t1;
        });
        table_metatable.set_function("is_array", [](sol::object tbl) {
            return lua_object_is_table_array(tbl);
        });
        table_metatable.set_function("contains", [](sol::table tbl, sol::object check) {
            if (!tbl.is<sol::table>()) {
                fatal_error("Error: table.contains() expected a table argument.");
            }

            for (auto& kv : tbl) {
                const sol::object& val = kv.second;
                if (val == check) {
                    return true;
                }
            }
            return false;
        });
        table_metatable.set_function("shuffle", [&lua](sol::object obj) {
            if (!obj.is<sol::table>()) {
                fatal_error("Error: table.shuffle() expected a table argument.");
            }
            sol::table t = obj.as<sol::table>();
            sol::table result = lua.create_table();
            
            std::vector<sol::object> values;
            for (auto& pair : t) {
                values.push_back(pair.second);
            }
        
            int n = static_cast<int>(values.size());
            for (int i = n - 1; i > 0; --i) {
                int j = static_cast<int>(std::floor(lua_random(lua) * (i + 1)));
                std::swap(values[i], values[j]);
            }
        
            for (int i = 0; i < n; ++i) {
                result[i + 1] = values[i];
            }
        
            return result;
        });
        table_metatable.set_function("filter", [&lua](sol::object obj, sol::object predicate, sol::object inclusive) {
            if (!obj.is<sol::table>()) {
                fatal_error("Error: table.filter() expected a table argument.");
            }
            sol::table tbl = obj.as<sol::table>();
            sol::table result = lua.create_table();
            
            bool is_table_array = lua_object_is_table_array(tbl);
            
            if (predicate.is<sol::function>()) {
                sol::function func = predicate.as<sol::function>();
                for (auto& pair : tbl) {
                    try {
                        sol::protected_function_result r = func(pair.second);
                        if (!r.valid()) {
                            sol::error err = r;
                            throw std::runtime_error(err.what());
                        }
                        
                        if (lua_is_truthy(r)) {
                            if (is_table_array) {
                                result[result.size() + 1] = pair.second;
                            }
                            else {
                                result[pair.first] = pair.second;
                            }
                        }
                    }
                    catch (const sol::error& err) {
                        fatal_error(err.what());
                    }
                }
            }
            else {
                for (auto& pair : tbl) {
                    if ((inclusive.is<bool>() && inclusive.as<bool>()) == (pair.second == predicate)) {
                        if (is_table_array) {
                            result[result.size() + 1] = pair.second;
                        }
                        else {
                            result[pair.first] = pair.second;
                        }
                    }
                }
            }
            
            return result;
        });
        table_metatable.set_function("ref_filter", [&lua](sol::object obj, sol::object predicate, sol::object inclusive) {
            if (!obj.is<sol::table>()) {
                fatal_error("Error: table.filter() expected a table argument.");
            }
            
            sol::table tbl = obj.as<sol::table>();
            if (tbl.size() == 0) return tbl;
            
            bool is_table_array = lua_object_is_table_array(tbl);
            
            auto should_keep = [&](sol::object value) -> bool {
                if (predicate.is<sol::function>()) {
                    sol::protected_function func = predicate.as<sol::protected_function>();
                    sol::protected_function_result r = func(value);
                    if (!r.valid()) {
                        sol::error err = r;
                        fatal_error(err.what());
                    }
                    return lua_is_truthy(r);
                } else {
                    bool is_inclusive = inclusive.is<bool>() && inclusive.as<bool>();
                    return (is_inclusive == (value == predicate));
                }
            };
        
            if (is_table_array) {
                int write_ptr = 1;
                int current_size = static_cast<int>(tbl.size());
        
                for (int read_ptr = 1; read_ptr <= current_size; ++read_ptr) {
                    sol::object val = tbl[read_ptr];
                    if (should_keep(val)) {
                        if (read_ptr != write_ptr) {
                            tbl[write_ptr] = val;
                        }
                        write_ptr++;
                    }
                }
                for (int i = write_ptr; i <= current_size; ++i) {
                    tbl[i] = sol::nil;
                }
            } else {
                std::vector<sol::object> keys_to_remove;
                for (auto& pair : tbl) {
                    if (!should_keep(pair.second)) {
                        keys_to_remove.push_back(pair.first);
                    }
                }
        
                for (const auto& key : keys_to_remove) {
                    tbl[key] = sol::nil;
                }
            }
        
            return tbl;
        });
        table_metatable.set_function("crop", [&lua](sol::object obj, sol::object arg) {
            if (!obj.is<sol::table>() || (!arg.is<int>() && !arg.is<double>() && !arg.is<float>())) {
                fatal_error("Error: table.crop() expected a (table, number) arguments.");
            }
            if (!arg.is<int>()) {
                fatal_error("Error: table.crop() expected an integer argument.");
            }
            sol::table t = obj.as<sol::table>();
            sol::table result = lua.create_table();
            
            int num = 0;
            if (arg.is<int>()) num = arg.as<int>();
            else if (arg.is<double>()) num = static_cast<int>(arg.as<double>());
            else if (arg.is<float>()) num = static_cast<int>(arg.as<float>());
            
            while (result.size() < num && result.size() < t.size()) {
                result[result.size() + 1] = t[result.size() + 1];
            }
        
            return result;
        });
        table_metatable.set_function("wrap_index", [](sol::object tbl_obj, sol::object index_obj) -> sol::object {
            if (!tbl_obj.is<sol::table>() || !index_obj.is<int>()) {
                fatal_error("Error: table.wrap_index() expected (table, integer) arguments.");
            }
            
            sol::table tbl = tbl_obj.as<sol::table>();
            int tbl_size = (int)tbl.size();
            if (tbl_size == 0) return sol::nil;
            
            int index = index_obj.as<int>();
            index = ((index - 1) % tbl_size + tbl_size) % tbl_size + 1; 
            
            return tbl[index];
        });
        table_metatable.set_function("remove_duplicates", [&lua](sol::object tbl_obj, sol::object check) {
        
                if (!lua_object_is_table_array(tbl_obj)) {
                    fatal_error("Error: table.remove_duplicates() expected an array-style table argument.");
                }
        
                sol::table tbl = tbl_obj.as<sol::table>();
                sol::state_view lua_state(lua.lua_state());
                sol::table new_table = lua_state.create_table();
        
                sol::function comparator;
                bool use_custom_compare = false;
        
                if (check.is<sol::function>()) {
                    comparator = check.as<sol::function>();
                    use_custom_compare = true;
                }
        
                std::size_t new_index = 1;
        
                for (std::size_t i = 1;; ++i) {
                    sol::object value = tbl.raw_get<sol::object>(i);
                    
                    if (!value.valid()) break;
        
                    bool is_duplicate = false;
        
                    for (std::size_t j = 1; j < new_index; ++j) {
                        sol::object existing = new_table.raw_get<sol::object>(j);
        
                        bool equal = false;
        
                        if (use_custom_compare) {
                            sol::protected_function_result result = comparator(value, existing);
                            if (!result.valid()) {
                                sol::error err = result;
                                fatal_error(std::string("Error: ") + err.what());
                            }
                            equal = result.get<bool>();
                        }
                        else {
                            sol::protected_function_result result = lua_state["rawequal"](value, existing);
        
                            if (!result.valid()) {
                                sol::error err = result;
                                fatal_error(std::string("Error: ") + err.what());
                            }
        
                            equal = result.get<bool>();
                        }
        
                        if (equal) {
                            is_duplicate = true;
                            break;
                        }
                    }
        
                    if (!is_duplicate) {
                        new_table.raw_set(new_index++, value);
                    }
                }
        
                return new_table;
            }
        );
        table_metatable.set_function("random", [&lua](sol::object obj) -> sol::object {
            if (!lua_object_is_table_array(obj)) {
                fatal_error("Error: table.random() expected an array-like table argument.");
            }
            
            sol::table tbl = obj.as<sol::table>();
            int tbl_size = (int)tbl.size();
            if (tbl_size == 0) return sol::nil;
            
            int index = static_cast<int>(lua_random(lua) * tbl_size) + 1;
            
            return tbl[index];
        });

        lua["fatal_error"] = [](sol::variadic_args args) {
            fatal_error(lua_string_concat(args));
        };
    }
}
