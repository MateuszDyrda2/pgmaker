#include <libpgmaker/preview.h>

#include <glad/glad.h>

namespace libpgmaker {
preview::preview(const resolution& size):
    size(size), texture(0)
{
    initialize_texture();
}
preview::~preview()
{
}
void preview::resize(const resolution& newSize)
{
}
void preview::update(frame* newFrame)
{
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                    size.width, size.height, GL_RGBA,
                    GL_UNSIGNED_BYTE, newFrame->data.get());
    glBindTexture(GL_TEXTURE_2D, 0);
}
unsigned int preview::get_texture_handle() const
{
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
                 size.width, size.height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
} // namespace libpgmaker
