// gl_api.h - the subset of OpenGL 4.1 core the renderer uses, loaded at runtime.
//
// Windows only ships OpenGL 1.1 headers, so instead of depending on a generated
// loader we declare exactly what we call and resolve it through the platform's
// GetProcAddress once a context exists.
#pragma once

#include <cstddef>
#include <cstdint>

#if defined(_WIN32)
#define RR_GL_APIENTRY __stdcall
#else
#define RR_GL_APIENTRY
#endif

namespace rr::render::gl {

using GLenum = unsigned int;
using GLboolean = unsigned char;
using GLbitfield = unsigned int;
using GLint = int;
using GLuint = unsigned int;
using GLsizei = int;
using GLfloat = float;
using GLchar = char;
using GLubyte = unsigned char;
using GLsizeiptr = std::ptrdiff_t;
using GLintptr = std::ptrdiff_t;

// Enumerant values from the OpenGL specification.
inline constexpr GLenum GL_FALSE = 0;
inline constexpr GLenum GL_TRUE = 1;
inline constexpr GLenum GL_NO_ERROR = 0;
inline constexpr GLenum GL_TRIANGLES = 0x0004;
inline constexpr GLenum GL_LINES = 0x0001;
inline constexpr GLenum GL_UNSIGNED_BYTE = 0x1401;
inline constexpr GLenum GL_UNSIGNED_SHORT = 0x1403;
inline constexpr GLenum GL_UNSIGNED_INT = 0x1405;
inline constexpr GLenum GL_FLOAT = 0x1406;
inline constexpr GLenum GL_RGBA = 0x1908;
inline constexpr GLenum GL_COLOR_BUFFER_BIT = 0x00004000;
inline constexpr GLenum GL_DEPTH_BUFFER_BIT = 0x00000100;
inline constexpr GLenum GL_DEPTH_TEST = 0x0B71;
inline constexpr GLenum GL_CULL_FACE = 0x0B44;
inline constexpr GLenum GL_BLEND = 0x0BE2;
inline constexpr GLenum GL_BACK = 0x0405;
inline constexpr GLenum GL_FRONT = 0x0404;
inline constexpr GLenum GL_CCW = 0x0901;
inline constexpr GLenum GL_LEQUAL = 0x0203;
inline constexpr GLenum GL_SRC_ALPHA = 0x0302;
inline constexpr GLenum GL_ONE_MINUS_SRC_ALPHA = 0x0303;
inline constexpr GLenum GL_ARRAY_BUFFER = 0x8892;
inline constexpr GLenum GL_ELEMENT_ARRAY_BUFFER = 0x8893;
inline constexpr GLenum GL_UNIFORM_BUFFER = 0x8A11;
inline constexpr GLenum GL_STATIC_DRAW = 0x88E4;
inline constexpr GLenum GL_DYNAMIC_DRAW = 0x88E8;
inline constexpr GLenum GL_FRAGMENT_SHADER = 0x8B30;
inline constexpr GLenum GL_VERTEX_SHADER = 0x8B31;
inline constexpr GLenum GL_COMPILE_STATUS = 0x8B81;
inline constexpr GLenum GL_LINK_STATUS = 0x8B82;
inline constexpr GLenum GL_INFO_LOG_LENGTH = 0x8B84;
inline constexpr GLenum GL_VENDOR = 0x1F00;
inline constexpr GLenum GL_RENDERER = 0x1F01;
inline constexpr GLenum GL_VERSION = 0x1F02;
inline constexpr GLenum GL_PACK_ALIGNMENT = 0x0D05;
inline constexpr GLuint GL_INVALID_INDEX = 0xFFFFFFFFu;

// Every function we use: X(return type, name, parameter list).
#define RR_GL_FUNCTIONS(X)                                                                         \
    X(void, glClearColor, (GLfloat, GLfloat, GLfloat, GLfloat))                                    \
    X(void, glClearDepthf, (GLfloat))                                                              \
    X(void, glClear, (GLbitfield))                                                                 \
    X(void, glViewport, (GLint, GLint, GLsizei, GLsizei))                                          \
    X(void, glEnable, (GLenum))                                                                    \
    X(void, glDisable, (GLenum))                                                                   \
    X(void, glDepthFunc, (GLenum))                                                                 \
    X(void, glDepthMask, (GLboolean))                                                              \
    X(void, glCullFace, (GLenum))                                                                  \
    X(void, glFrontFace, (GLenum))                                                                 \
    X(void, glBlendFunc, (GLenum, GLenum))                                                         \
    X(void, glPixelStorei, (GLenum, GLint))                                                        \
    X(void, glReadPixels, (GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void*))                 \
    X(const GLubyte*, glGetString, (GLenum))                                                       \
    X(GLenum, glGetError, ())                                                                      \
    X(void, glGenBuffers, (GLsizei, GLuint*))                                                      \
    X(void, glDeleteBuffers, (GLsizei, const GLuint*))                                             \
    X(void, glBindBuffer, (GLenum, GLuint))                                                        \
    X(void, glBufferData, (GLenum, GLsizeiptr, const void*, GLenum))                               \
    X(void, glBufferSubData, (GLenum, GLintptr, GLsizeiptr, const void*))                          \
    X(void, glBindBufferBase, (GLenum, GLuint, GLuint))                                            \
    X(void, glGenVertexArrays, (GLsizei, GLuint*))                                                 \
    X(void, glDeleteVertexArrays, (GLsizei, const GLuint*))                                        \
    X(void, glBindVertexArray, (GLuint))                                                           \
    X(void, glEnableVertexAttribArray, (GLuint))                                                   \
    X(void, glDisableVertexAttribArray, (GLuint))                                                  \
    X(void, glVertexAttribPointer, (GLuint, GLint, GLenum, GLboolean, GLsizei, const void*))       \
    X(GLuint, glCreateShader, (GLenum))                                                            \
    X(void, glDeleteShader, (GLuint))                                                              \
    X(void, glShaderSource, (GLuint, GLsizei, const GLchar* const*, const GLint*))                 \
    X(void, glCompileShader, (GLuint))                                                             \
    X(void, glGetShaderiv, (GLuint, GLenum, GLint*))                                               \
    X(void, glGetShaderInfoLog, (GLuint, GLsizei, GLsizei*, GLchar*))                              \
    X(GLuint, glCreateProgram, ())                                                                 \
    X(void, glDeleteProgram, (GLuint))                                                             \
    X(void, glAttachShader, (GLuint, GLuint))                                                      \
    X(void, glLinkProgram, (GLuint))                                                               \
    X(void, glGetProgramiv, (GLuint, GLenum, GLint*))                                              \
    X(void, glGetProgramInfoLog, (GLuint, GLsizei, GLsizei*, GLchar*))                             \
    X(void, glUseProgram, (GLuint))                                                                \
    X(GLuint, glGetUniformBlockIndex, (GLuint, const GLchar*))                                     \
    X(void, glUniformBlockBinding, (GLuint, GLuint, GLuint))                                       \
    X(void, glDrawArrays, (GLenum, GLint, GLsizei))                                                \
    X(void, glDrawElementsBaseVertex, (GLenum, GLsizei, GLenum, const void*, GLint))

struct Api {
#define RR_GL_DECLARE(ret, name, params) ret(RR_GL_APIENTRY* name) params = nullptr;
    RR_GL_FUNCTIONS(RR_GL_DECLARE)
#undef RR_GL_DECLARE

    // Resolves every entry point. Returns the name of the first missing one, or
    // null on success.
    template <typename Loader> const char* Load(Loader&& getProc) {
#define RR_GL_LOAD(ret, name, params)                                                              \
    name = reinterpret_cast<ret(RR_GL_APIENTRY*) params>(getProc(#name));                          \
    if (!name) return #name;
        RR_GL_FUNCTIONS(RR_GL_LOAD)
#undef RR_GL_LOAD
        return nullptr;
    }
};

} // namespace rr::render::gl
