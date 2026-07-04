namespace Amara {
    #ifdef AMARA_OPENGL

    class ShaderManager;

    class ShaderProgram {
    public:
        ShaderManager *manager = nullptr;

        bool destroyed = false;

        using UniformType =
            std::variant<bool, int, float, Vector2, Vector3, Vector4, Matrix4x4>;

        std::unordered_map<std::string, UniformType> uniforms;
        std::unordered_map<std::string, GLenum> type_check;
        std::unordered_map<std::string, GLint> locations;

        sol::object lua_object;
        sol::table uniform_proxy;

        Amara::GameProps *gameProps = nullptr;

        ShaderProgram() = default;
        ShaderProgram(unsigned int id) : programID(id) {}
        ShaderProgram(std::string k, unsigned int id) : key(k), programID(id) {}

        std::string key;
        unsigned int programID = 0;

        GLint getLocation(std::string name) {
            if (locations.find(name) == locations.end()) {
                locations[name] = glGetUniformLocation(programID, name.c_str());
            }
            return locations[name];
        }

        void init(Amara::GameProps *props) {
            gameProps = props;

            sol::state &lua = gameProps->lua;

            lua_object = sol::make_object(lua, this);

            uniform_proxy = lua.create_table();
            sol::table proxy_meta = lua.create_table();

            proxy_meta["__index"] = [this, &lua](sol::table t,
                                                std::string name) -> sol::object {
                if (this->uniforms.find(name) != this->uniforms.end()) {
                    if (this->isUniform<bool>(name)) {
                        return sol::make_object(lua, getUniform<bool>(name));
                    }
                    if (this->isUniform<int>(name)) {
                        return sol::make_object(lua, getUniform<int>(name));
                    }
                    if (this->isUniform<float>(name)) {
                        return sol::make_object(lua, getUniform<float>(name));
                    }
                    if (this->isUniform<Vector2>(name)) {
                        return sol::make_object(lua, getUniform<Vector2>(name));
                    }
                    if (this->isUniform<Vector3>(name)) {
                        return sol::make_object(lua, getUniform<Vector3>(name));
                    }
                    if (this->isUniform<Vector4>(name)) {
                        return sol::make_object(lua, getUniform<Vector4>(name));
                    }
                }
                return sol::lua_nil;
            };
            proxy_meta["__newindex"] = [this](sol::table t, std::string name,
                                            sol::object value) {
                this->setUniform(name, lua_to_json(value));
            };
            uniform_proxy[sol::metatable_key] = proxy_meta;
        }

        void applyShader() {
            if (destroyed)
                return;
            if (programID != 0) {
                glUseProgram(programID);
            }

            if (type_check.empty() && programID != 0) {
                GLint activeUniforms = 0;
                glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &activeUniforms);
                GLint maxNameLength = 0;
                glGetProgramiv(programID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);
                if (maxNameLength > 0) {
                    std::vector<GLchar> nameBuffer(maxNameLength);
                    for (GLint i = 0; i < activeUniforms; ++i) {
                        GLsizei length;
                        GLint size;
                        GLenum uType;
                        glGetActiveUniform(programID, i, maxNameLength, &length, &size,
                                        &uType, nameBuffer.data());
                        std::string uniformName(nameBuffer.data(), length);
                        size_t bracket = uniformName.find('[');
                        if (bracket != std::string::npos) {
                            uniformName = uniformName.substr(0, bracket);
                        }
                        type_check[uniformName] = uType;
                    }
                }
            }

            for (auto &uniform : uniforms) {
                GLint location = getLocation(uniform.first);
                if (location == -1) {
                    debug_log("Warning: Uniform \"", uniform.first,
                            "\" not found in shader program \"", key, "\".");
                    continue;
                }

                GLenum type = GL_NONE;
                auto it = type_check.find(uniform.first);
                if (it != type_check.end()) {
                    type = it->second;
                }

                if (isUniform<bool>(uniform.second)) {
                    glUniform1i(location, getUniform<bool>(uniform.second) ? 1 : 0);
                } else if (isUniform<int>(uniform.second)) {
                    if (type == GL_INT || type == GL_BOOL) {
                        glUniform1i(location, getUniform<int>(uniform.second));
                    } else {
                        glUniform1f(location,
                                    static_cast<float>(getUniform<int>(uniform.second)));
                    }
                } else if (isUniform<float>(uniform.second)) {
                    if (type == GL_INT || type == GL_BOOL) {
                        glUniform1i(location, static_cast<int>(
                                                round(getUniform<float>(uniform.second))));
                    } else {
                        glUniform1f(location, getUniform<float>(uniform.second));
                    }
                } else if (isUniform<Vector2>(uniform.second)) {
                    Vector2 vec = getUniform<Vector2>(uniform.second);
                    glUniform2f(location, vec.x, vec.y);
                } else if (isUniform<Vector3>(uniform.second)) {
                    Vector3 vec = getUniform<Vector3>(uniform.second);
                    glUniform3f(location, vec.x, vec.y, vec.z);
                } else if (isUniform<Vector4>(uniform.second)) {
                    Vector4 vec = getUniform<Vector4>(uniform.second);
                    glUniform4f(location, vec.x, vec.y, vec.z, vec.w);
                } else if (isUniform<Matrix4x4>(uniform.second)) {
                    Matrix4x4 mat = getUniform<Matrix4x4>(uniform.second);
                }
            }
        }

        void configure(nlohmann::json config);

        void setUniform(std::string name, nlohmann::json value) {
            if (destroyed)
                return;
            if (value.is_array()) {
                if (value.size() == 4) {
                    uniforms[name] = Vector4(value[0], value[1], value[2], value[3]);
                } else if (value.size() == 3) {
                    uniforms[name] = Vector3(value[0], value[1], value[2]);
                } else if (value.size() == 2) {
                    uniforms[name] = Vector2(value[0], value[1]);
                } else if (value.size() == 16) {
                    uniforms[name] = Matrix4x4(value[0], value[1], value[2], value[3],
                                            value[4], value[5], value[6], value[7],
                                            value[8], value[9], value[10], value[11],
                                            value[12], value[13], value[14], value[15]);
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
                    uniforms[name] = Vector4(color.r / 255.0f, color.g / 255.0f,
                                            color.b / 255.0f, color.a / 255.0f);
                    return;
                }
            }
            if (value.is_object()) {
                if (json_has(value, "r", "g", "b", "a")) {
                    uniforms[name] = Vector4(
                        value["r"].get<float>() / 255.0f, value["g"].get<float>() / 255.0f,
                        value["b"].get<float>() / 255.0f, value["a"].get<float>() / 255.0f);
                    return;
                }
                if (json_has(value, "r", "g", "b")) {
                    uniforms[name] = Vector4(value["r"].get<float>() / 255.0f,
                                            value["g"].get<float>() / 255.0f,
                                            value["b"].get<float>() / 255.0f, 1.0f);
                    return;
                }
                if (json_has(value, "x", "y", "z", "w")) {
                    uniforms[name] =
                        Vector4(value["x"], value["y"], value["z"], value["w"]);
                    return;
                }
                if (json_has(value, "x", "y", "w", "h")) {
                    uniforms[name] =
                        Vector4(value["x"], value["y"], value["w"], value["h"]);
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

        template <typename T> bool isUniform(UniformType type) {
            return std::holds_alternative<T>(type);
        }

        template <typename T> bool isUniform(std::string name) {
            return isUniform<T>(uniforms[name]);
        }

        template <typename T> T getUniform(std::string name) {
            return getUniform<T>(uniforms[name]);
        }

        template <typename T> T getUniform(UniformType type) {
            if (!isUniform<T>(type)) {
                throw std::bad_cast();
            }
            return std::get<T>(type);
        }

        virtual void destroy() {
            if (destroyed)
                return;
            glDeleteProgram(programID);

            destroyed = true;
            programID = 0;
        }

        static void bind_lua(sol::state &lua) {
            lua.new_usertype<ShaderProgram>(
                "ShaderProgram", "destroy", &ShaderProgram::destroy, "key",
                &ShaderProgram::key, "configure", &ShaderProgram::configure,
                "setUniform",
                sol::resolve<void(std::string, sol::object)>(
                    &ShaderProgram::setUniform),
                "uniforms", sol::readonly(&ShaderProgram::uniform_proxy));
        }
    };
    #endif
}