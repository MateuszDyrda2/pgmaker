#include <libpgmaker/timeline.h>

#include <cassert>

namespace libpgmaker {
timeline::timeline(const project_settings& settings):
    settings(settings)
{
}
timeline::~timeline()
{
}
channel* timeline::add_channel()
{
    return channels.emplace_back(std::make_unique<channel>()).get();
}
void timeline::remove_channel(std::size_t index)
{
    assert(index < channels.size());
    channels.erase(channels.begin() + index);
}
channel* timeline::get_channel(std::size_t index)
{
    assert(index < channels.size());
    return channels[index].get();
}
std::shared_ptr<frame> timeline::get_frame()
{
    for(auto&& ch : channels)
    {
        auto f = ch->get_frame();
    }
}
} // namespace libpgmaker
