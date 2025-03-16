namespace Amara {
    template <class T> bool vector_contains(std::vector<T> list, T f) {
        for (T obj: list) 
            if (obj == f) return true;
        return false;
    }
    
    template <class T> void vector_append(std::vector<T>& list1, std::vector<T> list2) {
        for (T element: list2) list1.push_back(element);
    }

    bool json_has(nlohmann::json data, std::string key) {
        return (data.find(key) != data.end()) ? true : false;
    }

    bool json_is(nlohmann::json data, std::string key) {
        return json_has(data, key) && data[key].is_boolean() && data[key];
    }

    bool json_erase(nlohmann::json& data, std::string key) {
        if (json_has(data, key)) {
            data.erase(key);
            return true;
        }
        return false;
    }

    template<typename... Args>
    std::string string_concat(Args... args) {
        std::ostringstream ss;
        (ss << ... << args);
        return ss.str();
    }

    bool string_endsWith(std::string str, std::string suffix) {
        if (suffix.size() > str.size()) {
            return false;
        }
        return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
    }

    bool string_startsWith(std::string& str, std::string& prefix) {
        if (prefix.size() > str.size()) {
            return false;
        }
        return std::equal(prefix.begin(), prefix.end(), str.begin());
    }

    bool string_equal(std::string str1, std::string str2) {
        return str1.compare(str2) == 0;
    }

    std::string float_string(float n) {
        nlohmann::json json(n);
        return json.dump();
    }

    float fixed_range(float num, float min, float max) {
        if (num < min) return min;
        if (num > max) return max;
        return num;
    }
    
    float abs_mod(float num, float den) {
        while (num < 0) num += den;
        float result = fmod(num, den);
        return result;
    }

    struct ProgramArgs {
        int argv;
        char** args;
    };
}