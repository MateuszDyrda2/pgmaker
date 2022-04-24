#include <libpgmaker/channel.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

namespace libpgmaker {
using namespace std;

channel::channel():
    currentClip{ clips.end() }, finished{ false }, prevFrame{},
    nextFrame{}, timestamp{ 0 }, stopped{ true }
{
}
channel::~channel()
{
    finished.store(false);
}
bool channel::add_clip(const std::shared_ptr<video>& vid, const std::chrono::milliseconds& at)
{
    clips.emplace_back(vid, at);
    currentClip = clips.begin();
    recalculate_lenght();
    return true;
}
frame* channel::get_frame(const std::chrono::milliseconds& delta)
{
    if(stopped)
    {
        decodeWorker = worker_type(&channel::decoding_job, this);
        decodeWorker.detach();
        videoWorker = worker_type(&channel::video_job, this);
        videoWorker.detach();
        stopped = false;
    }
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
}
void channel::jump2(const std::chrono::milliseconds& ts)
{
}
void channel::decoding_job()
{
    while(!finished.load())
    {
        if(currentClip != clips.end())
        {
            auto& c      = *currentClip;
            auto pPacket = av_packet_alloc();
            if(c.get_packet(&pPacket))
            {
                if(pPacket->stream_index == c.vsIndex)
                {
                    auto p = new packet{ &c, pPacket };
                    packetQueue.push(p);
                }
                else if(pPacket->stream_index == c.asIndex)
                {
                    av_packet_unref(pPacket);
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
    while(!finished.load())
    {
        packet* p = nullptr;
        packetQueue.pop(p);

        auto c = p->owner;
        if(finished) break;
        if(c->get_frame(p->payload, &pFrame))
        {
            auto fr = new frame;
            c->scale_frame(pFrame, &fr);
            frameQueue.push(fr);
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
        lenght           = last.startsAt + last.get_duration();
    }
}
} // namespace libpgmaker