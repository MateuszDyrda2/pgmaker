#include <libpgmaker/preview.h>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace libpgmaker {
using namespace glm;
void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();
    if(err != GL_NO_ERROR)
    {
        printf("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
        abort();
    }
}

#define GL_CHECK(stmt)                               \
    do {                                             \
        stmt;                                        \
        CheckOpenGLError(#stmt, __FILE__, __LINE__); \
    }                                                \
    while(0)

preview::preview(const std::pair<std::uint32_t, std::uint32_t>& size):
    size(size), texture(0)
{
    initialize_shaders();
    initialize_vao();
    initialize_texture();
}
preview::~preview()
{
    drop_texture();
    drop_vao();
    drop_shaders();
}
void preview::resize(const std::pair<std::uint32_t, std::uint32_t>& newSize)
{
    size = newSize;
}
void preview::update(frame* newFrame)
{
    // TODO: use tex image only if new size is bigger
    GL_CHECK(glUseProgram(shaderProgram));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));
    if(newFrame->size != textureSize)
    {
        textureSize = newFrame->size;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSize.first, textureSize.second, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    }
    else
    {
        GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                                 textureSize.first, textureSize.second, GL_RGBA,
                                 GL_UNSIGNED_BYTE, newFrame->data.data()));
    }

    mat4 viewProj = ortho(float(size.first) * -0.5f, float(size.first) * 0.5f,
                          float(size.second) * 0.5f, float(size.second) * -0.5f);
    viewProj      = scale(viewProj,
                          vec3(float(newFrame->size.first),
                               float(newFrame->size.second), 1.f));
    GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "viewProj"), 1, GL_FALSE, value_ptr(viewProj)));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
    GL_CHECK(glUseProgram(0));
}
void preview::draw()
{
    GL_CHECK(glUseProgram(shaderProgram));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));
    GL_CHECK(glBindVertexArray(vao));
    GL_CHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
    GL_CHECK(glUseProgram(0));
}
void preview::initialize_texture()
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 size.first, size.second, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
void preview::initialize_shaders()
{
    const char* vertexShaderCode   = R"(
		#version 330 core
		layout (location = 0) in vec2 aPos;
		layout (location = 1) in vec2 aTexCoord;

		out vec2 TexCoord;
		uniform mat4 viewProj;

		void main()
		{
			gl_Position = viewProj * vec4(aPos, 0.0, 1.0);
			TexCoord = aTexCoord;
		}
	)";
    const char* fragmentShaderCode = R"(
		#version 330 core
		out vec4 FragColor;

		in vec2 TexCoord;

		uniform sampler2D ourTexture;

		void main()
		{
			FragColor = texture(ourTexture, TexCoord);
		}
	)";

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
    int isCompiled = 0;
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        char errorMsg[128];
        int len;
        glGetShaderInfoLog(vertexShader, sizeof(errorMsg), &len, errorMsg);
        printf("%s\n", errorMsg);
        glDeleteShader(vertexShader);
        return;
    }
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        char errorMsg[128];
        int len;
        glGetShaderInfoLog(fragmentShader, sizeof(errorMsg), &len, errorMsg);
        printf("%s\n", errorMsg);
        glDeleteShader(fragmentShader);
        return;
    }
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    int isLinked = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);
    if(isLinked == GL_FALSE)
    {
        char errorMsg[128];
        int len;
        glGetProgramInfoLog(shaderProgram, sizeof(errorMsg), &len, errorMsg);
        printf("%s\n", errorMsg);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glUseProgram(shaderProgram);
}
void preview::initialize_vao()
{
    float vertices[] = {
        0.5f, 0.5f, 1.f, 1.f,
        0.5f, -0.5f, 1.f, 0.f,
        -0.5f, -0.5f, 0.f, 0.f,
        -0.5f, 0.5f, 0.f, 1.f
    };
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };
    unsigned int VBO, EBO;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void preview::drop_texture()
{
    if(texture)
    {
        glDeleteTextures(1, &texture);
    }
}
void preview::drop_vao()
{
    if(vao)
    {
        glDeleteVertexArrays(1, &vao);
    }
}
void preview::drop_shaders()
{
    if(shaderProgram)
    {
        glDeleteProgram(shaderProgram);
    }
}
unsigned int preview::draw2Texture()
{
    return texture;
}
} // namespace libpgmaker
