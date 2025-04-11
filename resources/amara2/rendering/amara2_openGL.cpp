#include <SDL3/SDL_opengl.h>

namespace Amara {
    #define GL_FUNCTION_LIST \
    X(void, glGenBuffers, GLsizei, GLuint*) \
    X(void, glBindVertexArray, GLuint) \
    X(void, glGenVertexArrays, GLsizei, GLuint*) \
    X(void, glShaderSource, GLuint, GLsizei, const GLchar**, const GLint*) \
    X(void, glCompileShader, GLuint) \
    X(GLuint, glCreateShader, GLenum) \
    X(void, glLinkProgram, GLuint) \
    X(void, glUseProgram, GLuint) \
    X(void, glGetShaderiv, GLuint, GLenum, GLint*) \
    X(void, glGetShaderInfoLog, GLuint, GLsizei, GLsizei*, GLchar*) \
    X(void, glAttachShader, GLuint, GLuint) \
    X(GLuint, glCreateProgram, void) \
    X(void, glDrawArrays, GLenum, GLint, GLsizei) \
    X(void, glEnableVertexAttribArray, GLuint) \
    X(void, glVertexAttribPointer, GLuint, GLint, GLenum, GLboolean, GLsizei, const void*) \
    X(void, glClearColor, GLfloat, GLfloat, GLfloat, GLfloat) \
    X(void, glClear, GLbitfield) \
    X(void, glViewport, GLint, GLint, GLsizei, GLsizei) \
    X(void, glDeleteTextures, GLsizei, const GLuint*) \
    X(void, glDeleteProgram, GLuint) \
    X(void, glDeleteShader, GLuint) \
    X(void, glDeleteBuffers, GLsizei, const GLuint*) \
    X(void, glDeleteVertexArrays, GLsizei, const GLuint*) \
    X(void, glGenTextures, GLsizei, GLuint*) \
    X(void, glBindTexture, GLenum, GLuint) \
    X(void, glTexImage2D, GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*) \
    X(void, glTexParameteri, GLenum, GLenum, GLint) \
    X(GLint, glGetUniformLocation, GLuint, const GLchar*) \
    X(void, glUniformMatrix4fv, GLint, GLsizei, GLboolean, const GLfloat*) \
    X(void, glUniform1i, GLint, GLint) \
    X(void, glGenFramebuffers, GLsizei, GLuint*) \
    X(void, glBindFramebuffer, GLenum, GLuint) \
    X(void, glFramebufferTexture2D, GLenum, GLenum, GLenum, GLuint, GLint) \
    X(void, glDrawElements, GLenum, GLsizei, GLenum, const void*) \
    X(void, glBufferData, GLenum, GLsizeiptr, const void*, GLenum) \
    X(void, glBufferSubData, GLenum, GLintptr, GLsizeiptr, const void*) \
    X(void, glGenQueries, GLsizei, GLuint*) \
    X(void, glBeginQuery, GLenum, GLuint) \
    X(void, glEndQuery, GLenum) \
    X(void, glGetQueryObjectiv, GLuint, GLenum, GLint*) \
    X(void, glFenceSync, GLenum, GLbitfield) \
    X(void, glClientWaitSync, GLsync, GLbitfield, GLuint64) \
    X(void, glBindBuffer, GLenum, GLuint) \
    X(void, glDeleteFramebuffers, GLsizei, GLuint*) \
    X(void, glDeleteRenderbuffers, GLsizei, GLuint*) \
    X(void, glDetachShader, GLuint, GLuint) \
    X(void, glValidateProgram, GLuint) \
    X(void, glDeleteProgramPipelines, GLsizei, const GLuint*) \
    X(void, glUniform1f, GLint, GLfloat) \
    X(void, glUniform2f, GLint, GLfloat, GLfloat) \
    X(void, glUniform3f, GLint, GLfloat, GLfloat, GLfloat) \
    X(void, glUniform4f, GLint, GLfloat, GLfloat, GLfloat, GLfloat) \
    X(void, glUniform1fv, GLint, GLsizei, const GLfloat*) \
    X(void, glUniform2fv, GLint, GLsizei, const GLfloat*) \
    X(void, glUniform3fv, GLint, GLsizei, const GLfloat*) \
    X(void, glUniform4fv, GLint, GLsizei, const GLfloat*) \
    X(void, glUniform1iv, GLint, GLsizei, const GLint*) \
    X(void, glUniform2iv, GLint, GLsizei, const GLint*) \
    X(void, glUniform3iv, GLint, GLsizei, const GLint*) \
    X(void, glUniform4iv, GLint, GLsizei, const GLint*) \
    X(GLenum, glCheckFramebufferStatus, GLenum) \
    X(void, glGenRenderbuffers, GLsizei, GLuint*) \
    X(void, glBindRenderbuffer, GLenum, GLuint) \
    X(void, glRenderbufferStorage, GLenum, GLenum, GLsizei, GLsizei) \
    X(void, glFramebufferRenderbuffer, GLenum, GLenum, GLenum, GLuint) \
    X(void, glTexSubImage2D, GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*) \
    X(void, glTexStorage2D, GLenum, GLsizei, GLenum, GLsizei, GLsizei) \
    X(void, glGenerateMipmap, GLenum) \
    X(void, glGetQueryObjectui64v, GLuint, GLenum, GLuint64*) \
    X(void, glWaitSync, GLsync, GLbitfield, GLuint64) \
    X(void, glDispatchCompute, GLuint, GLuint, GLuint) \
    X(void, glBindImageTexture, GLuint, GLuint, GLint, GLboolean, GLint, GLenum, GLenum) \
    X(void, glMemoryBarrier, GLbitfield) \
    X(void*, glMapBuffer, GLenum, GLenum) \
    X(GLboolean, glUnmapBuffer, GLenum) \
    X(void, glCopyBufferSubData, GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr) \
    X(void, glBindBufferBase, GLenum, GLuint, GLuint) \
    X(void, glBindBufferRange, GLenum, GLuint, GLuint, GLintptr, GLsizeiptr) \
    X(void, glGetProgramiv, GLuint, GLenum, GLint*) \
    X(void, glGetProgramInfoLog, GLuint, GLsizei, GLsizei*, GLchar*) \
    X(void, glGetActiveUniform, GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*) \
    X(void, glGetActiveAttrib, GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*) \
    X(GLint, glGetAttribLocation, GLuint, const GLchar*) \
    X(void, glEnable, GLenum) \
    X(void, glDisable, GLenum) \
    X(void, glBlendFunc, GLenum, GLenum) \
    X(void, glDepthFunc, GLenum) \
    X(void, glStencilFunc, GLenum, GLint, GLuint) \
    X(void, glStencilOp, GLenum, GLenum, GLenum) \
    X(void, glPolygonMode, GLenum, GLenum) \
    X(void, glCullFace, GLenum) \
    X(void, glFrontFace, GLenum) \
    X(void, glActiveTexture, GLenum) \
    X(void, glTexStorage3D, GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei) \
    X(void, glTexSubImage3D, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void*) \
    X(void, glDispatchComputeIndirect, GLintptr) \
    X(void, glPushDebugGroup, GLenum, GLuint, GLsizei, const GLchar*) \
    X(void, glPopDebugGroup, void) \
    X(GLboolean, glIsBuffer, GLuint) \
    X(GLboolean, glIsVertexArray, GLuint) \
    X(void, glVertexAttribDivisor, GLuint, GLuint) \
    X(void, glVertexAttribIPointer, GLuint, GLint, GLenum, GLsizei, const void*) \
    X(void, glGetShaderSource, GLuint, GLsizei, GLsizei*, GLchar*) \
    X(void, glGetUniformfv, GLuint, GLint, GLfloat*) \
    X(void, glGetUniformiv, GLuint, GLint, GLint*) \
    X(void, glGetUniformuiv, GLuint, GLint, GLuint*) \
    X(void, glGetUniformdv, GLuint, GLint, GLdouble*) \
    X(void, glGetActiveUniformBlockiv, GLuint, GLuint, GLenum, GLint*) \
    X(void, glGetActiveUniformBlockName, GLuint, GLuint, GLsizei, GLsizei*, GLchar*) \
    X(void, glGetActiveUniformsiv, GLuint, GLsizei, const GLuint*, GLenum, GLint*) \
    X(void, glUniformBlockBinding, GLuint, GLuint, GLuint) \
    X(void, glShaderBinary, GLsizei, const GLuint*, GLenum, const void*, GLsizei) \
    X(void, glProgramBinary, GLuint, GLenum, const void*, GLsizei) \
    X(void, glGetProgramBinary, GLuint, GLsizei, GLsizei*, GLenum*, void*) \
    X(void, glDrawArraysInstanced, GLenum, GLint, GLsizei, GLsizei) \
    X(void, glDrawElementsInstanced, GLenum, GLsizei, GLenum, const void*, GLsizei) \
    X(void, glDrawRangeElements, GLenum, GLuint, GLuint, GLsizei, GLenum, const void*) \
    X(void, glDrawTransformFeedback, GLenum, GLuint) \
    X(void, glDrawTransformFeedbackInstanced, GLenum, GLuint, GLsizei) \
    X(void, glDrawTransformFeedbackStream, GLenum, GLuint, GLuint) \
    X(void, glCreateBuffers, GLsizei, GLuint*) \
    X(void, glNamedBufferStorage, GLuint, GLsizeiptr, const void*, GLbitfield) \
    X(void, glNamedBufferData, GLuint, GLsizeiptr, const void*, GLenum) \
    X(void, glNamedBufferSubData, GLuint, GLintptr, GLsizeiptr, const void*) \
    X(void, glCopyNamedBufferSubData, GLuint, GLuint, GLintptr, GLintptr, GLsizeiptr) \
    X(void, glClearNamedBufferData, GLuint, GLenum, GLenum, GLenum, const void*) \
    X(void, glClearNamedBufferSubData, GLuint, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void*) \
    X(void*, glMapNamedBuffer, GLuint, GLenum) \
    X(void*, glMapNamedBufferRange, GLuint, GLintptr, GLsizeiptr, GLbitfield) \
    X(GLboolean, glUnmapNamedBuffer, GLuint) \
    X(void, glFlushMappedNamedBufferRange, GLuint, GLintptr, GLsizeiptr) \
    X(void, glGetNamedBufferParameteriv, GLuint, GLenum, GLint*) \
    X(void, glGetNamedBufferParameteri64v, GLuint, GLenum, GLint64*) \
    X(void, glGetNamedBufferPointerv, GLuint, GLenum, void**) \
    X(void, glGetNamedBufferSubData, GLuint, GLintptr, GLsizeiptr, void*) \
    X(void, glCreateFramebuffers, GLsizei, GLuint*) \
    X(void, glNamedFramebufferRenderbuffer, GLuint, GLenum, GLenum, GLuint) \
    X(void, glNamedFramebufferParameteri, GLuint, GLenum, GLint) \
    X(void, glNamedFramebufferTexture, GLuint, GLenum, GLuint, GLint) \
    X(void, glNamedFramebufferTextureLayer, GLuint, GLenum, GLuint, GLint, GLint) \
    X(void, glNamedFramebufferDrawBuffer, GLuint, GLenum) \
    X(void, glNamedFramebufferDrawBuffers, GLuint, GLsizei, const GLenum*) \
    X(void, glNamedFramebufferReadBuffer, GLuint, GLenum) \
    X(void, glInvalidateNamedFramebufferData, GLuint, GLsizei, const GLenum*) \
    X(void, glInvalidateNamedFramebufferSubData, GLuint, GLsizei, const GLenum*, GLint, GLint, GLsizei, GLsizei) \
    X(void, glClearNamedFramebufferiv, GLuint, GLenum, GLint, const GLint*) \
    X(void, glClearNamedFramebufferuiv, GLuint, GLenum, GLint, const GLuint*) \
    X(void, glClearNamedFramebufferfv, GLuint, GLenum, GLint, const GLfloat*) \
    X(void, glClearNamedFramebufferfi, GLuint, GLenum, GLint, GLfloat, GLint) \
    X(GLenum, glCheckNamedFramebufferStatus, GLuint, GLenum) \
    X(void, glGetNamedFramebufferParameteriv, GLuint, GLenum, GLint*) \
    X(void, glGetNamedFramebufferAttachmentParameteriv, GLuint, GLenum, GLenum, GLint*) \
    X(void, glCreateRenderbuffers, GLsizei, GLuint*) \
    X(void, glNamedRenderbufferStorage, GLuint, GLenum, GLsizei, GLsizei) \
    X(void, glNamedRenderbufferStorageMultisample, GLuint, GLsizei, GLenum, GLsizei, GLsizei) \
    X(void, glGetNamedRenderbufferParameteriv, GLuint, GLenum, GLint*) \
    X(void, glCreateTextures, GLenum, GLsizei, GLuint*) \
    X(void, glTextureBuffer, GLuint, GLenum, GLuint) \
    X(void, glTextureBufferRange, GLuint, GLenum, GLuint, GLintptr, GLsizeiptr) \
    X(void, glTextureStorage1D, GLuint, GLsizei, GLenum, GLsizei) \
    X(void, glTextureStorage2D, GLuint, GLsizei, GLenum, GLsizei, GLsizei) \
    X(void, glTextureStorage3D, GLuint, GLsizei, GLenum, GLsizei, GLsizei, GLsizei) \
    X(void, glTextureStorage2DMultisample, GLuint, GLsizei, GLenum, GLsizei, GLsizei, GLboolean) \
    X(void, glTextureStorage3DMultisample, GLuint, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean) \
    X(void, glTextureSubImage1D, GLuint, GLint, GLint, GLsizei, GLenum, GLenum, const void*) \
    X(void, glTextureSubImage2D, GLuint, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*) \
    X(void, glTextureSubImage3D, GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void*) \
    X(void, glCompressedTextureSubImage1D, GLuint, GLint, GLint, GLsizei, GLenum, GLsizei, const void*) \
    X(void, glCompressedTextureSubImage2D, GLuint, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void*) \
    X(void, glCompressedTextureSubImage3D, GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void*) \
    X(void, glCopyTextureSubImage1D, GLuint, GLint, GLint, GLint, GLint, GLsizei) \
    X(void, glCopyTextureSubImage2D, GLuint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei) \
    X(void, glCopyTextureSubImage3D, GLuint, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei) \
    X(void, glTextureParameterf, GLuint, GLenum, GLfloat) \
    X(void, glTextureParameterfv, GLuint, GLenum, const GLfloat*) \
    X(void, glTextureParameteri, GLuint, GLenum, GLint) \
    X(void, glTextureParameterIiv, GLuint, GLenum, const GLint*) \
    X(void, glTextureParameterIuiv, GLuint, GLenum, const GLuint*) \
    X(void, glTextureParameteriv, GLuint, GLenum, const GLint*) \
    X(void, glGenerateTextureMipmap, GLuint) \
    X(void, glBindTextureUnit, GLuint, GLuint) \
    X(void, glGetTextureImage, GLuint, GLint, GLenum, GLenum, GLsizei, void*) \
    X(void, glGetCompressedTextureImage, GLuint, GLint, GLsizei, void*) \
    X(void, glGetTextureLevelParameterfv, GLuint, GLint, GLenum, GLfloat*) \
    X(void, glGetTextureLevelParameteriv, GLuint, GLint, GLenum, GLint*) \
    X(void, glGetTextureParameterfv, GLuint, GLenum, GLfloat*) \
    X(void, glGetTextureParameterIiv, GLuint, GLenum, GLint*) \
    X(void, glGetTextureParameterIuiv, GLuint, GLenum, GLuint*) \
    X(void, glGetTextureParameteriv, GLuint, GLenum, GLint*) \
    X(void, glCreateVertexArrays, GLsizei, GLuint*) \
    X(void, glDisableVertexArrayAttrib, GLuint, GLuint) \
    X(void, glEnableVertexArrayAttrib, GLuint, GLuint) \
    X(void, glVertexArrayElementBuffer, GLuint, GLuint) \
    X(void, glVertexArrayVertexBuffer, GLuint, GLuint, GLuint, GLintptr, GLsizei) \
    X(void, glVertexArrayVertexBuffers, GLuint, GLuint, GLsizei, const GLuint*, const GLintptr*, const GLsizei*) \
    X(void, glVertexArrayAttribBinding, GLuint, GLuint, GLuint) \
    X(void, glPixelStorei, GLenum, GLint) \
    X(void, glVertexArrayAttribFormat, GLuint, GLuint, GLint, GLenum, GLboolean, GLuint)

    #define X(ret, name, ...) typedef ret (*PFN_##name)(__VA_ARGS__); PFN_##name name = NULL;
    GL_FUNCTION_LIST
    #undef X

    void LoadOpenGLFunctions() {
        #define X(ret, name, ...) name = (PFN_##name)SDL_GL_GetProcAddress(#name);
        GL_FUNCTION_LIST
        #undef X

        #define X(ret, name, ...) if (!name) { printf("Failed to load %s\n", #name); exit(1); }
        GL_FUNCTION_LIST
        #undef X
    }

    template<typename... Args>
    void glPrintError(Args... args) {
        GLint error = glGetError();
        if (error != GL_NO_ERROR) {
            std::ostringstream ss;
            (ss << ... << args);
            std::cout << ss.str().c_str() << error << std::endl;
            return;
        }
    }

    void glMakeFrameBuffer(
        GLuint& glTextureID,
        GLuint& glBufferID,
        GLuint* glRenderBufferIDPtr,
        int width,
        int height
    ) {
        glGenFramebuffers(1, &glBufferID);

        glBindFramebuffer(GL_FRAMEBUFFER, glBufferID);

        glGenTextures(1, &glTextureID);
        glBindTexture(GL_TEXTURE_2D, glTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);    
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glTextureID, 0);

        if (glRenderBufferIDPtr) {
            // Creating depth buffer
            std::cout << "WTF" << std::endl;
            GLuint& glRenderBufferID = *glRenderBufferIDPtr;

            glGenRenderbuffers(1, &glRenderBufferID);
            glBindRenderbuffer(GL_RENDERBUFFER, glRenderBufferID);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, glRenderBufferID);
        }
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            debug_log("Error: Failed to create frame buffer.");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        if (glRenderBufferIDPtr) {
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }
    }
    void glMakeFrameBuffer(
        GLuint& glTextureID,
        GLuint& glBufferID,
        GLuint& glDepthBufferID,
        int width,
        int height
    ) {
        glMakeFrameBuffer(glTextureID, glBufferID, &glDepthBufferID, width, height);
    }
    void glMakeFrameBuffer(
        GLuint& glTextureID,
        GLuint& glBufferID,
        int width,
        int height
    ) {
        glMakeFrameBuffer(glTextureID, glBufferID, nullptr, width, height);
    };

    const char* defaultVertexShader = R"(
        #version 330 core

        layout (location = 0) in vec2 _position;
        layout (location = 1) in vec2 _coord;

        out vec2 texCoord;
        
        void main() {
            gl_Position = vec4(_position, 0.0, 1.0);
            texCoord = _coord;
        }
    )";
    const char* defaultFragmentShader= R"(
        #version 330 core

        in vec2 texCoord;
        uniform sampler2D _texture;

        out vec4 fragColor;

        void main() {
            fragColor = texture(_texture, texCoord);
        }
    )";
    const char* tintedFragmentShader= R"(
        #version 330 core

        in vec2 texCoord;
        uniform sampler2D _texture;
        uniform vec4 _tintColor;

        out vec4 fragColor;
        
        void main() {
            fragColor = texture(_texture, texCoord) * _tintColor;
        }
    )";
}