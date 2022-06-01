#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

namespace libpgmaker {
struct video_info
{
    std::string path;
    std::uint32_t width, height;
    std::chrono::milliseconds duration;
    std::int64_t bitrate;
    int pixelFormat;
    int codecId;
};
class video
{
  public:
    video();
    video(const video_info& info, std::unique_ptr<std::uint8_t[]>&& tn);
    ~video();

    const video_info& get_info() const { return information; }
    unsigned int get_texture() const;

  private:
    video_info information;
    unsigned int texture;

    friend class video_reader;
    friend class channel;
};
} // namespace libpgmaker
