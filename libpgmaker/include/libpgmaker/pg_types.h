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

namespace libpgmaker {
struct resolution
{
    std::uint32_t width, height;
};
struct frame
{
    resolution size;
    std::unique_ptr<std::uint8_t[]> data;
    std::chrono::milliseconds timestamp;
};
struct audio_frame
{
    std::unique_ptr<std::uint8_t[]> data;
    std::size_t nbData;
};
struct packet
{
    class clip* owner;
    AVPacket* payload;
};
} // namespace libpgmaker
