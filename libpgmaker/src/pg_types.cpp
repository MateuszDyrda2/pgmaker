#include <libpgmaker/pg_types.h>

namespace libpgmaker {

pixel_size& pixel_size::operator+=(const pixel_size& other)
{
    width += other.width;
    height += other.height;
    return *this;
}
pixel_size& pixel_size::operator-=(const pixel_size& other)
{
    width -= other.width;
    height -= other.height;
    return *this;
}
pixel_size& pixel_size::operator*=(std::uint32_t value)
{
    width *= value;
    height *= value;
    return *this;
}
pixel_size& pixel_size::operator/=(std::uint32_t value)
{
    width /= value;
    height /= value;
    return *this;
}
pixel_size operator+(const pixel_size& lhs, const pixel_size& rhs)
{
    return pixel_size(lhs) += rhs;
}
pixel_size operator-(const pixel_size& lhs, const pixel_size& rhs)
{
    return pixel_size(lhs) -= rhs;
}
pixel_size operator*(const pixel_size& lhs, std::uint32_t rhs)
{
    return pixel_size(lhs) *= rhs;
}
pixel_size operator*(std::uint32_t lhs, const pixel_size& rhs)
{
    return pixel_size(rhs) *= lhs;
}
pixel_size operator/(const pixel_size& lhs, std::uint32_t rhs)
{
    return pixel_size(lhs) /= rhs;
}
bool operator==(const pixel_size& lhs, const pixel_size& rhs)
{
    return lhs.width == rhs.width && lhs.height == rhs.height;
}
bool operator!=(const pixel_size& lhs, const pixel_size& rhs)
{
    return !(lhs == rhs);
}

packet::packet(class clip* owner, AVPacket* payload):
    owner(owner), payload(payload) { }
packet::~packet()
{
    if(payload)
        av_packet_free(&payload);
}
packet::packet(packet&& other) noexcept:
    owner(other.owner), payload(other.payload)
{
    other.payload = nullptr;
}
packet& packet::operator=(packet&& other) noexcept
{
    if(this != &other)
    {
        if(payload)
            av_packet_free(&payload);
        owner         = other.owner;
        payload       = other.payload;
        other.payload = nullptr;
    }
    return *this;
}
void packet::unref()
{
    av_packet_unref(payload);
}
} // namespace libpgmaker
