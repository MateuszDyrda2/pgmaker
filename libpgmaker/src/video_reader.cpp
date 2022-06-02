#include <libpgmaker/video_reader.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>

#include <cstdint>

namespace libpgmaker {
using namespace std;
namespace fs = filesystem;
inline bool file_exists(const std::string& path)
{
    ifstream f(path);
    return f.good();
}
video_reader::video_handle video_reader::load_file(const std::string& path)
{
    if(!file_exists(path))
    {
        throw runtime_error("File " + path + " does not exist in the current context");
    }
    auto pFormatCtx = avformat_alloc_context();
    if(!pFormatCtx)
    {
        throw runtime_error("Could not allocate format context for file " + path);
    }
    if(avformat_open_input(&pFormatCtx, path.c_str(), NULL, NULL) != 0)
    {
        throw runtime_error("Could not open input file " + path);
    }
    return video_handle(pFormatCtx, path);
}
video_reader::video_handle::video_handle(AVFormatContext* ctx, const std::string& path):
    pFormatContext(ctx), pCodecContext{}, vsIndex{ -1 },
    info{}, thumbnail{}
{ 
    info.path = path;
}
video_reader::video_handle::~video_handle()
{
    if(pCodecContext)
        avcodec_free_context(&pCodecContext);

    if(pFormatContext)
    {
        avformat_close_input(&pFormatContext);
        avformat_free_context(pFormatContext);
    }
}
video_reader::video_handle& video_reader::video_handle::load_metadata()
{
    if(avformat_find_stream_info(pFormatContext, NULL) < 0)
    {
        throw runtime_error("Could not find stream information");
    }
    AVCodecParameters* pParams;
    for(int i = 0; i < pFormatContext->nb_streams; ++i)
    {
        auto streams = pFormatContext->streams[i];
        if(streams->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            pParams     = streams->codecpar;
            vsIndex     = i;
            info.width  = streams->codecpar->width;
            info.height = streams->codecpar->height;
            break;
        }
    }
    if(vsIndex == -1)
    {
        throw runtime_error("Could not find a video stream");
    };
    const AVCodec* pCodec = avcodec_find_decoder(pParams->codec_id);
    if(pCodec == nullptr)
    {
        throw runtime_error("Unsupported codec");
    }
    pCodecContext = avcodec_alloc_context3(pCodec);
    if(avcodec_parameters_to_context(pCodecContext, pParams) < 0)
    {
        throw runtime_error("Could not pass parameters to context");
    }
    if(pCodec == nullptr)
    {
        throw runtime_error("Could not allocate codec context");
    }
    if(avcodec_open2(pCodecContext, pCodec, NULL) < 0)
    {
        throw runtime_error("Could not open codec");
    }
    auto& vStream = pFormatContext->streams[vsIndex];
    auto timeBase = vStream->time_base.num / (double)vStream->time_base.den;
    auto duration = vStream->duration * timeBase * 1000.0;
    info.duration = chrono::duration_cast<chrono::milliseconds>(
        chrono::duration<double, std::milli>(duration));
    info.pixelFormat = pCodecContext->pix_fmt;
    info.codecId     = pCodecContext->codec_id;
    info.bitrate     = pCodecContext->bit_rate;
    fs::path p       = info.path;
    info.name        = p.stem().string();

    return *this;
}
video_reader::video_handle& video_reader::video_handle::load_thumbnail()
{
    auto swsCtx = sws_getContext(info.width, info.height, pCodecContext->pix_fmt,
                                 info.width, info.height, AV_PIX_FMT_RGB0,
                                 SWS_BILINEAR, NULL, NULL, NULL);
    if(!swsCtx)
    {
        throw runtime_error("Failed to allocate sws context");
    }
    auto pPacket = av_packet_alloc();
    if(!pPacket)
    {
        // do sth
        throw runtime_error("Could not allocate packet");
    }

    auto pFrame = av_frame_alloc();
    if(!pFrame)
    {
        // do sth
        throw runtime_error("Could not allocate frame");
    }

    while(av_read_frame(pFormatContext, pPacket) >= 0)
    {
        if(pPacket->stream_index != vsIndex)
        {
            continue;
        }
        auto response = avcodec_send_packet(pCodecContext, pPacket);
        if(response < 0)
            throw runtime_error("Failed to decode a packet");

        response = avcodec_receive_frame(pCodecContext, pFrame);
        if(response == AVERROR(EAGAIN) || response == AVERROR_EOF)
        {
            continue;
        }
        else if(response < 0)
            throw runtime_error("Failed to decode a packet");
        av_packet_unref(pPacket);
        break;
    }
    // convert to rgba

    thumbnail              = unique_ptr<uint8_t[]>(new unsigned char[info.width * info.height * 4]);
    unsigned char* dest[4] = { thumbnail.get(), NULL, NULL, NULL };
    int destLineSize[4]    = { int(info.width) * 4, 0, 0, 0 };
    sws_scale(swsCtx, pFrame->data, pFrame->linesize, 0, pFrame->height, dest, destLineSize);

    sws_freeContext(swsCtx);
    av_frame_free(&pFrame);
    av_packet_free(&pPacket);
    return *this;
}
shared_ptr<video> video_reader::video_handle::get()
{
    return make_shared<video>(info, std::move(thumbnail));
}
} // namespace libpgmaker
