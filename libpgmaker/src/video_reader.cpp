#include <libpgmaker/video_reader.h>

#include <chrono>
#include <memory>
#include <stdexcept>

#include <cstdint>
#include <libpgmaker/pg_types.h>

namespace libpgmaker {
using namespace std;
video_reader::video_reader()
{
}
video_reader::~video_reader()
{
}
std::shared_ptr<video> video_reader::load_file(const std::string& path)
{
    AVFormatContext* pFormatCtx;
    AVCodecContext* pCodecCtx;
    AVFrame* pFrame;
    AVPacket* pPacket;
    SwsContext* swsCtx;
    int vsIndex = -1, asIndex = -1;
    std::uint32_t width, height;

    pFormatCtx = avformat_alloc_context();
    if(!pFormatCtx)
    {
        throw runtime_error("Couldn't allocate format context\n");
    }
    // Open the file
    if(avformat_open_input(&pFormatCtx, path.c_str(), NULL, NULL) != 0)
    {
        throw runtime_error("Couldn't open input for file: " + path);
    }
    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        // Couldn't find stream information
        throw runtime_error("Couldn't find stream information");
    }
    av_dump_format(pFormatCtx, 0, path.c_str(), 0);

    AVCodecParameters* codecParams = nullptr;
    for(int i = 0; i < pFormatCtx->nb_streams; ++i)
    {
        auto streams = pFormatCtx->streams[i];
        if(streams->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            vsIndex     = i;
            width       = streams->codecpar->width;
            height      = streams->codecpar->height;
            codecParams = streams->codecpar;
        }
        if(streams->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            asIndex = i;
        }
    }
    if(vsIndex == -1 || asIndex == -1)
    {
        throw runtime_error("Didn't find a video stream");
    }
    const AVCodec* pCodec =
        avcodec_find_decoder(codecParams->codec_id);
    if(pCodec == NULL)
    {
        throw runtime_error("Unsupported codec");
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    if(avcodec_parameters_to_context(pCodecCtx, codecParams) < 0)
    {
        throw runtime_error("Could not pass parameters to codec");
    }
    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        throw runtime_error("Could not open codec");
    }
    pPacket = av_packet_alloc();
    if(!pPacket)
    {
        // do sth
    }

    pFrame = av_frame_alloc();
    if(!pFrame)
    {
        // do sth
    }
    while(av_read_frame(pFormatCtx, pPacket) >= 0)
    {
        if(pPacket->stream_index == vsIndex)
        {
            if(avcodec_send_packet(pCodecCtx, pPacket) < 0)
                throw runtime_error("Failed to decode a packet");
            if(int response = avcodec_receive_frame(pCodecCtx, pFrame);
               response == AVERROR(EAGAIN) || response == AVERROR_EOF)
            {
                av_packet_unref(pPacket);
            }
            else if(response < 0)
                throw runtime_error("Failed to decode a packet");
            else
            {
                av_packet_unref(pPacket);
                break;
            }
        }
        else
        {
            av_packet_unref(pPacket);
        }
    }
    // convert to rgba
    swsCtx = sws_getContext(width, height, pCodecCtx->pix_fmt,
                            width, height, AV_PIX_FMT_RGB0,
                            SWS_BILINEAR, NULL, NULL, NULL);
    if(!swsCtx)
    {
        // do sth
    }
    unsigned char* buffer  = new unsigned char[width * height * 4];
    unsigned char* dest[4] = { buffer, NULL, NULL, NULL };
    int destLineSize[4]    = { int(width) * 4, 0, 0, 0 };
    sws_scale(swsCtx, pFrame->data, pFrame->linesize, 0, pFrame->height, dest, destLineSize);

    video_info info;
    info.width  = width;
    info.height = height;

    auto& vStream = pFormatCtx->streams[vsIndex];
    auto timeBase = vStream->time_base.num / (double)vStream->time_base.den;
    auto duration = vStream->duration * timeBase * 1000.0;
    info.duration = chrono::duration_cast<chrono::milliseconds>(
        chrono::duration<double, std::milli>(duration));
    info.pixelFormat = pCodecCtx->pix_fmt;
    info.codecId     = pCodecCtx->codec_id;
    info.bitrate     = pCodecCtx->bit_rate;

    thumbnail tn{ width, height, unique_ptr<uint8_t[]>(buffer) };

    sws_freeContext(swsCtx);
    av_frame_free(&pFrame);
    av_packet_free(&pPacket);
    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);
    avcodec_free_context(&pCodecCtx);

    return std::make_shared<video>(path, info, std::move(tn));
}
} // namespace libpgmaker
