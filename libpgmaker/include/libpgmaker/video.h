#pragma once
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <chrono>
#include <cstdint>
#include <memory>

namespace libpgmaker {
struct video_state
{
    int width, height; ///< frame size

    // internal state
    AVFormatContext* avFormatContext{};
    AVCodecContext* avCodecContext{};
    int videoStreamIndex{ -1 };
    int audioStreamIndex{ -1 };
    SwsContext* swsScalerContext{};
    AVRational timeBase;
};
struct thumbnail
{
    std::uint32_t width, height;
    std::unique_ptr<std::uint8_t[]> data;
};
struct video_info
{
    std::uint32_t width, height;
    std::chrono::milliseconds duration;
    std::uint64_t bitrate;
    int pixelFormat;
    int codecId;
};
class video
{
  public:
    video() = default;
    video(const video_state& state, const video_info& info);
    ~video();

    void jump2(std::chrono::milliseconds timestamp);
    void read1packet();

    void allocate_thumbnail(int width, int height);
    const video_state& get_state() const { return state; }
    const video_info& get_info() const { return information; }

    thumbnail get_thumbnail(std::uint32_t width, std::uint32_t height);

  private:
    video_state state;
    video_info information;

    friend class video_reader;
    friend class channel;
};
} // namespace libpgmaker
