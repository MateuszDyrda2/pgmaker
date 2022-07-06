/** @file */

#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

namespace libpgmaker {
/** Metadata of the video */
struct video_info
{
    std::string name;
    std::string path;
    std::uint32_t width, height;
    std::chrono::milliseconds duration;
    std::int64_t bitrate;
    int pixelFormat;
    int codecId;
};
/** Video class for a video file */
class video
{
  public:
    video();
    video(const video_info& info, std::unique_ptr<std::uint8_t[]>&& tn);
    ~video();
    /** @return information about the video */
    const video_info& get_info() const { return information; }
    /** @return video thumbnail if loaded */
    unsigned int get_texture() const;

  private:
    video_info information;
    unsigned int texture;

    friend class video_reader;
    friend class channel;
};
} // namespace libpgmaker
