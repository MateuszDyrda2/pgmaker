#pragma once

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include <chrono>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace libpgmaker {
struct frame
{
    std::pair<std::uint32_t, std::uint32_t> size;
    std::vector<std::uint8_t> data;
    std::chrono::milliseconds timestamp;
};
struct audio_frame
{
    std::vector<float> data;
    std::chrono::milliseconds pts{};
};
struct packet
{
    class clip* owner;
    AVPacket* payload;
};
} // namespace libpgmaker
