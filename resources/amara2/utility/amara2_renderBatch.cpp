namespace Amara {
    class RenderBatch {
    public:
        #ifdef AMARA_OPENGL
        unsigned int VAO, VBO, EBO;
        GLuint glTextureID = 0;
        #endif

        RenderBatchType type = RenderBatchType::None;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

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
        void pushQuad(GLuint _glTextureID, const std::vector<float> _v) {
            if (glTextureID != _glTextureID) {
                flush();
                glTextureID = _glTextureID;
            }

            vertices.insert(vertices.end(), _v.begin(), _v.end());

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
            if (glTextureID != 0 && Props::graphics == Amara::GraphicsEnum::Render2D && Props::glContext != NULL) {

            }
            #endif

            vertices.clear();
            indices.clear();
        }
    };
}