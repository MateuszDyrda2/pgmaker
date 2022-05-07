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
    nextFrame{}, timestamp{ 0 }, stopped{ true }
{
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
bool channel::add_clip(const std::shared_ptr<video>& vid, const std::chrono::milliseconds& at)
{
    clips.emplace_back(make_unique<clip>(vid, at));
    currentClip = clips.begin();
    rebuild();
    return true;
}
frame* channel::get_frame(const std::chrono::milliseconds& delta)
{

    // dont do anything if the channel is empty
    if(clips.empty()) return nullptr;
    // increment time
    timestamp += delta;
    // end playing
    if(timestamp >= lenght)
    {
        // stopped = true;
        return nullptr;
    }
    // the channel just started
    // we don't have a next frame
    if(!nextFrame)
    {
        frameQueue.pop(nextFrame);
    }
    // when should the frame be displayed?
    auto pts = nextFrame->timestamp;
    // the frame is ready to be displayed
    if(timestamp >= pts)
    {
        // try to find a newer frame to display
        while(timestamp > pts)
        {
            prevFrame = nextFrame;
            if(!frameQueue.try_pop(nextFrame)) break;
            pts = nextFrame->timestamp;
        }
    }
    return prevFrame;
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
void channel::jump2(const std::chrono::milliseconds& ts)
{
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
                    // av_packet_unref(pPacket);
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
void channel::audio_decode_frame()
{
    packet* p       = nullptr;
    AVFrame* pFrame = av_frame_alloc();

    audioPacketQueue.pop(p);
    auto c = p->owner;
    if(c->get_frame(p->payload, &pFrame))
    {
        auto fr = new audio_frame;
    }
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
    auto err = Pa_Initialize();
    if(err != paNoError)
    {
        throw runtime_error("PortAudio failed while initializing with: " + string(Pa_GetErrorText(err)));
    }
    static constexpr size_t SAMPLE_RATE = 48000;

    err = Pa_OpenDefaultStream(
        &audioStream,
        0,
        2,
        paFloat32,
        SAMPLE_RATE,
        1024,
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
    err = Pa_Terminate();
    if(err != paNoError)
    {
        throw runtime_error("PortAudio failed while destroying with: " + string(Pa_GetErrorText(err)));
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
    auto& ch = *(channel*)userData;
    auto out = (float*)output;

    packet* p = nullptr;
    static std::vector<float> buff;
    int sampleCount = frameCount;
    while(sampleCount > 0)
    {
        ch.audioPacketQueue.pop(p);
        auto* c      = p->owner;
        int nbFrames = c->get_audio_frame(p->payload, buff);
        sampleCount -= (unsigned long)(0.5f * buff.size());
        std::memcpy(out, buff.data(), buff.size() * sizeof(float));
        out += buff.size();
        buff.clear();
    }
    return 0;

    /*
    static float val = 0.0f;
    static bool up   = true;
    for(int i = 0; i < frameCount; ++i)
    {
        *++out = val;
        if(up)
            val += 0.0001;
        else
            val -= 0.0001;

        if(val >= 1.f) up = false;
        if(val <= -1.f) up = true;
    }
    return 0;
    */
}
} // namespace libpgmaker
