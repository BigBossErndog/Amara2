namespace Amara { 
    #ifdef AMARA_OPENGL
    
    class ShaderProgram {
    public:
        bool destroyed = false;

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
        
        virtual void destroy() {
            if (destroyed) return;
            glDeleteProgram(programID);

            destroyed = true;
            programID = 0;
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<ShaderProgram>("ShaderProgram",
                "destroy", &ShaderProgram::destroy,
                "key", &ShaderProgram::key
            );
        }
    };
    #endif
}