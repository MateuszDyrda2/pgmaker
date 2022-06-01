#include <libpgmaker/timeline.h>

#include <algorithm>
#include <cassert>

namespace libpgmaker {
using namespace std;
using namespace chrono_literals;
timeline::timeline(const project_settings& settings):
    paused(false), settings(settings),
    ts(0), tsChecked()
{
    rebuild();
    set_paused(true);
    initialize_audio();
}
timeline::timeline(timeline&& other) noexcept:
    paused(other.paused), settings(other.settings),
    ts(0), tsChecked(), channels(std::move(other.channels))
{
}
timeline& timeline::operator=(timeline&& other) noexcept
{
    if(this != &other)
    {
		paused = other.paused;
		settings = other.settings;
		ts = milliseconds(0);
		tsChecked = time_point();
		channels = std::move(other.channels);
    }
    return *this;
}
timeline::~timeline()
{
    drop_audio();
}
channel* timeline::add_channel()
{
    return channels.emplace_back(make_unique<channel>(this)).get();
}
void timeline::remove_channel(std::size_t index)
{
    assert(index < channels.size());
    channels.erase(channels.begin() + index);
}
void timeline::remove_channel(channel* ch)
{
    assert(ch != nullptr);
    auto toEr = find_if(channels.begin(), channels.end(),
                        [&](auto& item) { return item.get() == ch; });
    if(toEr == channels.end())
        return;

    channels.erase(toEr);
}
channel* timeline::get_channel(std::size_t index)
{
    assert(index < channels.size());
    return channels[index].get();
}
channel* timeline::operator[](std::size_t index)
{
    return channels[index].get();
}
const channel* timeline::get_channel(std::size_t index) const
{
    assert(index < channels.size());
    return channels[index].get();
}
std::deque<std::unique_ptr<channel>>& timeline::get_channels()
{
    return channels;
}
const std::deque<std::unique_ptr<channel>>& timeline::get_channels() const
{
    return channels;
}
const channel* timeline::operator[](std::size_t index) const
{
    return channels[index].get();
}
frame* timeline::next_frame()
{
    if(channels.empty())
        return nullptr;

    if(!paused)
    {
        auto now = chrono::high_resolution_clock::now();
        ts += chrono::duration_cast<milliseconds>(
            now - tsChecked);
        tsChecked = now;
    }

    vector<frame*> frames;
    for(auto& ch : channels)
    {
        frames.push_back(ch->next_frame(ts, paused));
    }
    return frames.front();
}
bool timeline::set_paused(bool value)
{
    if(paused == value) return value;

    auto old = paused;
    paused   = value;
    if(!value)
    {
        tsChecked = chrono::high_resolution_clock::now();
    }
    for(auto& ch : channels)
    {
        ch->set_paused(value);
    }
    return old;
}

bool timeline::toggle_pause()
{
    auto old = paused;

    paused ^= 1;
    for_each(channels.begin(), channels.end(),
             [&](auto& ch) { ch->set_paused(paused); });

    if(paused == false) tsChecked = chrono::high_resolution_clock::now();

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
    ts        = 0ms;
    tsChecked = high_resolution_clock::now();
}
void timeline::seek(const milliseconds& ts)
{
    using namespace chrono;
    auto realTs = max(milliseconds(0), min(ts, get_duration()));

    this->ts = realTs;

    std::for_each(
        channels.begin(), channels.end(),
        [](auto& ch) { ch->stop(); });
    std::for_each(
        channels.begin(), channels.end(),
        [realTs](auto& ch) { ch->seek(realTs); });
    std::for_each(
        channels.begin(), channels.end(),
        [](auto& ch) { ch->start(); });

    tsChecked = high_resolution_clock::now();
}

timeline::milliseconds timeline::get_duration() const
{
    if(channels.empty()) return 0ms;

    auto max = max_element(
        channels.begin(), channels.end(),
        [](const auto& lhs, const auto& rhs) {
            return lhs->get_duration() < rhs->get_duration();
        });
    return (*max)->get_duration();
}
timeline::milliseconds timeline::get_timestamp() const
{
    using namespace chrono;
    if(paused)
        return ts;
    else
        return ts + duration_cast<milliseconds>(high_resolution_clock::now() - tsChecked);
}
bool timeline::add_clip(std::size_t ch, const std::shared_ptr<video>& vid, const milliseconds& at)
{
    assert(ch < channels.size());
    stop();
    auto res = channels[ch]->add_clip(vid, at);
    start();
    return res;
}
void timeline::append_clip(std::size_t ch, const std::shared_ptr<video>& vid)
{
    assert(ch < channels.size());
    stop();
    channels[ch]->append_clip(vid);
    start();
}
void timeline::move_clip(std::size_t ch, std::size_t cl, const milliseconds& to)
{
    assert(ch < channels.size());
    stop();
    channels[ch]->move_clip(cl, to);
    start();
}
void timeline::stop()
{
    set_paused(true);
    std::for_each(
        channels.begin(), channels.end(),
        [](auto& ch) {
            ch->stop();
        });
}
void timeline::start()
{
    std::for_each(
        channels.begin(), channels.end(),
        [](auto& ch) {
            ch->start();
        });
}
} // namespace libpgmaker
