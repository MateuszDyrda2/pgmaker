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
    class clip* owner;
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
    class clip* owner = nullptr;
    AVPacket* payload = nullptr;

    packet() = default;
    packet(class clip* owner, AVPacket* payload);
    ~packet();
    packet(const packet& other) = delete;
    packet(packet&& other) noexcept;
    packet& operator=(packet&& other) noexcept;
    void unref();
};
} // namespace libpgmaker
