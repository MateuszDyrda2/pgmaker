//#include <glad/glad.h>

#include "fbitem.h"

#include <QOpenGLFramebufferObjectFormat>
#include <QWindow>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace libpgmaker;
FbItem::FbItem():
    currentFrame(nullptr)
{ }
FbItemRenderer::FbItemRenderer(const FbItem* item):
    item(item), currentFrame(nullptr) //, prev(new preview({ 1920, 1080 }))
{
    ogl = new QOpenGLFunctions_3_3_Core;
    ogl->initializeOpenGLFunctions();
    printf("%s\n", ogl->glGetString(GL_VERSION));
}
FbItemRenderer::~FbItemRenderer()
{
    delete ogl;
    // delete prev;
}
QOpenGLFramebufferObject* FbItemRenderer::createFramebufferObject(const QSize& size)
{
    this->size = size;
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    initialize_shaders();
    initialize_vao();
    initialize_texture();
    return new QOpenGLFramebufferObject(size, format);
}
QQuickFramebufferObject::Renderer* FbItem::createRenderer() const
{
    return new FbItemRenderer(this);
}
void FbItemRenderer::render()
{
    ogl->glClearColor(1.f, 0.f, 0.f, 1.f);
    ogl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ogl->glUseProgram(shaderProgram);
    ogl->glBindTexture(GL_TEXTURE_2D, texture);
    if(currentFrame)
    {
        ogl->glClearColor(0.f, 1.f, 0.f, 1.f);
        ogl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if((currentFrame->size.width != size.width()) || (currentFrame->size.height != size.height()))
        {
            size.setWidth(currentFrame->size.width);
            size.setHeight(currentFrame->size.height);
            ogl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                              size.width(), size.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        }
        else
        {
            ogl->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                                 size.width(), size.height(), GL_RGBA,
                                 GL_UNSIGNED_BYTE, currentFrame->data.get());
        }
        glm::mat4 viewProj = glm::ortho(float(size.width()) * -0.5f, float(size.width()) * 0.5f,
                                        float(size.height()) * 0.5f, float(size.height()) * -0.5f);
        viewProj           = glm::scale(viewProj,
                                        glm::vec3(float(size.width()),
                                                  float(size.height()), 1.f));
        ogl->glUniformMatrix4fv(
            ogl->glGetUniformLocation(shaderProgram, "viewProj"), 1, GL_FALSE, glm::value_ptr(viewProj));
    }
    ogl->glBindVertexArray(vao);
    ogl->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    ogl->glBindTexture(GL_TEXTURE_2D, 0);
    ogl->glUseProgram(0);
}
void FbItemRenderer::synchronize(QQuickFramebufferObject* item)
{
    if(auto fr = static_cast<FbItem*>(item)->get_current_frame())
    {
        currentFrame = fr;
    }
}
void FbItem::frame_updated(libpgmaker::frame* f)
{
    if(f != nullptr)
    {
        currentFrame = f;
        update();
    }
}
libpgmaker::frame* FbItem::get_current_frame()
{
    return currentFrame;
}
void FbItemRenderer::initialize_texture()
{
    ogl->glGenTextures(1, &texture);
    ogl->glBindTexture(GL_TEXTURE_2D, texture);
    ogl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    ogl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    ogl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    ogl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    ogl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                      size.width(), size.height(), 0,
                      GL_RGBA, GL_UNSIGNED_BYTE, 0);
}
void FbItemRenderer::initialize_shaders()
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

    unsigned int vertexShader = ogl->glCreateShader(GL_VERTEX_SHADER);
    ogl->glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
    int isCompiled = 0;
    ogl->glCompileShader(vertexShader);
    ogl->glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        char errorMsg[128];
        int len;
        ogl->glGetShaderInfoLog(vertexShader, sizeof(errorMsg), &len, errorMsg);
        printf("%s\n", errorMsg);
        ogl->glDeleteShader(vertexShader);
        return;
    }
    unsigned int fragmentShader = ogl->glCreateShader(GL_FRAGMENT_SHADER);
    ogl->glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
    ogl->glCompileShader(fragmentShader);
    ogl->glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        char errorMsg[128];
        int len;
        ogl->glGetShaderInfoLog(fragmentShader, sizeof(errorMsg), &len, errorMsg);
        printf("%s\n", errorMsg);
        ogl->glDeleteShader(fragmentShader);
        return;
    }
    shaderProgram = ogl->glCreateProgram();
    ogl->glAttachShader(shaderProgram, vertexShader);
    ogl->glAttachShader(shaderProgram, fragmentShader);
    ogl->glLinkProgram(shaderProgram);
    int isLinked = 0;
    ogl->glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);
    if(isLinked == GL_FALSE)
    {
        char errorMsg[128];
        int len;
        ogl->glGetProgramInfoLog(shaderProgram, sizeof(errorMsg), &len, errorMsg);
        printf("%s\n", errorMsg);
    }
    ogl->glDeleteShader(vertexShader);
    ogl->glDeleteShader(fragmentShader);
    ogl->glUseProgram(shaderProgram);
}
void FbItemRenderer::initialize_vao()
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
    ogl->glGenVertexArrays(1, &vao);
    ogl->glBindVertexArray(vao);
    ogl->glGenBuffers(1, &VBO);
    ogl->glBindBuffer(GL_ARRAY_BUFFER, VBO);
    ogl->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    auto aPosLoc      = ogl->glGetAttribLocation(shaderProgram, "aPos");
    auto aTexCoordLoc = ogl->glGetAttribLocation(shaderProgram, "aTexCoord");
    ogl->glVertexAttribPointer(
        aPosLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    ogl->glVertexAttribPointer(
        aTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    ogl->glEnableVertexAttribArray(0);
    ogl->glEnableVertexAttribArray(1);
    ogl->glGenBuffers(1, &EBO);
    ogl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    ogl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}
