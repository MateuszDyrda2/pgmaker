#include <libpgmaker/channel.h>

namespace libpgmaker {
channel::channel(/* args */)
{
    decodeWorker = worker_type(&channel::decoding_job, this);
    videoWorker  = worker_type(&channel::video_job, this);
}
channel::~channel()
{
}
std::optional<frame> channel::get_frame()
{
    if(frame currentFrame; frameQueue.pop(currentFrame))
    {
        return currentFrame;
    }
    else
    {
        return {};
    }
}
void channel::decoding_job()
{
    while(true)
    {
        auto& current = *currentClip;
        if(auto vid = current.vid.lock())
        {
            auto& avFormatContext = vid->avFormatContext;
            auto& avCodecContext  = vid->avCodecContext;
            auto& swsContext      = vid->swsContext;
            auto videoStreamIndex = vid->videoStreamIndex;
            AVPacket* packet      = av_packet_alloc();
            while(av_read_frame(avFormatContext, packet) >= 0)
            {
                if(packet->stream_index == videoStreamIndex)
                {
                    // check if full
                    while(!packetQueue.push(packet))
                        ;
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
    while(true)
    {
        auto current = *currentClip;
        if(auto vid = current.vid.lock())
        {
            auto& avFormatContext = vid->avFormatContext;
            auto& avCodecContext  = vid->avCodecContext;
            auto& swsContext      = vid->swsContext;
            auto videoStreamIndex = vid->videoStreamIndex;

            AVPacket* packet = nullptr;
            AVFrame* avFrame = av_frame_alloc();
            while(!packetQueue.pop(packet))
                ;
            if(avcodec_send_packet(avCodecContext, packet) < 0)
            {
                // error decoding a packet
            }
            if(int response = avcodec_receive_frame(avCodecContext, avFrame);
               response == AVERROR(EAGAIN) || response == AVERROR_EOF)
            {
                av_packet_unref(packet);
            }
            else if(response < 0)
            {
                // error decoding a packet
            }
            av_packet_unref(packet);
            auto& info            = vid->information;
            std::uint8_t* buff    = new std::uint8_t[info.width
                                                  * info.height
                                                  * 4];
            std::uint8_t* dest[4] = { buff, nullptr, nullptr, nullptr };
            int destLineSize[4]   = { info.width * 4, 0, 0, 0 };
            sws_scale(swsContext, avFrame->data, avFrame->linesize,
                      0, avFrame->width,
                      dest, destLineSize);
            while(!frameQueue.push(frame{ buff, info.width, info.height }))
                ;
        }
    }
}
} // namespace libpgmaker
