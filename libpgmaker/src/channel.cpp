#include <libpgmaker/channel.h>

#include <iostream>

namespace libpgmaker {
channel::channel():
    currentClip{ clips.end() }, finished{ false },
    previousFrame{}, nextFrame{}, timestamp{ 0 }
{
    decodeWorker = worker_type(&channel::decoding_job, this);
    videoWorker  = worker_type(&channel::video_job, this);
}
channel::~channel()
{
    finished.store(false);
    decodeWorker.join();
    videoWorker.join();
}
bool channel::add_clip(const std::shared_ptr<video>& vid, const std::chrono::milliseconds& at)
{
    clips.emplace_back(vid, at);
    currentClip = clips.begin();
    return true;
}
frame* channel::get_frame(const std::chrono::milliseconds& delta)
{
    timestamp += delta;
    auto beg = timestamp - currentClip->startsAfter + currentClip->startOffset;

    if(!previousFrame)
    {
        while(!frameQueue.pop(previousFrame))
            ;
    }
    if(!nextFrame)
    {
        nextFrame = previousFrame;
    }

    auto nextFrameTimestamp = nextFrame->timestamp;

    if(beg >= nextFrameTimestamp)
    {
        auto currentFrame = nextFrame;
        while(beg > nextFrameTimestamp)
        {
            delete currentFrame;
            currentFrame = nextFrame;
            if(!frameQueue.pop(nextFrame))
                break;
            nextFrameTimestamp = nextFrame->timestamp;
        }
        delete previousFrame;
        previousFrame = currentFrame;
    }
    std::cout << beg.count() << " and " << previousFrame->timestamp.count() << '\n';
    return previousFrame;
}
void channel::decoding_job()
{
    while(!finished.load())
    {
        if(currentClip != clips.end())
        {
            auto vid              = currentClip->vid;
            auto& vidState        = vid->get_state();
            auto& avFormatContext = vidState.avFormatContext;
            auto videoStreamIndex = vidState.videoStreamIndex;
            auto audioStreamIndex = vidState.audioStreamIndex;
            AVPacket* packet      = av_packet_alloc();
            if(av_read_frame(avFormatContext, packet) >= 0)
            {
                if(packet->stream_index == videoStreamIndex)
                {
                    // check if full
                    while(!packetQueue.push(packet) && !finished.load())
                        ;
                }
                else if(packet->stream_index == audioStreamIndex)
                {
                    // audio stream
                    av_packet_unref(packet);
                }
                else
                {
                    av_packet_unref(packet);
                }
            }
        }
    }
}
void channel::video_job()
{
    AVFrame* avFrame = av_frame_alloc();
    while(!finished.load())
    {
        if(currentClip != clips.end())
        {
            auto vid              = currentClip->vid;
            auto& vidState        = vid->get_state();
            auto& avFormatContext = vidState.avFormatContext;
            auto& avCodecContext  = vidState.avCodecContext;
            auto& swsContext      = vidState.swsScalerContext;
            auto& timeBase        = vidState.timeBase;
            auto videoStreamIndex = vidState.videoStreamIndex;

            AVPacket* packet = nullptr;
            while(!packetQueue.pop(packet) && !finished.load())
                ;
            if(avcodec_send_packet(avCodecContext, packet) < 0)
            {
                // error decoding a packet
            }
            if(int response = avcodec_receive_frame(avCodecContext, avFrame);
               response == AVERROR(EAGAIN) || response == AVERROR_EOF)
            {
                av_packet_unref(packet);
                av_packet_free(&packet);
                continue;
            }
            else if(response < 0)
            {
                // error decoding a packet
            }
            av_packet_unref(packet);
            av_packet_free(&packet);
            auto& info = vid->information;

            auto buff             = new std::uint8_t[info.width
                                         * info.height
                                         * 4];
            std::uint8_t* dest[4] = { buff, nullptr, nullptr, nullptr };
            int destLineSize[4]   = { info.width * 4, 0, 0, 0 };
            sws_scale(swsContext, avFrame->data, avFrame->linesize,
                      0, avFrame->height,
                      dest, destLineSize);
            auto spts    = avFrame->pts * (double)timeBase.num / (double)timeBase.den;
            auto sptsDur = std::chrono::duration<double>(spts);

            auto fr = new frame{ { info.width, info.height },
                                 std::unique_ptr<std::uint8_t[]>(buff),
                                 std::chrono::duration_cast<std::chrono::milliseconds>(sptsDur) };
            while(!frameQueue.push(fr) && !finished.load())
                ;
        }
    }
    av_frame_free(&avFrame);
}
} // namespace libpgmaker
