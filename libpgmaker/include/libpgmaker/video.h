#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

namespace libpgmaker {
struct thumbnail
{
    std::uint32_t width, height;
    std::unique_ptr<std::uint8_t[]> data;
};
struct video_info
{
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
    video(const std::string& path, const video_info& info, thumbnail&& tn);
    ~video();

    const video_info& get_info() const { return information; }
    const thumbnail& get_thumbnail() { return tn; }
    const std::string& get_path() const { return path; }
    unsigned int get_texture() const;

  private:
    std::string path;
    video_info information;
    thumbnail tn;
    unsigned int texture;

    friend class video_reader;
    friend class channel;
};
} // namespace libpgmaker
