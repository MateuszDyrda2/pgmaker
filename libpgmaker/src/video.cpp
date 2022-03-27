#include <libpgmaker/video.h>

#include <stdexcept>

namespace libpgmaker {
using namespace std;
video::video(const video_state& state):
    state(state) { }
video::~video()
{
    // sws_freeContext(state.swsScalerContext);
    avformat_close_input(&state.avFormatContext);
    avformat_free_context(state.avFormatContext);
    av_frame_free(&state.avFrame);
    av_packet_free(&state.avPacket);
    avcodec_free_context(&state.avCodecContext);
}
thumbnail video::get_thumbnail(std::uint32_t width, std::uint32_t height)
{
    auto& avPacket        = state.avPacket;
    auto& avFormatContext = state.avFormatContext;
    auto& avFrame         = state.avFrame;
    auto& avCodecContext  = state.avCodecContext;

    size_t numBytes = width * height * 4;
    auto buffer     = std::make_unique<std::uint8_t[]>(numBytes);
    while(av_read_frame(avFormatContext, avPacket) >= 0)
    {
        if(avPacket->stream_index != state.videoStreamIndex)
        {
            av_packet_unref(avPacket);
            continue;
        }
        if(avcodec_send_packet(avCodecContext, avPacket) < 0)
        {
            throw runtime_error("Failed to decode a packet");
        }
        if(int response = avcodec_receive_frame(avCodecContext, avFrame);
           response == AVERROR(EAGAIN) || response == AVERROR_EOF)
        {
            av_packet_unref(avPacket);
        }
        else if(response < 0)
        {
            throw runtime_error("Failed to decode a packet");
        }
        av_packet_unref(avPacket);
        break;
    }
    SwsContext* swsScalerCtx = sws_getContext(state.width, state.height, avCodecContext->pix_fmt,
                                              width, height, AV_PIX_FMT_RGB0,
                                              SWS_BILINEAR, NULL, NULL, NULL);
    if(!swsScalerCtx)
    {
        throw runtime_error("Failed to initialize swscaler");
    }
    std::uint8_t* dest[4] = { buffer.get(), NULL, NULL, NULL };
    int destLineSize[4]   = { width * 4, 0, 0, 0 };
    sws_scale(swsScalerCtx, avFrame->data, avFrame->linesize, 0, avFrame->height, dest, destLineSize);

    sws_freeContext(swsScalerCtx);
    // restart file
    avio_seek(avFormatContext->pb, 0, SEEK_SET);
    avformat_seek_file(avFormatContext, state.videoStreamIndex,
                       0, 0, avFormatContext->streams[state.videoStreamIndex]->duration, 0);

    return thumbnail{ width, height, std::move(buffer) };
}
void video::tick_frame()
{
    auto& avFormatContext  = state.avFormatContext;
    auto& avCodecContext   = state.avCodecContext;
    auto& avFrame          = state.avFrame;
    auto& avPacket         = state.avPacket;
    auto& videoStreamIndex = state.videoStreamIndex;
    if(!state.swsScalerContext)
    {
        state.swsScalerContext = sws_getContext(state.width, state.height, avCodecContext->pix_fmt,
                                                state.width, state.height, AV_PIX_FMT_RGB0,
                                                SWS_BILINEAR, NULL, NULL, NULL);
        if(!state.swsScalerContext)
        {
            throw runtime_error("Could not initialize swscaler");
        }
    }
    if(!data.data)
    {
        data.data   = std::make_unique<std::uint8_t[]>(state.width * state.height * 4);
        data.width  = state.width;
        data.height = state.height;
    }
    auto& swsScalerContext = state.swsScalerContext;
    while(av_read_frame(avFormatContext, avPacket) >= 0)
    {
        if(avPacket->stream_index != videoStreamIndex)
        {
            av_packet_unref(avPacket);
            continue;
        }
        if(avcodec_send_packet(avCodecContext, avPacket) < 0)
        {
            throw runtime_error("Failed to decode packet");
        }
        if(int response = avcodec_receive_frame(avCodecContext, avFrame);
           response == AVERROR(EAGAIN) || response == AVERROR_EOF)
        {
            av_packet_unref(avPacket);
            continue;
        }
        else if(response < 0)
        {
            throw runtime_error("Failed to decode packet");
        }
        av_packet_unref(avPacket);
        break;
    }
    // presentation timestamp
    int pts = avFrame->pts;

    std::uint8_t* dest[4] = { data.data.get(), NULL, NULL, NULL };
    int destLineSize[4]   = { avFrame->width * 4, 0, 0, 0 };
    sws_scale(swsScalerContext, avFrame->data, avFrame->linesize,
              0, avFrame->height, dest, destLineSize);
}
} // namespace libpgmaker
