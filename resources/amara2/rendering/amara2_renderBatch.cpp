namespace Amara {
    class RenderBatch {
    public:
        #ifdef AMARA_OPENGL
        GLuint VAO, VBO, EBO, alphaBO;
        GLuint glTextureID = 0;

        ShaderProgram* shaderProgram = nullptr;
        #endif

        Amara::Rectangle viewport;
        bool insideFrameBuffer = false;

        Amara::BlendMode blendMode = Amara::BlendMode::Alpha;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        std::vector<float> alphas;
        
        int rec_buffer_size = 0;

        int offset = 0;

        bool external_buffers = false;
        
        RenderBatch() {}

        void init() {
            #ifdef AMARA_OPENGL
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);

            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

            // Position attribute
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            
            // Texture coordinate attribute
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

            glGenBuffers(1, &alphaBO);
            glBindBuffer(GL_ARRAY_BUFFER, alphaBO);

            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
            glVertexAttribDivisor(2, 1);

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            #endif
        }

        #ifdef AMARA_OPENGL
        void init(GLuint _vao, GLuint _vbo, GLuint _ebo) {
            VAO = _vao;
            VBO = _vbo;
            EBO = _ebo;

            external_buffers = true;
        }

        void newCycle() {
            vertices.clear();
            indices.clear();
            shaderProgram = nullptr;
            glTextureID = 0;
            offset = 0;
        }

        void pushQuad(
            ShaderProgram* _shaderProgram,
            GLuint _glTextureID, 
            const std::array<float, 16>& _vertices,
            float _alpha,
            const Rectangle& _viewport,
            bool _insideFrameBuffer,
            Amara::BlendMode _blendMode
        ) {
            if (
                shaderProgram != _shaderProgram ||
                glTextureID != _glTextureID ||
                viewport != _viewport ||
                insideFrameBuffer != _insideFrameBuffer ||
                blendMode != _blendMode
            ) {
                flush();
                if (shaderProgram != _shaderProgram) {
                    shaderProgram = _shaderProgram;
                    shaderProgram->applyShader();
                }
                glTextureID = _glTextureID;
                viewport = _viewport;
                insideFrameBuffer = _insideFrameBuffer;
                blendMode = _blendMode;
            }

            vertices.insert(vertices.end(), _vertices.begin(), _vertices.end());

            indices.push_back(offset + 0);
            indices.push_back(offset + 1);
            indices.push_back(offset + 2);
            indices.push_back(offset + 2);
            indices.push_back(offset + 3);
            indices.push_back(offset + 0);
            offset += 4;

            alphas.push_back(_alpha);
        }
        #endif

        void flush() {
            if (vertices.size() == 0) return;

            #ifdef AMARA_OPENGL
            if (glTextureID != 0 && Props::graphics == Amara::GraphicsEnum::OpenGL && Props::glContext != NULL) {
                glDisable(GL_DEPTH_TEST);
                
                if (!insideFrameBuffer) {
                    glViewport(
                        Props::window_dim.w - viewport.x - viewport.w, 
                        Props::window_dim.h - viewport.y - viewport.h, 
                        viewport.w, viewport.h
                    );
                }
                else glViewport(viewport.x, viewport.y, viewport.w, viewport.h);
                
                glBindVertexArray(VAO);

                bool buffer_size_changed = false;
                if (vertices.size() > rec_buffer_size) {
                    buffer_size_changed = true;
                    rec_buffer_size = vertices.size();
                }
                
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                if (buffer_size_changed) glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
                else glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
                
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                if (buffer_size_changed) glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);
                else glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(unsigned int), indices.data());

                glBindBuffer(GL_ARRAY_BUFFER, alphaBO);
                if (buffer_size_changed) glBufferData(GL_ARRAY_BUFFER, alphas.size() * sizeof(float), alphas.data(), GL_DYNAMIC_DRAW);
                else glBufferSubData(GL_ARRAY_BUFFER, 0, alphas.size() * sizeof(float), alphas.data());

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, glTextureID);

                GLint location = glGetUniformLocation(Props::currentShaderProgram->programID, "_texture");
                if (location == -1) {
                    debug_log("Error: Uniform '_texture' not found in shader: \"", Props::currentShaderProgram->key, "\".");
                }
                glUniform1i(location, 0);

                switch (blendMode) {
                    case BlendMode::Alpha:
                        glEnable(GL_BLEND);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                        break;
                    case BlendMode::Additive:
                        glEnable(GL_BLEND);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                        break;
                    case BlendMode::Multiply:
                        glEnable(GL_BLEND);
                        glBlendFunc(GL_DST_COLOR, GL_ZERO);
                        break;
                    case BlendMode::PremultipliedAlpha:
                        glEnable(GL_BLEND);
                        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                        break; 
                    default:
                        glDisable(GL_BLEND);
                        break;
                }

                // Draw the sprite
                glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

                // Unbind
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindVertexArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            }
            #endif

            vertices.clear();
            indices.clear();
            alphas.clear();
            offset = 0;

            glTextureID = 0;
        }

        void destroy() {
            #ifdef AMARA_OPENGL
            if (!external_buffers) {
                if (VAO) {
                    glDeleteVertexArrays(1, &VAO);
                    VAO = 0;
                }
                if (VBO) {
                    glDeleteBuffers(1, &VBO);
                    VBO = 0;
                }
                if (EBO) {
                    glDeleteBuffers(1, &EBO);
                    EBO = 0;
                }
                if (alphaBO) {
                    glDeleteBuffers(1, &alphaBO);
                    alphaBO = 0;
                }
            }
            #endif
        }
    };
}