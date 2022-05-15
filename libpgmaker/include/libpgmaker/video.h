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
    video() = default;
    video(const std::string& path, const video_info& info, thumbnail&& tn):
        path(path), information(info), tn(std::move(tn)) { }

    const video_info& get_info() const { return information; }
    const thumbnail& get_thumbnail() { return tn; }
    const std::string& get_path() const { return path; }

  private:
    std::string path;
    video_info information;
    thumbnail tn;

    friend class video_reader;
    friend class channel;
};
} // namespace libpgmaker
