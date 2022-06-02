#include <libpgmaker/pg_types.h>

namespace libpgmaker {
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
