namespace Amara {
    enum class ShaderTypeEnum {
        Vertex = GL_VERTEX_SHADER,
        Fragment = GL_FRAGMENT_SHADER,
        Geometry = GL_GEOMETRY_SHADER,
        Compute = GL_COMPUTE_SHADER,
        TessControl = GL_TESS_CONTROL_SHADER,
        TessEvaluation = GL_TESS_EVALUATION_SHADER
    };

    ShaderTypeEnum shaderTypeFromString(std::string key) {
        if (string_equal(key, "vertex")) return ShaderTypeEnum::Vertex;
        if (string_equal(key, "fragment")) return ShaderTypeEnum::Fragment;
        if (string_equal(key, "geometry")) return ShaderTypeEnum::Geometry;
        if (string_equal(key, "compute")) return ShaderTypeEnum::Compute;
        if (string_equal(key, "tessControl")) return ShaderTypeEnum::TessControl;
        if (string_equal(key, "tessEvaluation")) return ShaderTypeEnum::TessEvaluation;
        return ShaderTypeEnum::Vertex;
    }

    std::string shaderTypeToString(ShaderTypeEnum type) {
        switch (type) {
            case ShaderTypeEnum::Vertex: return "vertex";
            case ShaderTypeEnum::Fragment: return "fragment";
            case ShaderTypeEnum::Geometry: return "geometry";
            case ShaderTypeEnum::Compute: return "compute";
            case ShaderTypeEnum::TessControl: return "tessControl";
            case ShaderTypeEnum::TessEvaluation: return "tessEvaluation";
            default: return "undefined";
        }
    }
    
    class ShaderManager {
    public:
        #ifdef AMARA_OPENGL
        std::unordered_map<std::string, unsigned int> glShaders;
        std::unordered_map<std::string, ShaderProgram*> glPrograms;

        Amara::GameProps* gameProps = nullptr;

        ShaderManager() {
            glShaders.clear();
            glPrograms.clear();
        }
        #else
        ShaderManager() = default;
        #endif

        bool hasShader(std::string key) {
            #ifdef AMARA_OPENGL
            return glShaders.find(key) != glShaders.end();
            #endif
            
            return false;
        }

        #ifdef AMARA_OPENGL
        unsigned int getShader(std::string key) {
            if (hasShader(key)) return glShaders[key];
            return 0;
        }

        bool hasShaderProgram(std::string key) {
            return glPrograms.find(key) != glPrograms.end();
        }

        ShaderProgram* getShaderProgram(std::string key) {
            if (hasShaderProgram(key)) return glPrograms[key];
            return nullptr;
        }

        unsigned int compileGLShader(std::string key, std::string source, ShaderTypeEnum type) {
            if (gameProps->graphics != GraphicsEnum::OpenGL) {
                debug_log("Error: Cannot compile shader without an OpenGL context.");
                gameProps->breakWorld();                
                return 0;
            }
            unsigned int shader = glCreateShader((unsigned int)type);
            if (shader == 0) {
                debug_log("Error: Failed to create shader of type ", (int)type);
                return 0;
            }

            const char* c_source = source.c_str();

            glShaderSource(shader, 1, &c_source, NULL);
            Amara::glCompileShader(shader);
            
            int success;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                char infoLog[512];
                glGetShaderInfoLog(shader, 512, NULL, infoLog);
                debug_log("Error: Shader compilation error: ", infoLog);
                return 0;
            }

            if (!key.empty()) glShaders[key] = shader;

            return shader;
        }

        ShaderProgram* createShaderProgram(nlohmann::json config) {
            if (gameProps->graphics != GraphicsEnum::OpenGL) {
                debug_log("Error: Cannot create shader program without an OpenGL context.");               
                return nullptr;
            }
            unsigned int shaderProgramID = glCreateProgram();

            bool has_compute_shader = false;

            if (json_has(config, "compute")) {
                bool temp = false;
                std::string shader_key = json_extract(config, "compute");
                unsigned int shaderID = 0;
                if (hasShader(shader_key)) {
                    shaderID = getShader(shader_key);
                }
                else if (!shader_key.empty()) {
                    std::string filePath = gameProps->system->getAssetPath(shader_key);
                    if (gameProps->system->fileExists(filePath)) {
                        std::string source = gameProps->system->readFile(filePath);
                        shaderID = compileGLShader("", source, ShaderTypeEnum::Compute);
                        temp = true;
                    }
                    else {
                        debug_log("Error: Compute shader not found: ", shader_key);
                        return nullptr;
                    }
                }
                if (shaderID != 0) {
                    glAttachShader(shaderProgramID, shaderID);
                    has_compute_shader = true;
                }
                else {
                    debug_log("Error: Unable to compile compute shader: ", shader_key);
                    return nullptr;
                }
                if (temp) glDeleteShader(shaderID);
            }

            if (json_has(config, "vertex")) {
                bool temp = false;
                std::string shader_key = json_extract(config, "vertex");
                unsigned int shaderID = 0;
                if (hasShader(shader_key)) {
                    shaderID = getShader(shader_key);
                }
                else if (!shader_key.empty()) {
                    std::string filePath = gameProps->system->getAssetPath(shader_key);
                    if (gameProps->system->fileExists(filePath)) {
                        std::string source = gameProps->system->readFile(filePath);
                        shaderID = compileGLShader("", source, ShaderTypeEnum::Vertex);
                        temp = true;
                    }
                    else {
                        debug_log("Error: Vertex shader not found: ", shader_key);
                        return nullptr;
                    }
                }
                if (shaderID != 0) glAttachShader(shaderProgramID, shaderID);
                else {
                    debug_log("Error: Unable to compile vertex shader: ", shader_key);
                    return nullptr;
                }
                if (temp) glDeleteShader(shaderID);
            }
            else if (!has_compute_shader) {
                debug_log("Error: No vertex shader specified.");
                return nullptr;
            }

            if (json_has(config, "fragment")) {
                bool temp = false;
                std::string shader_key = json_extract(config, "fragment");
                unsigned int shaderID = 0;
                if (hasShader(shader_key)) {
                    shaderID = getShader(shader_key);
                }
                else if (!shader_key.empty()) {
                    std::string filePath = gameProps->system->getAssetPath(shader_key);
                    if (gameProps->system->fileExists(filePath)) {
                        std::string source = gameProps->system->readFile(filePath);
                        shaderID = compileGLShader("", source, ShaderTypeEnum::Fragment);
                        temp = true;
                    }
                    else {
                        debug_log("Error: Fragment shader not found: ", shader_key);
                        return nullptr;
                    }
                }
                if (shaderID != 0) glAttachShader(shaderProgramID, shaderID);
                else {
                    debug_log("Error: Unable to compile fragment shader: ", shader_key);
                    return nullptr;
                }
                if (temp) glDeleteShader(shaderID);
            }
            else if (!has_compute_shader) {
                debug_log("Error: No fragment shader specified.");
                return nullptr;
            }
            
            for (auto it = config.begin(); it != config.end(); ++it) {
                bool temp = false;
                ShaderTypeEnum type = shaderTypeFromString(it.key());
                std::string shader_key = it.value();
                unsigned int shaderID = 0;
                if (hasShader(shader_key)) {
                    std::string filePath = gameProps->system->getAssetPath(shader_key);
                    if (gameProps->system->fileExists(filePath)) {
                        std::string source = gameProps->system->readFile(filePath);
                        shaderID = compileGLShader("", source, type);
                    }
                    else {
                        debug_log("Error: Shader not found: ", shader_key);
                        return nullptr;
                    }
                }
                else if (!shader_key.empty()) {
                    debug_log("Error: Shader not found: ", shader_key);
                    return nullptr;
                }
                if (shaderID != 0) glAttachShader(shaderProgramID, shaderID);
                else {
                    debug_log("Error: Unable to compile ", it.key() ," shader: ", shader_key);
                    return nullptr;
                }
                if (temp) glDeleteShader(shaderID);
            }
            
            glLinkProgram(shaderProgramID);

            GLint success;
            glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &success);
            if (!success) {
                GLint logLength;
                glGetProgramiv(shaderProgramID, GL_INFO_LOG_LENGTH, &logLength);
                char* log = new char[logLength];
                glGetProgramInfoLog(shaderProgramID, logLength, &logLength, log);
                debug_log("Error: Shader Program Linking Failed: ", log);
                delete[] log;
                return 0;
            }

            ShaderProgram* newProgram = new ShaderProgram(shaderProgramID);
            newProgram->manager = this;
            newProgram->configure(config);
        
            return newProgram;
        }

        ShaderProgram* createShaderProgram(std::string key, nlohmann::json config) {
            if (gameProps->graphics != GraphicsEnum::OpenGL) {
                debug_log("Error: Cannot create shader program without an OpenGL context.");
                return nullptr;
            }

            ShaderProgram* shaderProgram = createShaderProgram(config);
            if (shaderProgram == nullptr) {
                debug_log("Error: Failed to create shader program \"", key, "\".");
                return nullptr;
            }
            if (hasShaderProgram(key)) {
                debug_log("Note: Shader program with key \"", key, "\" already exists. Overwriting.");
                ShaderProgram* existing = glPrograms[key];
                existing->destroy();
                delete existing;
            }
            shaderProgram->key = key;

            glPrograms[key] = shaderProgram;
            
            return shaderProgram;
        }

        ShaderProgram* createShaderProgram(std::string key, sol::object config) {
            return createShaderProgram(key, lua_to_json(config));
        }
        ShaderProgram* createShaderProgram(sol::object config) {
            return createShaderProgram(lua_to_json(config));
        }
        #endif

        void clear() {
            #ifdef AMARA_OPENGL
            for (auto& shader : glShaders) {
                glDeleteShader(shader.second);
            }
            for (auto& program : glPrograms) {
                program.second->destroy();
                delete program.second;
            }
            glShaders.clear();
            glPrograms.clear();
            #endif
        }

        bool loadShader(std::string key, std::string path, ShaderTypeEnum type) {
            if (hasShader(key)) {
                debug_log("Error: Shader with key \"", key, "\" already exists.");
                return false;
            }
            
            std::string filePath = gameProps->system->getAssetPath(path);
            std::string source = gameProps->system->readFile(filePath);

            #ifdef AMARA_OPENGL
            unsigned int shader = compileGLShader(key, source, type);
            if (shader != 0) return true;
            #endif

            debug_log("Error: Failed to load shader from file: ", path);
            return false;
        }
        
        static void bindLua(sol::state& lua) {
            lua.new_usertype<ShaderManager>("ShaderManager",
                #ifdef AMARA_OPENGL
                "createShaderProgram", sol::resolve<ShaderProgram*(std::string, sol::object)>(&ShaderManager::createShaderProgram),
                "hasShaderProgram", &ShaderManager::hasShaderProgram,
                #endif
                "hasShader", &ShaderManager::hasShader,
                "loadShader", &ShaderManager::loadShader
            );
        }
    };

    #ifdef AMARA_OPENGL
    void ShaderProgram::configure(nlohmann::json config) {

    }
    #endif
}