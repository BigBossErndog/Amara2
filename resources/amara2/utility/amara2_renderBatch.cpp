namespace Amara {
    class RenderBatch {
    public:
        #ifdef AMARA_OPENGL
        unsigned int VAO, VBO, EBO;
        GLuint glTextureID = 0;
        #endif

        Rectangle viewport;
        BlendMode blendMode = BlendMode::Alpha;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        int rec_buffer_size = 0;

        int offset = 0;
        
        RenderBatch() {}

        void init() {
            #ifdef AMARA_OPENGL
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);
            #endif
        }

        #ifdef AMARA_OPENGL
        void pushQuad(
            GLuint _glTextureID, 
            const std::array<float, 16> _vertices,
            const Rectangle& _viewport,
            BlendMode _blendMode
        ) {
            if (
                glTextureID != _glTextureID ||
                viewport != _viewport ||
                blendMode != _blendMode
            ) {
                flush();
                glTextureID = _glTextureID;
                viewport = _viewport;
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
        }
        #endif

        void flush() {
            if (vertices.size() == 0) return;

            #ifdef AMARA_OPENGL
            if (glTextureID != 0 && Props::graphics == Amara::GraphicsEnum::OpenGL && Props::glContext != NULL) {
                glViewport(viewport.x, Props::window_dim.h - viewport.y - viewport.h, viewport.w, viewport.h);
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

                // Position attribute
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);

                // Texture coordinate attribute
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
                glEnableVertexAttribArray(1);
                
                GLint location = glGetUniformLocation(Props::currentShaderProgram->programID, "_texture");
                if (location == -1) {
                    debug_log("Error: Uniform '_texture' not found in shader: \"", Props::currentShaderProgram->key, "\".");
                }
                glUniform1i(location, 0);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, glTextureID);
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
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                // Draw the sprite
                glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

                // Unbind VAO
                glBindVertexArray(0);
            }
            #endif

            vertices.clear();
            indices.clear();
            offset = 0;

            glTextureID = 0;
        }
    };
}