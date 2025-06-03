namespace Amara { 
    #ifdef AMARA_OPENGL
    
    class ShaderManager;

    class ShaderProgram {
    public:
        ShaderManager* manager = nullptr;

        bool destroyed = false;

        using UniformType = std::variant<
            bool,
            int,
            float,
            Vector2,
            Vector3,
            Vector4,
            Matrix4x4
        >;

        std::unordered_map<std::string, UniformType> uniforms;

        ShaderProgram() = default;
        ShaderProgram(unsigned int id) : programID(id) {}
        ShaderProgram(std::string k, unsigned int id) : key(k), programID(id) {}
        
        std::string key;
        unsigned int programID = 0;

        void applyShader() {
            if (destroyed) return;
            if (programID != 0) {
                glUseProgram(programID);
            }
            for (auto& uniform : uniforms) {
                GLint location = glGetUniformLocation(programID, uniform.first.c_str());
                if (location == -1) {
                    debug_log("Warning: Uniform \"", uniform.first, "\" not found in shader program \"", key, "\".");
                    continue;
                }
                if (isUniform<bool>(uniform.second) || isUniform<int>(uniform.second)) {
                    glUniform1i(location, getUniform<int>(uniform.second));
                }
                else if (isUniform<float>(uniform.second)) {
                    glUniform1f(location, getUniform<float>(uniform.second));
                }
                else if (isUniform<Vector2>(uniform.second)) {
                    Vector2 vec = getUniform<Vector2>(uniform.second);
                    glUniform2f(location, vec.x, vec.y);
                }
                else if (isUniform<Vector3>(uniform.second)) {
                    Vector3 vec = getUniform<Vector3>(uniform.second);
                    glUniform3f(location, vec.x, vec.y, vec.z);
                }
                else if (isUniform<Vector4>(uniform.second)) {
                    Vector4 vec = getUniform<Vector4>(uniform.second);
                    glUniform4f(location, vec.x, vec.y, vec.z, vec.w);
                }
                else if (isUniform<Matrix4x4>(uniform.second)) {
                    Matrix4x4 mat = getUniform<Matrix4x4>(uniform.second);
                }
            }
        }

        void configure(nlohmann::json config);

        void setUniform(std::string name, nlohmann::json value) {
            if (destroyed) return;
            if (value.is_array()) {
                if (value.size() == 4) {
                    uniforms[name] = Vector4(value[0], value[1], value[2], value[3]);
                }
                else if (value.size() == 3) {
                    uniforms[name] = Vector3(value[0], value[1], value[2]);
                }
                else if (value.size() == 2) {
                    uniforms[name] = Vector2(value[0], value[1]);
                }
                else if (value.size() == 16) {
                    uniforms[name] = Matrix4x4(
                        value[0], value[1], value[2], value[3],
                        value[4], value[5], value[6], value[7],
                        value[8], value[9], value[10], value[11],
                        value[12], value[13], value[14], value[15]
                    );
                }
                return;
            }
            if (value.is_number_integer()) {
                uniforms[name] = (int)value;
                return;
            }
            if (value.is_number_float()) {
                uniforms[name] = (float)value;
                return;
            }
            if (value.is_boolean()) {
                uniforms[name] = (bool)value;
                return;
            }
            if (value.is_string()) {
                if (Color::isColor(value)) {
                    Color color = value;
                    uniforms[name] = Vector4(
                        color.r / 255.0f,
                        color.g / 255.0f,
                        color.b / 255.0f,
                        color.a / 255.0f
                    );
                    return;
                }
            }
            if (value.is_object()) {
                if (json_has(value, "r", "g", "b", "a")) {
                    uniforms[name] = Vector4(value["r"], value["g"], value["b"], value["a"]);
                    return;
                }
                if (json_has(value, "x", "y", "z", "w")) {
                    uniforms[name] = Vector4(value["x"], value["y"], value["z"], value["w"]);
                    return;
                }
                if (json_has(value, "x", "y", "w", "h")) {
                    uniforms[name] = Vector4(value["x"], value["y"], value["w"], value["h"]);
                    return;
                }
                if (json_has(value, "x", "y", "z")) {
                    uniforms[name] = Vector3(value["x"], value["y"], value["z"]);
                    return;
                }
                if (json_has(value, "x", "y")) {
                    uniforms[name] = Vector2(value["x"], value["y"]);
                    return;
                }
            }
        }
        void setUniform(std::string name, sol::object config) {
            setUniform(name, lua_to_json(config));
        }

        template <typename T>
        bool isUniform(UniformType type) {
            return std::holds_alternative<T>(type);
        }

        template <typename T>
        T getUniform(std::string name) {
            return getUniform<T>(uniforms[name]);
        }

        template <typename T>
        T getUniform(UniformType type) {
            if (!isUniform<T>(type)) {
                throw std::bad_cast();
            }
            return std::get<T>(type);
        }
        
        virtual void destroy() {
            if (destroyed) return;
            glDeleteProgram(programID);

            destroyed = true;
            programID = 0;
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<ShaderProgram>("ShaderProgram",
                "destroy", &ShaderProgram::destroy,
                "key", &ShaderProgram::key,
                "configure", &ShaderProgram::configure,
                "setUniform", sol::resolve<void(std::string, sol::object)>(&ShaderProgram::setUniform)
            );
        }
    };
    #endif
}