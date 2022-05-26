#include <libpgmaker/channel.h>

#include <libpgmaker/timeline.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

namespace libpgmaker {
using namespace std;

channel::channel(const timeline& tl):
    currentClip{ clips.end() }, prevFrame{},
    nextFrame{}, stopped{ true }, paused{ true },
    tl(tl), audioStream{}, lenght(0)
{
    silentBuffer.resize(nbChannels * nbFrames, 0.f);
    audioBuffer.resize(nbChannels * nbFrames);
    set_paused(true);
    // init_audio();
}
channel::~channel()
{
    drop_audio();
    stopped = true;
    videoPacketQueue.notify();
    audioPacketQueue.notify();
    frameQueue.notify();
    if(decodeWorker.joinable()) decodeWorker.join();
    if(videoWorker.joinable()) videoWorker.join();
    // if(audioWorker.joinable()) audioWorker.join();
}
bool channel::add_clip(const shared_ptr<video>& vid, const chrono::milliseconds& at)
{
    clips.emplace_back(make_unique<clip>(vid, at));
    currentClip = clips.begin();
    stop();
    recalculate_lenght();
    start();
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
std::list<std::unique_ptr<clip>>& channel::get_clips()
{
    return clips;
}
const std::list<std::unique_ptr<clip>>& channel::get_clips() const
{
    return clips;
}
frame* channel::get_frame(const duration& timestamp)
{
    if(paused)
    {
        return nextFrame;
    }
    if(timestamp >= lenght)
    {
        return nullptr;
    }
    if(!nextFrame)
    {
        if(!frameQueue.try_pop(nextFrame))
            return nullptr;
    }

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
            if(!frameQueue.try_pop(nextFrame)) break;
            pts = nextFrame->timestamp;
        }
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
    videoPacketQueue.notify();
    audioPacketQueue.notify();
    frameQueue.notify();

    if(decodeWorker.joinable()) decodeWorker.join();
    if(videoWorker.joinable()) videoWorker.join();
    // if(audioWorker.joinable()) audioWorker.join();

    frameQueue.flush();
    videoPacketQueue.flush();
    audioPacketQueue.flush();
    drop_audio();

    nextFrame = prevFrame = nullptr;
}
void channel::start()
{
    stopped      = false;
    decodeWorker = worker_type(&channel::decoding_job, this);
    videoWorker  = worker_type(&channel::video_job, this);
    init_audio();
}
void channel::jump2(const chrono::milliseconds& ts)
{
    stop();

    auto it = find_if(
        clips.begin(), clips.end(),
        [&, this](auto& c) {
            return c->contains(ts);
        });
    if(it == clips.end()) return;

    auto cl = it->get();
    if(!cl->seek(ts))
    {
        throw runtime_error("Failed to jump to requested timestamp");
    }
    seek.store(true, memory_order_relaxed);
    seekPts = cl->video_reconvert_pts(ts);

    start();
}
void channel::decoding_job()
{
    while(!stopped)
    {
        if(currentClip != clips.end())
        {
            auto& c      = *currentClip;
            auto pPacket = av_packet_alloc();
            if(c->get_packet(&pPacket))
            {
                if(pPacket->stream_index == c->vsIndex)
                {
                    if(seek)
                    {
                        printf("%lld - %lld\n", pPacket->pts, seekPts);
                        seek = false;
                    }
                    auto p = new packet{ c.get(), pPacket };
                    videoPacketQueue.push(p, [this] { return stopped == true; });
                }
                else if(pPacket->stream_index == c->asIndex)
                {
                    auto p = new packet{ c.get(), pPacket };
                    audioPacketQueue.push(p, [this] { return stopped == true; });
                }
                else
                {
                    av_packet_unref(pPacket);
                }
            }
            else
            {
                // the clip has ended
                advance(currentClip, 1);
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
    AVFrame* pFrame = av_frame_alloc();
    while(!stopped)
    {
        packet* p = nullptr;
        videoPacketQueue.pop(p, [this] { return stopped == true; });
        if(!p) break;

        auto c = p->owner;
        if(c->get_frame(p->payload, &pFrame))
        {
            auto fr = new frame;
            c->convert_frame(pFrame, &fr);
            frameQueue.push(fr, [this] { return stopped == true; });
        }
        av_packet_unref(p->payload);
        av_packet_free(&p->payload);
    }
    av_frame_free(&pFrame);
}
void channel::recalculate_lenght()
{
    if(clips.empty())
        lenght = chrono::milliseconds(0);
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
        outParams.channelCount              = nbChannels;
        outParams.device                    = Pa_GetDefaultOutputDevice();
        outParams.suggestedLatency          = 0;
        outParams.hostApiSpecificStreamInfo = NULL;
        outParams.sampleFormat              = paFloat32;

        auto err = Pa_OpenStream(
            &audioStream,
            NULL,
            &outParams,
            sampleRate,
            nbFrames,
            paClipOff,
            pa_stream_callback,
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
int channel::pa_stream_callback(
    const void* /*input*/,
    void* output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData)
{
    static constexpr milliseconds NoSyncThreshold{ 24 };
    static constexpr milliseconds minusNoSyncThreshold{ -24 };
    auto& ch               = *(static_cast<channel*>(userData));
    auto out               = static_cast<float*>(output);
    auto& audioPacketQueue = ch.audioPacketQueue;
    auto& silentBuffer     = ch.silentBuffer;
    auto& audioBuffer      = ch.audioBuffer;
    auto& tl               = ch.tl;
    const auto& nbChannels = ch.nbChannels;

    int sampleCount = frameCount;
    if(ch.paused)
    {
        // if the channel is paused => fill the buffer with silence
        copy_n(silentBuffer.begin(), sampleCount * nbChannels, out);
        return 0;
    }

    packet* p = nullptr;
    auto ptr  = audioBuffer.data();
    while(sampleCount > 0)
    {
        if(!audioPacketQueue.top(p))
        {
            copy_n(silentBuffer.begin(), sampleCount * nbChannels, out);
            break;
        }
        auto tlTs       = tl.get_timestamp();
        auto* c         = p->owner;
        auto newPts     = c->audio_convert_pts(p->payload->pts);
        const auto diff = newPts - tlTs;
        if(diff > NoSyncThreshold)
        {
            // fill with silence
            copy_n(silentBuffer.begin(), sampleCount * nbChannels, out);
            return 0;
        }
        ch.audioPacketQueue.pop();
        if(diff < minusNoSyncThreshold)
        {
            continue;
        }
        auto bSize = c->get_audio_frame(p, audioBuffer);
        copy_n(audioBuffer.begin(), bSize * nbChannels, out);
        sampleCount -= bSize;
    }
    return 0;
}
} // namespace libpgmaker
