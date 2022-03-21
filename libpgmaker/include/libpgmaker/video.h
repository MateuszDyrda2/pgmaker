#pragma once
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

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
};
class video
{
  public:
    video(const video_state& state);
    ~video();

    const video_data& get_data() const { return data; }
    const video_state& get_state() const { return state; }

    const std::shared_ptr<unsigned char[]> get_thumbnail() const { return thumbnail; }

  private:
    video_data data;
    video_state state;

    std::shared_ptr<unsigned char[]> thumbnail;
};
} // namespace libpgmaker
