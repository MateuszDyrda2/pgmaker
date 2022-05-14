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
    pFormatCtx{}, pVideoCodecCtx{}, pAudioCodecCtx{},
    vsIndex{ -1 }, asIndex{ -1 },
    swsCtx{}, vidTimebase{}, audioTimebase{}
{
    assert(vid);
    open_input(vid->get_path());
}
clip::~clip()
{
    swr_free(&swrCtx);
    sws_freeContext(swsCtx);
    avcodec_free_context(&pVideoCodecCtx);
    avcodec_free_context(&pAudioCodecCtx);
    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);
}
void clip::open_input(const string& path)
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
    for(int i = 0; i < pFormatCtx->nb_streams; ++i)
    {
        const auto streams = pFormatCtx->streams[i];
        if(streams->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            vsIndex = i;
            width   = streams->codecpar->width;
            height  = streams->codecpar->height;
        }
        else if(streams->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            asIndex    = i;
            sampleRate = streams->codecpar->sample_rate;
            nbChannels = streams->codecpar->channels;
        }
    }
    if(vsIndex == -1 || asIndex == -1)
    {
        throw runtime_error("The file does not contain video or audio streams: " + path);
    }
    if(!open_codec(pFormatCtx->streams[vsIndex]->codecpar, &pVideoCodecCtx))
    {
        throw runtime_error("Could not allocate codec for video");
    }
    if(!open_codec(pFormatCtx->streams[asIndex]->codecpar, &pAudioCodecCtx))
    {
        throw runtime_error("Could not allocate codec for audio");
    }

    // convert to rgba
    swsCtx = sws_getContext(width, height, pVideoCodecCtx->pix_fmt,
                            width, height, AV_PIX_FMT_RGB0,
                            SWS_BILINEAR, NULL, NULL, NULL);
    if(!swsCtx)
    {
        throw runtime_error("Could not allocate SWScaler context");
    }
    swrCtx = swr_alloc_set_opts(
        NULL,
        pAudioCodecCtx->channel_layout,
        AV_SAMPLE_FMT_FLT,
        pAudioCodecCtx->sample_rate,
        pAudioCodecCtx->channel_layout,
        pAudioCodecCtx->sample_fmt,
        pAudioCodecCtx->sample_rate,
        0, 0);
    swr_init(swrCtx);
    if(!swr_is_initialized(swrCtx))
    {
        throw runtime_error("Could not allocate SWResample context");
    }

    vidTimebase   = pFormatCtx->streams[vsIndex]->time_base;
    audioTimebase = pFormatCtx->streams[asIndex]->time_base;
}
bool clip::open_codec(AVCodecParameters* codecParams, AVCodecContext** ctx)
{
    if(const auto codec = avcodec_find_decoder(codecParams->codec_id))
    {
        *ctx = avcodec_alloc_context3(codec);
        if(avcodec_parameters_to_context(*ctx, codecParams) < 0) return false;
        if(avcodec_open2(*ctx, codec, NULL) < 0) return false;
        return true;
    }
    return false;
}
void clip::move_to(const milliseconds& startsAt)
{
    this->startsAt = startsAt;
}
void clip::change_start_offset(const milliseconds& startOffset)
{
    this->startOffset = startOffset;
    seek_start();
}
void clip::change_end_offset(const milliseconds& endOffset)
{
    this->endOffset = endOffset;
}
void clip::seek_start()
{
    const auto pts = startOffset.count() * vidTimebase.den / (double)vidTimebase.num;

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

    const auto time      = (*pPacket)->pts * vidTimebase.num / (double)vidTimebase.den;
    const auto millitime = chrono::duration<double>(time);
    const auto endtime   = vid->get_info().duration - endOffset;

    return (millitime < endtime);
}
bool clip::get_frame(AVPacket* pPacket, AVFrame** frame)
{
    if(avcodec_send_packet(pVideoCodecCtx, pPacket) < 0)
    {
        throw runtime_error("Failed to decode a packet");
    }
    if(int response = avcodec_receive_frame(pVideoCodecCtx, *frame);
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
void clip::convert_frame(AVFrame* iFrame, frame** oFrame)
{
    auto buff             = new std::uint8_t[width * height * 4];
    std::uint8_t* dest[4] = { buff, nullptr, nullptr, nullptr };
    int destLineSize[4]   = { int(width) * 4, 0, 0, 0 };
    sws_scale(swsCtx, iFrame->data, iFrame->linesize,
              0, iFrame->height,
              dest, destLineSize);

    (*oFrame)->size   = { width, height };
    (*oFrame)->data   = unique_ptr<std::uint8_t[]>(buff);
    const auto pts    = iFrame->pts * vidTimebase.num / double(vidTimebase.den);
    const auto ts     = chrono::duration_cast<milliseconds>(chrono::duration<double>(pts));
    const auto realTs = startsAt - startOffset + ts;

    (*oFrame)->timestamp = realTs;
}
pair<std::size_t, chrono::milliseconds> clip::get_audio_frame(packet* pPacket, float*& b)
{
    if(avcodec_send_packet(pAudioCodecCtx, pPacket->payload) < 0)
    {
        throw runtime_error("Failed to decode a packet");
    }
    std::size_t bSize = 0;
    chrono::milliseconds realTs(0);
    // may contain multiple frames
    for(;;)
    {
        AVFrame* frame = av_frame_alloc();
        if(int response = avcodec_receive_frame(pAudioCodecCtx, frame);
           response == AVERROR(EAGAIN) || response == AVERROR_EOF)
        {
            av_frame_free(&frame);
            break;
        }
        else if(response < 0)
        {
            throw runtime_error("Failed to decode a packet");
        }
        float* buffer[] = { b };

        auto cSamples       = swr_convert(swrCtx, (std::uint8_t**)buffer,
                                          frame->nb_samples,
                                          const_cast<const std::uint8_t**>(frame->extended_data),
                                          frame->nb_samples);
        const auto nbBuffer = cSamples * nbChannels;
        std::advance(b, nbBuffer);
        bSize += cSamples;
        const auto pts = frame->pts * audioTimebase.num / double(audioTimebase.den);
        const auto ts  = chrono::duration_cast<milliseconds>(chrono::duration<double>(pts));
        if(realTs == chrono::milliseconds(0))
            realTs = startsAt - startOffset + ts;

        av_frame_free(&frame);
    }

    return { bSize, realTs };
}
bool clip::contains(const std::chrono::milliseconds& ts) const
{
    return ts >= startsAt && ts < startsAt + get_duration();
}
bool clip::seek(const milliseconds& ts)
{
    assert(ts >= startsAt);
    assert(ts <= startsAt + get_duration());
    auto regTs   = ts - startsAt + startOffset;
    uint64_t pts = regTs.count() * vidTimebase.den / (double)vidTimebase.num;
    return (avformat_seek_file(pFormatCtx, vsIndex, INT64_MIN, pts, INT64_MAX, 0) >= 0);
}
}
