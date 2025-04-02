namespace Amara {
    enum class ShaderTypeEnum {
        Vertex = GL_VERTEX_SHADER,
        Fragment = GL_FRAGMENT_SHADER,
        Geometry = GL_GEOMETRY_SHADER,
        Compute = GL_COMPUTE_SHADER,
        TessControl = GL_TESS_CONTROL_SHADER,
        TessEvaluation = GL_TESS_EVALUATION_SHADER
    };

    #ifdef AMARA_OPENGL
    class ShaderProgram {
    public:
        ShaderProgram() = default;
        ShaderProgram(unsigned int id) : programID(id) {}
        unsigned int programID = 0;
    };
    #endif

    class ShaderManager {
    public:
        #ifdef AMARA_OPENGL
        std::unordered_map<std::string, unsigned int> glShaders;
        std::unordered_map<std::string, ShaderProgram> glPrograms;

        ShaderManager() {
            glShaders.clear();
            glPrograms.clear();
        }
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
            if (hasShaderProgram(key)) return &glPrograms[key];
            return nullptr;
        }

        unsigned int compileGLShader(std::string key, std::string source, ShaderTypeEnum type) {
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

        unsigned int createShaderProgram(std::string key, std::string vertex_key, std::string fragment_key) {
            unsigned int shaderProgram = glCreateProgram();
            unsigned int vertexShader, fragmentShader;

            bool tempVertex = false;
            bool tempFragment = false;

            if (hasShader(vertex_key)) {
                vertexShader = getShader(vertex_key);
            }
            else {
                std::string filePath = Props::files->getAssetPath(vertex_key);\
                if (Props::files->fileExists(filePath)) {
                    std::string source = Props::files->readFile(filePath);
                    vertexShader = compileGLShader("", source, ShaderTypeEnum::Vertex);
                    tempVertex = true;
                }
                else {
                    debug_log("Error: Vertex shader file not found: ", filePath);
                    return 0;
                }
            }

            if (hasShader(fragment_key)) {
                fragmentShader = getShader(fragment_key);
            }
            else {
                std::string filePath = Props::files->getAssetPath(fragment_key);
                if (Props::files->fileExists(filePath)) {
                    std::string source = Props::files->readFile(filePath);
                    fragmentShader = compileGLShader("", source, ShaderTypeEnum::Fragment);
                    tempFragment = true;
                }
                else {
                    debug_log("Error: Fragment shader file not found: ", filePath);
                    return 0;
                }
            }

            if (tempVertex) glDeleteShader(vertexShader);
            if (tempFragment) glDeleteShader(fragmentShader);

            glAttachShader(shaderProgram, vertexShader);
            glAttachShader(shaderProgram, fragmentShader);

            if (hasShaderProgram(key)) {
                ShaderProgram& existing = glPrograms[key];
                existing.programID = shaderProgram;
            }
            else {
                glPrograms[key] = ShaderProgram(shaderProgram);
            }

            return shaderProgram;
        }
        #endif

        void clear() {
            #ifdef AMARA_OPENGL
            for (auto& shader : glShaders) {
                glDeleteShader(shader.second);
            }
            for (auto& program : glPrograms) {
                glDeleteProgram(program.second.programID);
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
            
            std::string filePath = Props::files->getAssetPath(path);
            std::string source = Props::files->readFile(filePath);

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
                "createShaderProgram", &ShaderManager::createShaderProgram,
                "hasShaderProgram", &ShaderManager::hasShaderProgram,
                #endif
                "hasShader", &ShaderManager::hasShader,
                "loadShader", &ShaderManager::loadShader
            );
        }
    };
}