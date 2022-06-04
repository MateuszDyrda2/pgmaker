#include <libpgmaker/timeline.h>

#include <algorithm>
#include <cassert>

#include <glad/glad.h>

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
        paused    = other.paused;
        settings  = other.settings;
        ts        = milliseconds(0);
        tsChecked = time_point();
        channels  = std::move(other.channels);
    }
    return *this;
}
timeline::~timeline()
{
    auto s = textures.size();
    for(size_t i = 0; i < s; ++i)
    {
        destroy_texture();
    }
    drop_audio();
}
channel* timeline::add_channel()
{
    generate_texture(channels.size());
    return channels.emplace_back(make_unique<channel>(this, channels.size())).get();
}
void timeline::remove_channel(std::size_t index)
{
    destroy_texture();
    assert(index < channels.size());
    channels.erase(channels.begin() + index);
}
void timeline::remove_channel(channel* ch)
{
    destroy_texture();
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
std::pair<const std::vector<timeline::texture_t>&, std::size_t> timeline::next_frame()
{
    if(channels.empty())
        return { textures, 0 };

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
        // frames.push_back(ch->next_frame(ts, paused));
        auto frame = ch->next_frame(ts, paused);
        if(frame) frames.push_back(frame);
    }

    for(size_t index = 0; index < frames.size(); ++index)
    {
        texture_for_frame(index, frames[index]);
    }

    return { textures, frames.size() };
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

bool timeline::add_clip(std::size_t ch, const std::shared_ptr<video>& vid, const milliseconds& at,
                        const milliseconds& startOffset, const milliseconds& endOffset)
{
    assert(ch < channels.size());
    stop();
    auto res = channels[ch]->add_clip(vid, at, startOffset, endOffset);
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
void timeline::texture_for_frame(std::size_t index, frame* f)
{
    auto& tex = textures[index];
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, tex.handle);
    auto fsize = f->owner->get_size();
    if(fsize != tex.size)
    {
        tex.size = fsize;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.size.first, tex.size.second,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, f->data.data());
    }
    else
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                        tex.size.first, tex.size.second, GL_RGBA,
                        GL_UNSIGNED_BYTE, f->data.data());
    }
}
void timeline::generate_texture(std::size_t index)
{
    unsigned int texture;
    glActiveTexture(GL_TEXTURE0 + index);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 size.first, size.second, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, 0);
    textures.push_back({ texture, size });
}
void timeline::destroy_texture()
{
    auto tex = textures.back().handle;
    glDeleteTextures(1, &tex);
    textures.pop_back();
}
} // namespace libpgmaker
