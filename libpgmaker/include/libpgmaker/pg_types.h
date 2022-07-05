#pragma once

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include <chrono>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace libpgmaker {
struct pixel_size
{
    std::uint32_t width, height;

    pixel_size& operator+=(const pixel_size& other);
    pixel_size& operator-=(const pixel_size& other);
    pixel_size& operator*=(std::uint32_t value);
    pixel_size& operator/=(std::uint32_t value);
};
pixel_size operator+(const pixel_size& lhs, const pixel_size& rhs);
pixel_size operator-(const pixel_size& lhs, const pixel_size& rhs);
pixel_size operator*(const pixel_size& lhs, std::uint32_t rhs);
pixel_size operator*(std::uint32_t lhs, const pixel_size& rhs);
pixel_size operator/(const pixel_size& lhs, std::uint32_t rhs);
bool operator==(const pixel_size& lhs, const pixel_size& rhs);
bool operator!=(const pixel_size& lhs, const pixel_size& rhs);

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
