namespace Amara {
    bool json_has(const nlohmann::json& data, std::string key) {
        return (data.find(key) != data.end()) ? true : false;
    }

    template<typename... Keys>
    inline bool json_has(const nlohmann::json& data, const std::string& first_key, const Keys&... other_keys) {
        return json_has(data, first_key) && (json_has(data, other_keys) && ...);
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
}