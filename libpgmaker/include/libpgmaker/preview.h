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
    unsigned int get_texture_handle() const;

  private:
    resolution size;
    unsigned int texture;

  private:
    void initialize_texture();
};
} // namespace libpgmaker
