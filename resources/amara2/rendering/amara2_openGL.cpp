namespace Amara {
    // Function pointers for OpenGL
    typedef void (*GL_GENBUFFERS)(GLsizei, GLuint*);
    typedef void (*GL_BINDBUFFER)(GLenum, GLuint);
    typedef void (*GL_BUFFERDATA)(GLenum, GLsizeiptr, const void*, GLenum);
    typedef void (*GL_GENVERTEXARRAYS)(GLsizei, GLuint*);
    typedef void (*GL_BINDVERTEXARRAY)(GLuint);
    typedef void (*GL_VERTEXATTRIBPOINTER)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
    typedef void (*GL_ENABLEVERTEXATTRIBARRAY)(GLuint);
    typedef void (*GL_DRAWELEMENTS)(GLenum, GLsizei, GLenum, const void*);

    // OpenGL function pointers
    GL_GENBUFFERS glGenBuffers;
    GL_BINDBUFFER glBindBuffer;
    GL_BUFFERDATA glBufferData;
    GL_GENVERTEXARRAYS glGenVertexArrays;
    GL_BINDVERTEXARRAY glBindVertexArray;
    GL_VERTEXATTRIBPOINTER glVertexAttribPointer;
    GL_ENABLEVERTEXATTRIBARRAY glEnableVertexAttribArray;
    GL_DRAWELEMENTS glDrawElements;

    // Vertex data (x, y, s, t)
    GLfloat vertices[] = {
        // Positions    // Tex Coords
        0.5f,  0.5f,   1.0f, 1.0f,  // Top Right
        0.5f, -0.5f,   1.0f, 0.0f,  // Bottom Right
        -0.5f, -0.5f,   0.0f, 0.0f,  // Bottom Left
        -0.5f,  0.5f,   0.0f, 1.0f   // Top Left
    };

    // Index data (EBO)
    GLuint indices[] = {
        0, 1, 2,   // First Triangle
        2, 3, 0    // Second Triangle
    };

    // Load OpenGL functions using SDL
    void LoadOpenGLFunctions() {
        glGenBuffers = (GL_GENBUFFERS)SDL_GL_GetProcAddress("glGenBuffers");
        glBindBuffer = (GL_BINDBUFFER)SDL_GL_GetProcAddress("glBindBuffer");
        glBufferData = (GL_BUFFERDATA)SDL_GL_GetProcAddress("glBufferData");
        glGenVertexArrays = (GL_GENVERTEXARRAYS)SDL_GL_GetProcAddress("glGenVertexArrays");
        glBindVertexArray = (GL_BINDVERTEXARRAY)SDL_GL_GetProcAddress("glBindVertexArray");
        glVertexAttribPointer = (GL_VERTEXATTRIBPOINTER)SDL_GL_GetProcAddress("glVertexAttribPointer");
        glEnableVertexAttribArray = (GL_ENABLEVERTEXATTRIBARRAY)SDL_GL_GetProcAddress("glEnableVertexAttribArray");
        glDrawElements = (GL_DRAWELEMENTS)SDL_GL_GetProcAddress("glDrawElements");
    }

    // Load texture using stb_image
    GLuint LoadGLTexture(const char *filePath) {
        int width, height, channels;
        unsigned char *data = stbi_load(filePath, &width, &height, &channels, 4);  // Force 4 channels (RGBA)
        if (!data) {
            printf("Failed to load image: %s\n", filePath);
            return 0;
        }

        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        // Texture settings
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);  // Free image memory
        return textureID;
    }

}