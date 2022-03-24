#include <libpgmaker/textureGL.h>

#include <glad/glad.h>

namespace libpgmaker {
textureGL::textureGL(std::uint32_t width, std::uint32_t height):
    width(width), height(height)
{
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
textureGL::textureGL(std::uint32_t width, std::uint32_t height, std::uint8_t* data):
    width(width), height(height)
{
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}
void textureGL::update_data(std::uint8_t* data)
{
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                    width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}
void textureGL::bind()
{
    glBindTexture(GL_TEXTURE_2D, handle);
}
void textureGL::unbind()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}
textureGL::~textureGL()
{
    glDeleteTextures(1, &handle);
}
} // namespace libpgmaker
