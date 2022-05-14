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
    tl(tl)
{
    silentBuffer.resize(nbChannels * nbFrames, 0.f);
    set_paused(true);
    init_audio();
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
            prevFrame = nextFrame;
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

    nextFrame = prevFrame = nullptr;
}
void channel::start()
{
    stopped      = false;
    decodeWorker = worker_type(&channel::decoding_job, this);
    videoWorker  = worker_type(&channel::video_job, this);
    // audioWorker  = worker_type(&channel::audio_job, this);
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
    /*
 PaStreamParameters outParams;
 outParams.channelCount              = nbChannels;
 outParams.device                    = Pa_GetDefaultOutputDevice();
 outParams.suggestedLatency          = Pa_GetDeviceInfo(outParams.device)->defaultLowOutputLatency;
 outParams.hostApiSpecificStreamInfo = NULL;
 outParams.sampleFormat              = paFloat32;

 auto err = Pa_OpenStream(
     &audioStream,
     NULL,
     &outParams,
     sampleRate,
     nbFrames,
     paClipOff,
     NULL,
     NULL);
 if(err != paNoError)
 {
     throw runtime_error("PortAudio failed while opening a stream with: " + string(Pa_GetErrorText(err)));
 }
 err = Pa_StartStream(audioStream);
 if(err != paNoError)
 {
     throw runtime_error("PortAudio failed while starting a stream with: " + string(Pa_GetErrorText(err)));
 }
 */
    /*
        auto err = Pa_OpenDefaultStream(
            &audioStream,
            0,
            nbChannels,
            paFloat32,
            sampleRate,
            nbFrames,
            pa_stream_callback,
            this);
            */

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
    audioBuffer.resize(nbChannels * nbFrames);
}
void channel::drop_audio()
{
    auto err = Pa_StopStream(audioStream);
    if(err != paNoError)
    {
        throw runtime_error("PortAudio failed while stopping a stream with: " + string(Pa_GetErrorText(err)));
    }
    err = Pa_CloseStream(audioStream);
    if(err != paNoError)
    {
        throw runtime_error("PortAudio failed while closing a stream with: " + string(Pa_GetErrorText(err)));
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
    static const double coef = std::exp(std::log(0.01) / 20);
    auto& ch                 = *(static_cast<channel*>(userData));
    auto out                 = static_cast<float*>(output);

    int sampleCount = frameCount;
    if(ch.paused)
    {
        // if the channel is paused => fill the buffer with silence
        copy_n(ch.silentBuffer.begin(), sampleCount * ch.nbChannels, out);
        return 0;
    }

    packet* p = nullptr;
    auto ptr  = ch.audioBuffer.data();
    while(sampleCount > 0)
    {
        auto tlTs = ch.tl.get_timestamp();
        if(!ch.audioPacketQueue.try_pop(p))
        {
            // no available frames => fill with silence
            copy_n(ch.silentBuffer.begin(), sampleCount * ch.nbChannels, out);
            break;
        }
        auto* c             = p->owner;
        auto&& [bSize, pts] = c->get_audio_frame(p, ptr);
        // sync code
        //////////////////////////////////////
        auto diff = pts - tlTs;
        printf("%lld\n", diff.count());
        // do we need to sync the audio?
        // TODO: implement accurate syncing every frame
        if(diff < minusNoSyncThreshold)
        {
            // skip this frame
            continue;
        }
        else if(diff > NoSyncThreshold)
        {
            ch.lastAudioFrame.pts = pts;
            ch.lastAudioFrame.data.assign(prev(ptr, bSize * ch.nbChannels), ptr);
            copy_n(ch.silentBuffer.begin(), sampleCount * ch.nbChannels, out);
            return 0;
        }
        //////////////////////////////////////
        sampleCount -= bSize;
    }
    copy(ch.audioBuffer.begin(), ch.audioBuffer.end(), out);
    return 0;
}
} // namespace libpgmaker
