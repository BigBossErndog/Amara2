#include <array> // Ensure this is included

namespace Amara {
    class RenderBatch {
    public:
        #ifdef AMARA_OPENGL
        // Use separate VBOs for clarity, or interleave if preferred later
        GLuint VAO = 0, VBO = 0, EBO = 0, alphaVBO = 0, tintVBO = 0;
        GLuint glTextureID = 0;

        ShaderProgram* shaderProgram = nullptr;
        #endif

        Amara::Rectangle viewport;
        bool insideFrameBuffer = false;

        Amara::BlendMode blendMode = Amara::BlendMode::Alpha;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        std::vector<float> alphas_per_vertex;
        std::vector<float> tints_per_vertex;

        size_t rec_buffer_size_vbo = 0;
        size_t rec_buffer_size_alpha = 0;
        size_t rec_buffer_size_tint = 0;
        size_t rec_buffer_size_ebo = 0;

        int vertex_offset = 0;

        RenderBatch() {}

        void init() {
            #ifdef AMARA_OPENGL
            if (Props::graphics != GraphicsEnum::OpenGL || Props::glContext == NULL) return;

            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);

            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

            glGenBuffers(1, &alphaVBO);
            glBindBuffer(GL_ARRAY_BUFFER, alphaVBO);

            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);

            glGenBuffers(1, &tintVBO);
            glBindBuffer(GL_ARRAY_BUFFER, tintVBO);

            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            #endif
        }

        #ifdef AMARA_OPENGL
        void newCycle() {
            vertices.clear();
            indices.clear();
            alphas_per_vertex.clear();
            tints_per_vertex.clear();
            shaderProgram = nullptr;
            glTextureID = 0;
            vertex_offset = 0;
        }

        void pushQuad(
            ShaderProgram* _shaderProgram,
            GLuint _glTextureID,
            const std::array<float, 16>& _vertices,
            float _alpha,
            const Color& _tint,
            const Amara::Rectangle& _viewport,
            bool _insideFrameBuffer,
            Amara::BlendMode _blendMode
        ) {
            if (vertices.empty() ||
                shaderProgram != _shaderProgram ||
                glTextureID != _glTextureID ||
                viewport != _viewport ||
                insideFrameBuffer != _insideFrameBuffer ||
                blendMode != _blendMode
            ) {
                flush();

                shaderProgram = _shaderProgram;
                if (shaderProgram) {
                    shaderProgram->applyShader();
                } else {
                    shaderProgram = Props::defaultShaderProgram;
                    if (shaderProgram) shaderProgram->applyShader();
                    else {
                         debug_log("Error: No valid shader program available in RenderBatch::pushQuad.");
                         Props::breakWorld();
                         return;
                    }
                }
                glTextureID = _glTextureID;
                viewport = _viewport;
                insideFrameBuffer = _insideFrameBuffer;
                blendMode = _blendMode;
            }

            vertices.insert(vertices.end(), _vertices.begin(), _vertices.end());

            for (int i = 0; i < 4; ++i) {
                alphas_per_vertex.push_back(_alpha);
            }

            float r = _tint.r / 255.0f;
            float g = _tint.g / 255.0f;
            float b = _tint.b / 255.0f;
            float a = _tint.a / 255.0f;
            for (int i = 0; i < 4; ++i) {
                tints_per_vertex.push_back(r);
                tints_per_vertex.push_back(g);
                tints_per_vertex.push_back(b);
                tints_per_vertex.push_back(a);
            }

            indices.push_back(vertex_offset + 0);
            indices.push_back(vertex_offset + 1);
            indices.push_back(vertex_offset + 2);
            indices.push_back(vertex_offset + 2);
            indices.push_back(vertex_offset + 3);
            indices.push_back(vertex_offset + 0);

            vertex_offset += 4;
        }
        #endif

        void flush() {
            if (vertices.empty() || indices.empty()) return;

            #ifdef AMARA_OPENGL

            if (glTextureID != 0 && shaderProgram != nullptr && Props::graphics == Amara::GraphicsEnum::OpenGL && Props::glContext != NULL) {
                glDisable(GL_DEPTH_TEST);

                if (!insideFrameBuffer && Props::current_window != nullptr) {
                     int window_h = static_cast<int>(Props::window_dim.h);
                     glViewport(
                         static_cast<GLint>(viewport.x),
                         window_h - static_cast<GLint>(viewport.y + viewport.h),
                         static_cast<GLsizei>(viewport.w),
                         static_cast<GLsizei>(viewport.h)
                     );
                }
                else {
                    glViewport(
                        static_cast<GLint>(viewport.x),
                        static_cast<GLint>(viewport.y),
                        static_cast<GLsizei>(viewport.w),
                        static_cast<GLsizei>(viewport.h)
                    );
                }
                
                glBindVertexArray(VAO);

                size_t required_vbo_size = vertices.size() * sizeof(float);
                size_t required_alpha_size = alphas_per_vertex.size() * sizeof(float);
                size_t required_tint_size = tints_per_vertex.size() * sizeof(float);
                size_t required_ebo_size = indices.size() * sizeof(unsigned int);

                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                if (required_vbo_size > rec_buffer_size_vbo) {
                    glBufferData(GL_ARRAY_BUFFER, required_vbo_size, vertices.data(), GL_DYNAMIC_DRAW);
                    rec_buffer_size_vbo = required_vbo_size;
                }
                else {
                    glBufferSubData(GL_ARRAY_BUFFER, 0, required_vbo_size, vertices.data());
                }

                glBindBuffer(GL_ARRAY_BUFFER, alphaVBO);
                 if (required_alpha_size > rec_buffer_size_alpha) {
                    glBufferData(GL_ARRAY_BUFFER, required_alpha_size, alphas_per_vertex.data(), GL_DYNAMIC_DRAW);
                    rec_buffer_size_alpha = required_alpha_size;
                }
                else {
                    glBufferSubData(GL_ARRAY_BUFFER, 0, required_alpha_size, alphas_per_vertex.data());
                }

                glBindBuffer(GL_ARRAY_BUFFER, tintVBO);
                if (required_tint_size > rec_buffer_size_tint) {
                    glBufferData(GL_ARRAY_BUFFER, required_tint_size, tints_per_vertex.data(), GL_DYNAMIC_DRAW);
                    rec_buffer_size_tint = required_tint_size;
                }
                else {
                    glBufferSubData(GL_ARRAY_BUFFER, 0, required_tint_size, tints_per_vertex.data());
                }

                if (required_ebo_size > rec_buffer_size_ebo) {
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, required_ebo_size, indices.data(), GL_DYNAMIC_DRAW);
                    rec_buffer_size_ebo = required_ebo_size;
                } 
                else {
                    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, required_ebo_size, indices.data());
                }
                
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, glTextureID);

                GLint location = glGetUniformLocation(shaderProgram->programID, "_texture");
                if (location != -1) {
                    glUniform1i(location, 0);
                }

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

                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

                glBindVertexArray(0);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            #endif

            vertices.clear();
            indices.clear();
            alphas_per_vertex.clear();
            tints_per_vertex.clear();
            vertex_offset = 0;
        }

        void destroy() {
            #ifdef AMARA_OPENGL
            if (Props::graphics == GraphicsEnum::OpenGL && Props::glContext != NULL) {
                glBindVertexArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

                if (VAO) { glDeleteVertexArrays(1, &VAO); VAO = 0; }
                if (VBO) { glDeleteBuffers(1, &VBO); VBO = 0; }
                if (EBO) { glDeleteBuffers(1, &EBO); EBO = 0; }
                if (alphaVBO) { glDeleteBuffers(1, &alphaVBO); alphaVBO = 0; }
                if (tintVBO) { glDeleteBuffers(1, &tintVBO); tintVBO = 0; }

            } else {
                 VAO = VBO = EBO = alphaVBO = tintVBO = 0;
            }
            #endif

            vertices.clear();
            indices.clear();
            alphas_per_vertex.clear();
            tints_per_vertex.clear();
            rec_buffer_size_vbo = 0;
            rec_buffer_size_alpha = 0;
            rec_buffer_size_tint = 0;
            rec_buffer_size_ebo = 0;
            vertex_offset = 0;
        }
    };
}
