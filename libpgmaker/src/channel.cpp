#include <libpgmaker/channel.h>

#include <libpgmaker/id_generator.h>
#include <libpgmaker/timeline.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

#include <glad/glad.h>

namespace libpgmaker {
using namespace std;
using namespace chrono_literals;
channel::channel(const timeline* tl, std::size_t index):
    currentClip{ clips.end() }, prevFrame{},
    nextFrame{}, stopped{ true }, paused{ false },
    tl(tl), audioStream{}, lenght(0),
    videoPacketQueue(64), audioPacketQueue(64),
    frameQueue(64), index(index), channelId(id_generator::get_next())
{
    silentBuffer.resize(NB_CHANNELS * NB_FRAMES, 0.f);
    audioBuffer.resize(NB_CHANNELS * NB_FRAMES);
    set_paused(true);
}
channel::channel(std::deque<std::unique_ptr<clip>>&& clips):
    clips{ std::move(clips) }, currentClip{ clips.end() },
    prevFrame{}, nextFrame{}, stopped{ true }, paused{ false },
    audioStream{}, lenght(0), videoPacketQueue{ 64 },
    audioPacketQueue(64), frameQueue(64), channelId(id_generator::get_next())
{
    silentBuffer.resize(NB_CHANNELS * NB_FRAMES, 0.f);
    audioBuffer.resize(NB_CHANNELS * NB_FRAMES);
    set_paused(true);
}
void channel::set_timeline(const timeline* tl)
{
    this->tl = tl;
}
channel::~channel()
{
    drop_audio();
    stopped = true;
    if(decodeWorker.joinable()) decodeWorker.join();
    if(videoWorker.joinable()) videoWorker.join();
}
bool channel::add_clip(const shared_ptr<video>& vid, const chrono::milliseconds& at)
{
    clips.emplace_back(make_unique<clip>(vid, at));
    currentClip = clips.begin();
    recalculate_lenght();
    return true;
}
bool channel::add_clip(const std::shared_ptr<video>& vid, const milliseconds& at,
                       const milliseconds& startOffset, const milliseconds& endOffset)
{
    clips.emplace_back(make_unique<clip>(vid, at, startOffset, endOffset));
    currentClip = clips.begin();
    recalculate_lenght();
    return true;
}
bool channel::append_clip(const std::shared_ptr<video>& vid)
{
    return add_clip(vid, lenght);
}
clip* channel::get_clip(std::size_t index)
{
    auto it = clips.begin();
    std::advance(it, index);
    return it->get();
}
const clip* channel::get_clip(std::size_t index) const
{
    auto it = clips.begin();
    std::advance(it, index);
    return it->get();
}
clip* channel::operator[](std::size_t index)
{
    return get_clip(index);
}
const clip* channel::operator[](std::size_t index) const
{
    return get_clip(index);
}
std::deque<std::unique_ptr<clip>>& channel::get_clips()
{
    return clips;
}
const std::deque<std::unique_ptr<clip>>& channel::get_clips() const
{
    return clips;
}
frame* channel::next_frame(const duration& timestamp, bool paused)
{
    if(timestamp > lenght)
    {
        return nullptr;
    }
    if(!nextFrame && !frameQueue.try_dequeue(nextFrame)) return nullptr;

    auto pts = nextFrame->timestamp;
    if(timestamp >= pts)
    {
        while(timestamp > pts)
        {
            if(prevFrame != nextFrame)
            {
                delete prevFrame;
                prevFrame = nextFrame;
            }
            if(!frameQueue.try_dequeue(nextFrame)) return nullptr;
            pts = nextFrame->timestamp;
        }
    }
    if(prevFrame && timestamp > prevFrame->owner->get_ends_at())
    {
        prevFrame = nullptr;
    }

    return prevFrame;
}
bool channel::set_paused(bool value)
{
    auto old = paused.load();
    paused   = value;
    return old;
}
void channel::stop()
{
    stopped = true;
    if(decodeWorker.joinable()) decodeWorker.join();
    if(videoWorker.joinable()) videoWorker.join();

    frame_queue_t(MAX_PACKETS).swap(frameQueue);
    videoPacketQueue = packet_queue_t(MAX_PACKETS);
    audioPacketQueue = packet_queue_t(MAX_PACKETS);

    drop_audio();
    for(auto& clip : clips)
    {
        clip->reset();
    }

    nextFrame = prevFrame = nullptr;
}
void channel::start()
{
    stopped      = false;
    decodeWorker = worker_type(&channel::decoding_job, this);
    videoWorker  = worker_type(&channel::video_job, this);
    init_audio();
}
void channel::seek(const chrono::milliseconds& ts)
{
    currentClip = clips.begin();
    for(auto iter = clips.begin(); iter != clips.end(); ++iter)
    {
        if(!(*iter)->contains(ts))
            continue;

        currentClip = iter;
        if(!(*iter)->seek(ts))
        {
            throw runtime_error("Failed to jump to requested timestamp");
        }
        break;
    }
}
void channel::decoding_job()
{
    static constexpr size_t MAX_NOOP                = 8;
    static constexpr chrono::milliseconds SLEEP_DUR = 10ms;
    size_t nbNoop                                   = 0ULL;

    while(!stopped)
    {
        if(currentClip != clips.end())
        {
            if(nbNoop >= MAX_NOOP)
            {
                nbNoop = 0;
                this_thread::sleep_for(SLEEP_DUR);
            }
            auto& c = *currentClip;

            if(videoPacketQueue.size_approx() >= MAX_PACKETS
               && audioPacketQueue.size_approx() >= MAX_PACKETS)
            {
                ++nbNoop;
                continue;
            }
            packet _packet;
            if(c->get_packet(_packet))
            {
                if(_packet.payload->stream_index == c->vsIndex)
                {
                    videoPacketQueue.enqueue(std::move(_packet));
                }
                else if(_packet.payload->stream_index == c->asIndex)
                {
                    audioPacketQueue.enqueue(std::move(_packet));
                }
            }
            else
            {
                // the clip has ended
                advance(currentClip, 1);
                if(currentClip != clips.end())
                {
                    (*currentClip)->reset();
                }
            }
        }
        else
        {
            // there are no more clips
            return;
        }
    }
}
void channel::video_job()
{
    static constexpr size_t MAX_NOOP                = 8;
    static constexpr chrono::milliseconds SLEEP_DUR = 10ms;
    size_t nbNoop                                   = 0;

    AVFrame* pFrame = av_frame_alloc();
    while(!stopped)
    {
        if(nbNoop >= MAX_NOOP)
        {
            nbNoop = 0;
            this_thread::sleep_for(SLEEP_DUR);
        }
        packet _packet;
        if(!videoPacketQueue.try_dequeue(_packet))
        {
            ++nbNoop;
            continue;
        }
        nbNoop = 0;
        auto c = _packet.owner;
        if(c->get_frame(_packet, &pFrame))
        {
            auto fr = new frame;
            c->convert_frame(pFrame, &fr);
            while(!stopped && !frameQueue.wait_enqueue_timed(fr, 10ms))
                ;
        }
    }
    av_frame_free(&pFrame);
}
void channel::recalculate_lenght()
{
    if(clips.empty())
        lenght = 0ms;
    else
    {
        const auto& last = clips.back();
        lenght           = last->startsAt + last->get_duration();
    }
}
void channel::init_audio()
{
    if(!audioStream)
    {
        PaStreamParameters outParams;
        outParams.channelCount              = NB_CHANNELS;
        outParams.device                    = Pa_GetDefaultOutputDevice();
        outParams.suggestedLatency          = 0;
        outParams.hostApiSpecificStreamInfo = NULL;
        outParams.sampleFormat              = paFloat32;

        auto err = Pa_OpenStream(
            &audioStream,
            NULL,
            &outParams,
            SAMPLE_RATE,
            NB_FRAMES,
            paClipOff,
            [](const void*,
               void* output,
               unsigned long frameCount,
               const PaStreamCallbackTimeInfo* timeInfo,
               PaStreamCallbackFlags,
               void* userData) {
                return static_cast<channel*>(userData)->audio_stream_callback(
                    output, frameCount, timeInfo);
            },
            this);

        if(err != paNoError)
        {
            throw runtime_error("PortAudio failed while opening a stream with: " + string(Pa_GetErrorText(err)));
        }
        err = Pa_StartStream(audioStream);
        if(err != paNoError)
        {
            throw runtime_error("PortAudio failed while starting a stream with: " + string(Pa_GetErrorText(err)));
        }
    }
}
void channel::drop_audio()
{
    if(audioStream)
    {
        auto err = Pa_AbortStream(audioStream);
        if(err != paNoError)
        {
            throw runtime_error("PortAudio failed while stopping a stream with: " + string(Pa_GetErrorText(err)));
        }
        err = Pa_CloseStream(audioStream);
        if(err != paNoError)
        {
            throw runtime_error("PortAudio failed while closing a stream with: " + string(Pa_GetErrorText(err)));
        }
        audioStream = nullptr;
    }
}
int channel::audio_stream_callback(
    void* output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo)
{
    static constexpr milliseconds NoSyncThreshold      = 24ms;
    static constexpr milliseconds minusNoSyncThreshold = -24ms;
    auto out                                           = static_cast<float*>(output);

    int sampleCount = frameCount;
    if(paused)
    {
        // if the channel is paused => fill the buffer with silence
        std::copy_n(silentBuffer.begin(), sampleCount * NB_CHANNELS, out);
        return 0;
    }

    auto ptr = audioBuffer.data();
    while(sampleCount > 0)
    {
        auto pptr = audioPacketQueue.peek();
        if(!pptr)
        {
            std::copy_n(silentBuffer.begin(), sampleCount * NB_CHANNELS, out);
            break;
        }
        auto tlTs       = tl->get_timestamp();
        auto* c         = pptr->owner;
        auto newPts     = c->audio_convert_pts(pptr->payload->pts);
        const auto diff = newPts - tlTs;
        if(diff > NoSyncThreshold)
        {
            // fill with silence
            std::copy_n(silentBuffer.begin(), sampleCount * NB_CHANNELS, out);
            return 0;
        }

        if(diff < minusNoSyncThreshold)
        {
            audioPacketQueue.pop();
            continue;
        }
        auto bSize = c->get_audio_frame(pptr, audioBuffer);
        std::copy_n(audioBuffer.begin(), bSize * NB_CHANNELS, out);
        sampleCount -= bSize;
        audioPacketQueue.pop();
    }
    return 0;
}

void channel::move_clip(std::size_t index, const milliseconds& to)
{
    assert(index < clips.size());

    if(to < 0ms) return;
    auto cl = std::next(clips.begin(), index)->get();

    auto end     = to + cl->get_duration();
    auto canMove = std::find_if(
        clips.begin(), clips.end(),
        [&](auto& clip) {
            if(clip.get() == cl) return false;
            if(clip->contains(to) || clip->contains(end))
                return true;
            return false;
        });
    if(canMove != clips.end()) return;

    cl->set_starts_at(to);

    reorder_clips();
    recalculate_lenght();
}
void channel::reorder_clips()
{
    std::sort(clips.begin(), clips.end(),
              [](const auto& lhs, const auto& rhs) {
                  return lhs->get_starts_at() < rhs->get_starts_at();
              });
}
}