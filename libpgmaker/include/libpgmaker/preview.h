#pragma once

#include "pg_types.h"

namespace libpgmaker {
class preview
{
  public:
    /** @brief Creates a preview
     * @param size size of the preview
     */
    preview(const std::pair<std::uint32_t, std::uint32_t>& size);
    ~preview();
    /** @brief Specify a new size for the preview
     * @param newSize new size to be set
     */
    void resize(const std::pair<std::uint32_t, std::uint32_t>& newSize);
    void update(frame* newFrame);
    void draw();
    unsigned int draw2Texture();

  private:
    std::pair<std::uint32_t, std::uint32_t> size;
    std::pair<std::uint32_t, std::uint32_t> textureSize;
    unsigned int texture;
    unsigned int shaderProgram;
    unsigned int vao;

  private:
    void initialize_texture();
    void initialize_shaders();
    void initialize_vao();
    void drop_texture();
    void drop_shaders();
    void drop_vao();
};
} // namespace libpgmaker
