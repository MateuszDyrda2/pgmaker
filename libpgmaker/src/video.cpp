#include <libpgmaker/video.h>

#include <glad/glad.h>

namespace libpgmaker {
video::video():
    texture(0)
{
}
video::video(const std::string& path, const video_info& info, thumbnail&& tn):
    path(path), information(info), tn(std::move(tn))
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 tn.width, tn.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tn.data.get());
}
video::~video()
{
    if(texture)
        glDeleteTextures(1, &texture);
}
unsigned int video::get_texture() const
{
    return texture;
}
} // libpgmaker