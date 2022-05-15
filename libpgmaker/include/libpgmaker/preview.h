#pragma once

#include "pg_types.h"

namespace libpgmaker {
class preview
{
  public:
    /** @brief Creates a preview
     * @param size size of the preview
     */
    preview(const resolution& size);
    ~preview();
    /** @brief Specify a new size for the preview
     * @param newSize new size to be set
     */
    void resize(const resolution& newSize);
    void update(frame* newFrame);
    void draw();

  private:
    resolution size;
    resolution textureSize;
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
