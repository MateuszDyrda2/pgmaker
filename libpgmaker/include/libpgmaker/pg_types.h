/** @file */

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
/** Type representing a resolution */
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

/** type representing a single frame */
struct frame
{
    class clip* owner;                   ///< clip the frame belongs to
    std::vector<std::uint8_t> data;      ///< data contained in the frame
    std::chrono::milliseconds timestamp; ///< time the frame occurs in the video
};
/** type representing a single audio frame */
struct audio_frame
{
    std::vector<float> data;         ///<  data contained in the frame
    std::chrono::milliseconds pts{}; ///< time the frame occurs in the video
};
/** type representing a packet */
struct packet
{
    class clip* owner = nullptr; ///< clip the packet belongs to
    AVPacket* payload = nullptr; ///< payload of the packet

    packet() = default;
    packet(class clip* owner, AVPacket* payload);
    ~packet();
    packet(const packet& other) = delete;
    packet(packet&& other) noexcept;
    packet& operator=(packet&& other) noexcept;
    /** @brief Ureference the data the packet contains */
    void unref();
};
} // namespace libpgmaker
