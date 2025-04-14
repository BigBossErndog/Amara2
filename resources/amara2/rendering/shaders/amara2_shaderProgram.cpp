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

        virtual void applyShader() {
            if (destroyed) return;
            if (programID != 0) {
                glUseProgram(programID);
            }
        }

        virtual void configure(nlohmann::json config);

        virtual void setUniform(std::string name, nlohmann::json value) {
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
            if (value.is_object()) {
                if (json_has(value, "x", "y")) {
                    uniforms[name] = Vector2(value["x"], value["y"]);
                    return;
                }
                if (json_has(value, "x", "y", "z")) {
                    uniforms[name] = Vector3(value["x"], value["y"], value["z"]);
                    return;
                }
                if (json_has(value, "x", "y", "z", "w")) {
                    uniforms[name] = Vector4(value["x"], value["y"], value["z"], value["w"]);
                    return;
                }
            }
        }

        template <typename T>
        bool isUniform(UniformType type) {
            return std::holds_alternative<T>(type);
        }

        template <typename T>
        T getUniform(std::string name) {
            if (!isUniform<T>(uniforms[name])) {
                throw std::bad_cast();
            }
            return std::get<T>(uniforms[name]);
        }
        
        virtual void destroy() {
            if (destroyed) return;
            glDeleteProgram(programID);

            destroyed = true;
            programID = 0;
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<ShaderProgram>("ShaderProgram",
                "destroy", &ShaderProgram::destroy,
                "key", &ShaderProgram::key,
                "configure", &ShaderProgram::configure,
                "setUniform", &ShaderProgram::setUniform
            );
        }
    };
    #endif
}