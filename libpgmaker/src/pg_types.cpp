#include <libpgmaker/pg_types.h>

namespace libpgmaker {
packet::packet(class clip* owner, AVPacket* payload):
    owner(owner), payload(payload) { }
packet::~packet()
{
    free();
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
        free();
        owner         = other.owner;
        payload       = other.payload;
        other.payload = nullptr;
    }
    return *this;
}

void packet::free()
{
    if(payload)
        av_packet_free(&payload);
}
void packet::unref()
{
    av_packet_unref(payload);
}
} // namespace libpgmaker
