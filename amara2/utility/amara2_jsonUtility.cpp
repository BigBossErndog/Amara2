namespace Amara {
    bool json_has(const nlohmann::json& data, std::string key) {
        return (data.find(key) != data.end()) ? true : false;
    }

    template<typename... Keys>
    inline bool json_has(const nlohmann::json& data, const std::string& first_key, const Keys&... other_keys) {
        return json_has(data, first_key) && (json_has(data, other_keys) && ...);
    }

    template<typename... Keys>
    bool json_has_any(const nlohmann::json& data, const std::string& first_key, const Keys&... other_keys) {
        if (json_has(data, first_key)) {
            return true;
        }
        return (json_has(data, other_keys) || ...);
    }

    bool json_is(const nlohmann::json& data, std::string key) {
        return json_has(data, key) && data[key].is_boolean() && data[key];
    }
    
    bool json_erase(nlohmann::json& data, std::string key) {
        if (json_has(data, key)) {
            data.erase(key);
            return true;
        }
        return false;
    }

    nlohmann::json json_extract(nlohmann::json& data, std::string key) {
        if (json_has(data, key)) {
            nlohmann::json value = data[key];
            data.erase(key);
            return value;
        }
        return nullptr;
    }
    
    nlohmann::json string_to_json(const std::string& input, bool allow_null) {
        if (input.size() == 0) return allow_null ? nullptr : std::string("");
        
        if (String::equal(input, "TRUE")) {
            return true;
        }
        if (String::equal(input, "FALSE")) {
            return false;
        }
        if (nlohmann::json::accept(input)) {
            nlohmann::json json = nlohmann::json::parse(input);
            if (json.is_string()) {
                return input;
            }
            return json;
        }
        return input;
    }
    nlohmann::json string_to_json(const std::string& input) {
        return string_to_json(input, true);
    }
    
    template <typename T>
    T json_get(const nlohmann::json& obj, const std::string& key) {
        try {
            T prop = obj[key];
            return prop;
        }
        catch (const nlohmann::json::exception& e) {
            fatal_error(e.what(), " - for property \"", key, "\"");
        }
    }
}