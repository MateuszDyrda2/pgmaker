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
#include <vector>

namespace libpgmaker {
struct resolution
{
    std::uint32_t width, height;
};
static bool operator==(const resolution& lhs, const resolution& rhs)
{
    return lhs.width == rhs.width && lhs.height == rhs.height;
}
static bool operator!=(const resolution& lhs, const resolution& rhs)
{
    return !(lhs == rhs);
}
struct frame
{
    resolution size;
    std::unique_ptr<std::uint8_t[]> data;
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
