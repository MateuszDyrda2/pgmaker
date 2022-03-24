#pragma once

#include <cstdint>

namespace libpgmaker {
class textureGL
{
  public:
    textureGL() = default;
    textureGL(std::uint32_t width, std::uint32_t height);
    textureGL(std::uint32_t width, std::uint32_t height, std::uint8_t* data);
    void update_data(std::uint8_t* data);
    ~textureGL();

    std::uint32_t get_handle() const { return handle; }
    void bind();
    void unbind();

  private:
    std::uint32_t handle;
    std::uint32_t width, height;
};
} // namespace libpgmaker
