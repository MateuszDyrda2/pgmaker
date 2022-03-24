#pragma once
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <cstdint>
#include <memory>

namespace libpgmaker {
struct video_state
{
    int width, height; ///< frame size

    // internal state
    AVFormatContext* avFormatContext{};
    AVCodecContext* avCodecContext{};
    int videoStreamIndex{};
    AVFrame* avFrame{};
    AVPacket* avPacket{};
    SwsContext* swsScalerContext{};
    AVRational timeBase;
};
struct video_data
{
    std::uint32_t width, height;
    std::unique_ptr<std::uint8_t[]> data;
};
struct thumbnail
{
    std::uint32_t width, height;
    std::unique_ptr<std::uint8_t[]> data;
};
class video
{
  public:
    video() = default;
    video(const video_state& state);
    ~video();

    void allocate_thumbnail(int width, int height);
    const video_data& get_data() const { return data; }
    const video_state& get_state() const { return state; }
    void tick_frame();

    thumbnail get_thumbnail(std::uint32_t width, std::uint32_t height);

  private:
    video_data data;
    video_state state;
    friend class video_reader;
};
} // namespace libpgmaker
