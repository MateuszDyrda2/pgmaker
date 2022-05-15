#include <libpgmaker/timeline.h>

#include <cassert>

namespace libpgmaker {
using namespace std;
timeline::timeline(const project_settings& settings):
    paused(false), start(), pausedOffset(0),
    startOffset(0), settings(settings), pauseStarted()
{
    rebuild();
    set_paused(true);
    initialize_audio();
}
timeline::~timeline()
{
    drop_audio();
}
channel* timeline::add_channel()
{
    return channels.emplace_back(make_unique<channel>(*this)).get();
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
frame* timeline::get_frame()
{
    if(channels.empty())
        return nullptr;

    const auto timestamp = get_timestamp();

    vector<frame*> frames;
    for(auto& ch : channels)
    {
        frames.push_back(ch->get_frame(timestamp));
    }
    return frames.front();
}
bool timeline::set_paused(bool value)
{
    using namespace chrono;
    if(paused == value) return value;

    auto old = paused;
    paused   = value;
    if(value)
    {
        pauseStarted = high_resolution_clock::now();
    }
    else
    {
        pausedOffset += high_resolution_clock::now() - pauseStarted;
    }
    for(auto& ch : channels)
    {
        ch->set_paused(value);
    }
    return old;
}
void timeline::initialize_audio()
{
    auto err = Pa_Initialize();
    if(err != paNoError)
    {
        throw runtime_error("PortAudio failed while initializing with: " + string(Pa_GetErrorText(err)));
    }
}
void timeline::drop_audio()
{
    channels.clear();
    auto err = Pa_Terminate();
    if(err != paNoError)
    {
        throw runtime_error("PortAudio failed while destroying with: " + string(Pa_GetErrorText(err)));
    }
}
void timeline::rebuild()
{
    using namespace chrono;
    start        = high_resolution_clock::now();
    pausedOffset = duration(0);
}
void timeline::jump2(const milliseconds& ts)
{
    using namespace chrono;
    // auto old    = set_paused(true);
    startOffset  = ts;
    pausedOffset = duration(0);
    // rebuild();
    start = pauseStarted = high_resolution_clock::now();
    for(auto& ch : channels)
    {
        ch->jump2(ts);
    }
}
timeline::milliseconds timeline::get_timestamp() const
{
    using namespace chrono;

    return duration_cast<milliseconds>(
        high_resolution_clock::now() - start - pausedOffset + startOffset);
}
} // namespace libpgmaker
