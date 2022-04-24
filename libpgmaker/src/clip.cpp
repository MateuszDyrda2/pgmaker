#include <chrono>
#include <cstdint>
#include <libpgmaker/clip.h>

#include <cassert>
#include <stdexcept>

namespace libpgmaker {
using namespace std;
clip::clip(const std::shared_ptr<video>& vid, std::chrono::milliseconds startsAt):
    vid(vid), startOffset{}, endOffset{}, startsAt(startsAt),
    width{}, height{},
    pFormatCtx{}, pCodecCtx{}, vsIndex{ -1 }, asIndex{ -1 },
    swsCtx{}, timebase{}
{
    assert(vid);
    open_input(vid->get_path());
    //    fill_buffer();
}
clip::~clip()
{
    sws_freeContext(swsCtx);
    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);
    avcodec_free_context(&pCodecCtx);
}
void clip::open_input(const std::string& path)
{
    if(!(pFormatCtx = avformat_alloc_context()))
    {
        throw runtime_error("Failed to allocate context");
    }
    if(avformat_open_input(&pFormatCtx, path.c_str(), NULL, NULL) != 0)
    {
        throw runtime_error("Failed to open input for " + path);
    }
    if(avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        throw runtime_error("Could not find stream information for " + path);
    }
    AVCodecParameters* codecParams = nullptr;
    for(int i = 0; i < pFormatCtx->nb_streams; ++i)
    {
        auto streams = pFormatCtx->streams[i];
        if(streams->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            vsIndex     = i;
            codecParams = streams->codecpar;
            width       = codecParams->width;
            height      = codecParams->height;
        }
        else if(streams->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            asIndex = i;
        }
    }
    if(vsIndex == -1 || asIndex == -1)
    {
        throw runtime_error("The file does not contain video or audio streams: " + path);
    }
    const AVCodec* pCodec = avcodec_find_decoder(codecParams->codec_id);
    if(!pCodec)
    {
        throw runtime_error("Could not allocate codec for video");
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if(!pCodecCtx)
    {
        throw runtime_error("Could not allocate codec context");
    }
    if(avcodec_parameters_to_context(pCodecCtx, codecParams) < 0)
    {
        throw runtime_error("Could not set codec context parameters");
    }
    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        throw runtime_error("Could not open codec for file " + path);
    }
    swsCtx = sws_getContext(width, height, pCodecCtx->pix_fmt,
                            width, height, AV_PIX_FMT_RGB0,
                            SWS_BILINEAR, NULL, NULL, NULL);
    if(!swsCtx)
    {
        throw runtime_error("Could not allocate SWScaler context");
    }

    timebase = pFormatCtx->streams[vsIndex]->time_base;
}
void clip::move_to(const std::chrono::milliseconds& startsAt)
{
    this->startsAt = startsAt;
}
void clip::change_start_offset(const std::chrono::milliseconds& startOffset)
{
    this->startOffset = startOffset;
    seek_start();
}
void clip::change_end_offset(const std::chrono::milliseconds& endOffset)
{
    this->endOffset = endOffset;
}
void clip::seek_start()
{
    const auto pts = startOffset.count() * timebase.den / (double)timebase.num;

    if(avformat_seek_file(pFormatCtx, vsIndex, INT64_MIN, pts, INT64_MAX, 0) < 0)
    {
        throw runtime_error("Failed to seek frame");
    }
}
void clip::reset()
{
    seek_start();
}
bool clip::get_packet(AVPacket** pPacket)
{
    if(av_read_frame(pFormatCtx, *pPacket) < 0)
        return false;

    const auto time      = (*pPacket)->pts * timebase.num / (double)timebase.den;
    const auto millitime = chrono::duration<double>(time);
    const auto endtime   = vid->get_info().duration - endOffset;

    return (millitime < endtime);
}
bool clip::get_frame(AVPacket* pPacket, AVFrame** frame)
{
    if(avcodec_send_packet(pCodecCtx, pPacket) < 0)
    {
        throw runtime_error("Failed to decode a packet");
    }
    if(int response = avcodec_receive_frame(pCodecCtx, *frame);
       response == AVERROR(EAGAIN) || response == AVERROR_EOF)
    {
        return false;
    }
    else if(response < 0)
    {
        throw runtime_error("Failed to decode a packet");
    }
    return true;
}
void clip::scale_frame(AVFrame* iFrame, frame** oFrame)
{
    auto buff             = new std::uint8_t[width * height * 4];
    std::uint8_t* dest[4] = { buff, nullptr, nullptr, nullptr };
    int destLineSize[4]   = { int(width) * 4, 0, 0, 0 };
    sws_scale(swsCtx, iFrame->data, iFrame->linesize,
              0, iFrame->height,
              dest, destLineSize);

    (*oFrame)->size   = { width, height };
    (*oFrame)->data   = unique_ptr<std::uint8_t[]>(buff);
    const auto pts    = iFrame->pts * timebase.num / double(timebase.den);
    const auto ts     = chrono::duration_cast<chrono::milliseconds>(chrono::duration<double>(pts));
    const auto realTs = startsAt - startOffset + ts;

    (*oFrame)->timestamp = realTs;
}
bool clip::contains(const std::chrono::milliseconds& ts) const
{
    return ts >= startsAt && ts < get_ends_at();
}
}
