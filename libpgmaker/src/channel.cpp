#include <libpgmaker/channel.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

namespace libpgmaker {
using namespace std;

channel::channel():
    currentClip{ clips.end() }, prevFrame{},
    nextFrame{}, stopped{ true }, paused{ true }
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
    frameQueue.notify();
    if(decodeWorker.joinable()) decodeWorker.join();
    if(videoWorker.joinable()) videoWorker.join();
}
bool channel::add_clip(const shared_ptr<video>& vid, const chrono::milliseconds& at)
{
    clips.emplace_back(make_unique<clip>(vid, at));
    currentClip = clips.begin();
    rebuild();
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
    return paused;
}
void channel::rebuild()
{
    stopped = true;
    videoPacketQueue.notify();
    frameQueue.notify();

    if(decodeWorker.joinable())
        decodeWorker.join();
    if(videoWorker.joinable())
        videoWorker.join();

    recalculate_lenght();

    frameQueue.flush();
    videoPacketQueue.flush();

    stopped      = false;
    decodeWorker = worker_type(&channel::decoding_job, this);
    videoWorker  = worker_type(&channel::video_job, this);
}
void channel::jump2(const chrono::milliseconds& ts)
{
    // TODO: implement seeking
    auto it = find_if(
        clips.begin(), clips.end(),
        [&, this](auto& c) {
            return c->contains(ts);
        });
    if(it == clips.end()) return;

    auto diff = ts - (*it)->startsAt;
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
    auto err = Pa_OpenDefaultStream(
        &audioStream,
        0,
        nbChannels,
        paFloat32,
        sampleRate,
        nbFrames,
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
    static std::vector<float> buff;
    auto& ch = *(channel*)userData;
    auto out = (float*)output;

    packet* p       = nullptr;
    int sampleCount = frameCount * nbChannels;
    if(ch.paused)
    {
        // std::memcpy(out, ch.silentBuffer.data(), sampleCount * 2 * sizeof(float));
        // std::copy(ch.silentBuffer.begin(), ch.silentBuffer.end(), out);
        std::copy_n(ch.silentBuffer.begin(), sampleCount, out);
    }
    else
    {
        while(sampleCount > 0)
        {
            if(ch.audioPacketQueue.try_pop(p))
            {
                auto* c  = p->owner;
                auto pts = c->get_audio_frame(p->payload, buff);
                sampleCount -= buff.size();
                std::copy(buff.begin(), buff.end(), out);
                std::advance(out, buff.size());
                buff.clear();
            }
            else
            {
                // std::memcpy(out, ch.silentBuffer.data(), sampleCount * 2 * sizeof(float));
                // std::copy(ch.silentBuffer.begin(), ch.silentBuffer.end(), out);
                std::copy_n(ch.silentBuffer.begin(), sampleCount, out);
                sampleCount = 0;
            }
        }
    }
    return 0;
}
} // namespace libpgmaker
