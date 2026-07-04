namespace Amara {
    #ifdef AMARA_OPENGL
    enum class ShaderTypeEnum {
        None = -1,
        Vertex = GL_VERTEX_SHADER,
        Fragment = GL_FRAGMENT_SHADER,
        Geometry = GL_GEOMETRY_SHADER,
        Compute = GL_COMPUTE_SHADER,
        TessControl = GL_TESS_CONTROL_SHADER,
        TessEvaluation = GL_TESS_EVALUATION_SHADER
    };
    #else
    enum class ShaderTypeEnum {
        None = -1,
        Vertex = 0,
        Fragment = 1,
        Geometry = 2,
        Compute = 3,
        TessControl = 4,
        TessEvaluation = 5
    };
    #endif

    ShaderTypeEnum shaderTypeFromString(std::string key) {
        if (String::equal(key, "vertex")) return ShaderTypeEnum::Vertex;
        if (String::equal(key, "fragment")) return ShaderTypeEnum::Fragment;
        if (String::equal(key, "geometry")) return ShaderTypeEnum::Geometry;
        if (String::equal(key, "compute")) return ShaderTypeEnum::Compute;
        if (String::equal(key, "tessControl")) return ShaderTypeEnum::TessControl;
        if (String::equal(key, "tessEvaluation")) return ShaderTypeEnum::TessEvaluation;
        return ShaderTypeEnum::None;
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
        Amara::GameProps* gameProps = nullptr;
        
        #ifdef AMARA_OPENGL
        std::unordered_map<std::string, unsigned int> glShaders;
        std::unordered_map<std::string, ShaderProgram*> glPrograms;

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
        std::string readShader(std::string path, ShaderTypeEnum type) {
            std::string source = gameProps->system->readFile(path);
            return source;
}
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
            std::string versionHeader = "";

            if (!String::startsWith(source, "#version")) {
                
                #if defined(__ANDROID__) || defined(__IPHONEOS__)
                    if (type == ShaderTypeEnum::Compute) {
                        versionHeader = "#version 310 es\n";
                    } 
                    else if (type == ShaderTypeEnum::Geometry || 
                            type == ShaderTypeEnum::TessControl || 
                            type == ShaderTypeEnum::TessEvaluation) {
                        versionHeader = "#version 320 es\n";
                    } 
                    else {
                        versionHeader = "#version 300 es\n";
                        if (type == ShaderTypeEnum::Fragment) {
                            versionHeader += "precision highp float;\n";
                        }
                        if (type == ShaderTypeEnum::Vertex) {
                            versionHeader += "precision highp float;\n";
                        }
                    }

                #elif defined(__APPLE__)
                    versionHeader = "#version 410 core\n";

                #else
                    if (type == ShaderTypeEnum::Compute || 
                        type == ShaderTypeEnum::TessControl || 
                        type == ShaderTypeEnum::TessEvaluation) {
                        versionHeader = "#version 430 core\n";
                    } 
                    else if (type == ShaderTypeEnum::Geometry) {
                        versionHeader = "#version 330 core\n"; 
                    } 
                    else {
                        versionHeader = "#version 330 core\n";
                    }
                #endif

                if (!versionHeader.empty()) {
                    source.insert(0, versionHeader);
                }
            }
            
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
                    if (gameProps->system->exists(filePath)) {
                        std::string source = readShader(filePath, ShaderTypeEnum::Compute);
                        shaderID = compileGLShader("", source, ShaderTypeEnum::Compute);
                        temp = true;
                    }
                    else {
                        fatal_error("Error: Compute shader not found: ", shader_key);
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
                    if (gameProps->system->exists(filePath)) {
                        std::string source = readShader(filePath, ShaderTypeEnum::Vertex);
                        shaderID = compileGLShader("", source, ShaderTypeEnum::Vertex);
                        temp = true;
                    }
                    else {
                        fatal_error("Error: Vertex shader not found: ", shader_key);
                        return nullptr;
                    }
                }
                if (shaderID != 0) glAttachShader(shaderProgramID, shaderID);
                else {
                    fatal_error("Error: Unable to compile vertex shader: ", shader_key);
                    return nullptr;
                }
                if (temp) glDeleteShader(shaderID);
            }
            else if (!has_compute_shader) {
                fatal_error("Error: No vertex shader specified.");
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
                    if (gameProps->system->exists(filePath)) {
                        std::string source = readShader(filePath, ShaderTypeEnum::Fragment);
                        shaderID = compileGLShader("", source, ShaderTypeEnum::Fragment);
                        temp = true;
                    }
                    else {
                        fatal_error("Error: Fragment shader not found: ", shader_key);
                        return nullptr;
                    }
                }
                if (shaderID != 0) glAttachShader(shaderProgramID, shaderID);
                else {
                    fatal_error("Error: Unable to compile fragment shader: ", shader_key);
                    return nullptr;
                }
                if (temp) glDeleteShader(shaderID);
            }
            else if (!has_compute_shader) {
                fatal_error("Error: No fragment shader specified.");
                return nullptr;
            }
            
            auto config_items = config.items();
            std::vector<std::string> shader_types;
            for (auto it : config_items) {
                bool temp = false;
                ShaderTypeEnum type = shaderTypeFromString(it.key());

                if (type == ShaderTypeEnum::None) {
                    continue;
                }
                std::string shader_key = it.value();
                shader_types.push_back(it.key());

                unsigned int shaderID = 0;
                if (hasShader(shader_key)) {
                    std::string filePath = gameProps->system->getAssetPath(shader_key);
                    if (gameProps->system->exists(filePath)) {
                        std::string source = readShader(filePath, type);
                        shaderID = compileGLShader("", source, type);
                    }
                    else {
                        fatal_error("Error: Shader not found: ", shader_key);
                        return nullptr;
                    }
                }
                else if (!shader_key.empty()) {
                    fatal_error("Error: Shader not found: ", shader_key);
                    return nullptr;
                }
                if (shaderID != 0) glAttachShader(shaderProgramID, shaderID);
                else {
                    fatal_error("Error: Unable to compile ", it.key() ," shader: ", shader_key);
                    return nullptr;
                }
                if (temp) glDeleteShader(shaderID);
            }
            for (std::string key: shader_types) {
                config.erase(key);
            }
            
            glLinkProgram(shaderProgramID);

            GLint success;
            glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &success);
            if (!success) {
                GLint logLength;
                glGetProgramiv(shaderProgramID, GL_INFO_LOG_LENGTH, &logLength);
                char* log = new char[logLength];
                glGetProgramInfoLog(shaderProgramID, logLength, &logLength, log);
                fatal_error("Error: Shader Program Linking Failed: ", log);
                delete[] log;
                return 0;
            }

            ShaderProgram* newProgram = new ShaderProgram(shaderProgramID);
            newProgram->manager = this;
            newProgram->configure(config);
            newProgram->init(gameProps);
            
            return newProgram;
        }

        ShaderProgram* createShaderProgram(std::string key, nlohmann::json config) {
            if (gameProps->graphics != GraphicsEnum::OpenGL) {
                debug_log("Error: Cannot create shader program without an OpenGL context.");
                return nullptr;
            }

            ShaderProgram* shaderProgram = createShaderProgram(config);
            if (shaderProgram == nullptr) {
                fatal_error("Error: Failed to create shader program \"", key, "\".");
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
            std::string source = readShader(filePath, type);

            #ifdef AMARA_OPENGL
            unsigned int shader = compileGLShader(key, source, type);
            if (shader != 0) return true;
            #endif

            debug_log("Error: Failed to load shader from file: ", path);
            return false;
        }
        
        static void bind_lua(sol::state& lua) {
            lua.new_usertype<ShaderManager>("ShaderManager",
                #ifdef AMARA_OPENGL
                "createShaderProgram", sol::resolve<ShaderProgram*(std::string, sol::object)>(&ShaderManager::createShaderProgram),
                "hasShaderProgram", &ShaderManager::hasShaderProgram,
                "getShaderProgram", &ShaderManager::getShaderProgram,
                #endif
                "hasShader", &ShaderManager::hasShader,
                "loadShader", &ShaderManager::loadShader
            );
        }
    };

    #ifdef AMARA_OPENGL
    void ShaderProgram::configure(nlohmann::json config) {
        for (auto it = config.begin(); it != config.end(); ++it) {
            setUniform(it.key(), it.value());
        }
    }

    Amara::ShaderProgram* AssetManager::getShaderProgram(std::string key) {
        return gameProps->shaders->getShaderProgram(key);
    }
    #endif
}