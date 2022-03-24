#include <libpgmaker/video_reader.h>

#include <stdexcept>

#include <cstdint>

namespace libpgmaker {
using namespace std;
video_reader::video_reader()
{
}
video_reader::~video_reader()
{
}
std::unique_ptr<video> video_reader::load_file(const std::string& path)
{
    AVFormatContext* avFormatContext = avformat_alloc_context();
    if(!avFormatContext)
    {
        throw runtime_error("Couldn't allocate format context\n");
    }
    // Open the file
    if(avformat_open_input(&avFormatContext, path.c_str(), NULL, NULL) != 0)
    {
        throw runtime_error("Couldn't open input for file: " + path);
    }
    // Retrieve stream information
    if(avformat_find_stream_info(avFormatContext, NULL) < 0)
    {
        // Couldn't find stream information
        throw runtime_error("Couldn't find stream information");
    }
    av_dump_format(avFormatContext, 0, path.c_str(), 0);

    int videoStream = -1;
    int audioStream = -1;
    int width, height;
    AVCodecParameters* codecParams = nullptr;
    for(int i = 0; i < avFormatContext->nb_streams; ++i)
    {
        auto streams = avFormatContext->streams[i];
        if(streams->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            width       = streams->codecpar->width;
            height      = streams->codecpar->height;
            codecParams = streams->codecpar;
        }
        if(streams->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStream = i;
        }
    }
    if(videoStream == -1 || audioStream == -1)
    {
        throw runtime_error("Didn't find a video stream");
    }
    const AVCodec* pCodec =
        avcodec_find_decoder(avFormatContext->streams[videoStream]->codecpar->codec_id);
    if(pCodec == NULL)
    {
        throw runtime_error("Unsupported codec");
    }
    AVCodecContext* pCodecCtx = avcodec_alloc_context3(pCodec);
    if(avcodec_parameters_to_context(pCodecCtx, codecParams) < 0)
    {
        throw runtime_error("Could not pass parameters to codec");
    }
    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        throw runtime_error("Could not open codec");
    }

    AVFrame* pFrame = av_frame_alloc();
    if(!pFrame)
    {
        throw runtime_error("Failed to allocate a frame");
    }
    AVPacket* pPacket = av_packet_alloc();
    if(!pPacket)
    {
        throw runtime_error("Failed to allocate a packet");
    }
    /*
        size_t numBytes = width * height * 4; // for rgba 32bit
        auto buffer     = std::make_unique<std::uint8_t[]>(numBytes);

        while(av_read_frame(avFormatContext, pPacket) >= 0)
        {
            if(pPacket->stream_index != videoStream)
            {
                av_packet_unref(pPacket);
                continue;
            }
            int response = avcodec_send_packet(pCodecCtx, pPacket);
            if(response < 0)
            {
                throw runtime_error("Failed to decode a packet");
            }
            response = avcodec_receive_frame(pCodecCtx, pFrame);
            if(response == AVERROR(EAGAIN) || response == AVERROR_EOF)
            {
                av_packet_unref(pPacket);
                continue;
            }
            else if(response < 0)
            {
                throw runtime_error("Failed to decode a packet");
            }
            av_packet_unref(pPacket);
        }
        SwsContext* swsScalerCtx = sws_getContext(width, height, pCodecCtx->pix_fmt,
                                                  width, height, AV_PIX_FMT_RGB0,
                                                  SWS_BILINEAR,
                                                  NULL, NULL, NULL);
        if(!swsScalerCtx)
        {
            throw runtime_error("Could not initialize swscaler");
        }
        uint8_t* dest[4]   = { buffer.get(), NULL, NULL, NULL };
        int desLineSize[4] = { pFrame->width * 4, 0, 0, 0 };
        sws_scale(swsScalerCtx, pFrame->data, pFrame->linesize, 0, pFrame->height, dest, desLineSize);
    */
    video_state state;
    state.width            = width;
    state.height           = height;
    state.avFormatContext  = avFormatContext;
    state.avCodecContext   = pCodecCtx;
    state.videoStreamIndex = videoStream;
    state.avFrame          = pFrame;
    state.avPacket         = pPacket;
    state.timeBase         = avFormatContext->streams[videoStream]->time_base;

    auto vid = std::make_unique<video>(state);
    // vid->thumbnail = std::move(buffer);
    return vid;
#if 0
    int videoStreamIndex = -1, audioStreamIndex = -1;
    AVCodecParameters* avCodecParams;
    AVCodec* avCodec;
    for(size_t i = 0; i < avFormatContext->nb_streams; ++i)
    {
        avCodecParams = avFormatContext->streams[i]->codecpar;
        avCodec       = const_cast<AVCodec*>(avcodec_find_decoder(avCodecParams->codec_id));
        if(!avCodec) continue;
        if(avCodec->type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex = i;
        }
        else if(avCodec->type == AVMEDIA_TYPE_AUDIO)
        {
            audioStreamIndex = i;
        }
    }
    if(videoStreamIndex == -1)
    {
        throw runtime_error("Couldn't find video stream in file");
    }
#endif
}
} // namespace libpgmaker
