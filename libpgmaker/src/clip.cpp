#include <chrono>
#include <cstdint>
#include <libpgmaker/clip.h>
#include <libpgmaker/id_generator.h>

#include <cassert>
#include <stdexcept>

namespace libpgmaker {
using namespace std;
clip::clip(const std::shared_ptr<video>& vid, std::chrono::milliseconds startsAt):
    vid(vid),
    name(vid->get_info().name), startOffset{}, endOffset{}, startsAt(startsAt),
    size{},
    pFormatCtx{}, pVideoCodecCtx{}, pAudioCodecCtx{},
    vsIndex{ -1 }, asIndex{ -1 },
    swsCtx{}, vidTimebase{}, audioTimebase{}, clipId(id_generator::get_next())
{
    assert(vid);
    open_input(vid->get_info().path);
    audioFrame = av_frame_alloc();
}

clip::clip(const std::shared_ptr<video>& vid, const milliseconds& startsAt,
           const milliseconds& startOffset, const milliseconds& endOffset):
    vid(vid),
    name(vid->get_info().name), startsAt(startsAt), startOffset(startOffset),
    endOffset(endOffset), size{},
    pFormatCtx{}, pVideoCodecCtx{}, pAudioCodecCtx{},
    vsIndex{ -1 }, asIndex{ -1 },
    swsCtx{}, vidTimebase{}, audioTimebase{}, clipId(id_generator::get_next())
{
    assert(vid);
    open_input(vid->get_info().path);
    audioFrame = av_frame_alloc();
}
clip::~clip()
{
    av_frame_free(&audioFrame);
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
            size    = { streams->codecpar->width, streams->codecpar->height };
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
    swsCtx = sws_getContext(size.first, size.second, pVideoCodecCtx->pix_fmt,
                            size.first, size.second, AV_PIX_FMT_RGB0,
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
void clip::set_starts_at(const milliseconds& startsAt)
{
    this->startsAt = startsAt;
}
void clip::set_start_offset(const milliseconds& startOffset)
{
    this->startOffset = startOffset;
    seek_start();
}
void clip::change_start_offset(const milliseconds& by)
{
    auto realBy = by;
    if(startOffset + realBy < milliseconds(0))
        realBy = -startOffset;

    if(startOffset + realBy > vid->get_info().duration - endOffset)
        return;

    startOffset += realBy;
    startsAt += realBy;

    // seek_impl(startOffset);
    // seek_start();
}
void clip::set_end_offset(const milliseconds& endOffset)
{
    this->endOffset = endOffset;
}
void clip::change_end_offset(const milliseconds& by)
{
    auto realBy = by;
    if(endOffset + realBy < milliseconds(0))
        realBy = -endOffset;

    if(endOffset + realBy > vid->get_info().duration - startOffset)
        return;

    endOffset += realBy;
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
    // seek_start();
    seek_impl(startOffset);
}
bool clip::get_packet(packet& pPacket)
{
    AVPacket* p = av_packet_alloc();
    auto res    = av_read_frame(pFormatCtx, p);
    if(res == AVERROR_EOF)
        return false;
    else if(res < 0)
    {
        return true;
    }

    const auto time      = p->pts * vidTimebase.num / (double)vidTimebase.den;
    const auto millitime = chrono::duration<double>(time);
    const auto endtime   = vid->get_info().duration - endOffset;
    vidCurrentStreamPos  = p->pos;
    vidCurrentTs         = p->pts;
    pPacket.owner        = this;
    pPacket.payload      = p;
    return (millitime < endtime);
}
bool clip::get_frame(packet& pPacket, AVFrame** frame)
{
    if(avcodec_send_packet(pVideoCodecCtx, pPacket.payload) < 0)
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
    // auto buff             = new std::uint8_t[width * height * 4];
    std::vector<std::uint8_t> buff(size.first * size.second * 4);
    std::uint8_t* dest[4] = { buff.data(), nullptr, nullptr, nullptr };
    int destLineSize[4]   = { int(size.first) * 4, 0, 0, 0 };
    sws_scale(swsCtx, iFrame->data, iFrame->linesize,
              0, iFrame->height,
              dest, destLineSize);

    (*oFrame)->owner  = this;
    (*oFrame)->data   = std::move(buff);
    const auto pts    = iFrame->pts * vidTimebase.num / double(vidTimebase.den);
    const auto ts     = chrono::duration_cast<milliseconds>(chrono::duration<double>(pts));
    const auto realTs = startsAt - startOffset + ts;
    av_frame_unref(iFrame);

    (*oFrame)->timestamp = realTs;
}
std::size_t clip::get_audio_frame(packet* pPacket, vector<float>& b)
{
    if(avcodec_send_packet(pAudioCodecCtx, pPacket->payload) < 0)
    {
        throw runtime_error("Failed to decode a packet");
    }
    std::size_t bSize = 0;
    chrono::milliseconds realTs(0);
    auto ptr = b.data();
    // may contain multiple frames
    for(;;)
    {
        //        AVFrame* frame = av_frame_alloc();
        if(int response = avcodec_receive_frame(pAudioCodecCtx, audioFrame);
           response == AVERROR(EAGAIN) || response == AVERROR_EOF)
        {
            break;
        }
        else if(response < 0)
        {
            throw runtime_error("Failed to decode a packet");
        }
        float* buffer[] = { ptr };

        auto cSamples = swr_convert(swrCtx, (std::uint8_t**)buffer,
                                    audioFrame->nb_samples,
                                    const_cast<const std::uint8_t**>(audioFrame->extended_data),
                                    audioFrame->nb_samples);
        std::advance(ptr, cSamples * nbChannels);
        bSize += cSamples;

        av_frame_unref(audioFrame);
    }

    return bSize;
}
bool clip::contains(const std::chrono::milliseconds& ts) const
{
    return ts >= startsAt && ts < startsAt + get_duration();
}
bool clip::seek(const milliseconds& ts)
{
    // TODO: fix
    assert(ts >= startsAt);
    assert(ts <= startsAt + get_duration());

    auto realTs = ts - startsAt + startOffset;
    return seek_impl(realTs);
}
bool clip::seek_impl(const milliseconds& localTs)
{
    auto currentPos = video_convert_pts(vidCurrentTs);
    auto diff       = localTs - currentPos;

    const auto currentPosInSec = chrono::duration_cast<chrono::duration<double>>(currentPos);
    const auto diffInSec       = chrono::duration_cast<chrono::duration<double>>(diff);
    const auto reqInSec        = chrono::duration_cast<chrono::duration<double>>(localTs);

    std::int64_t curr = currentPosInSec.count() * AV_TIME_BASE;
    std::int64_t inc  = diffInSec.count() * AV_TIME_BASE;
    std::int64_t req  = reqInSec.count() * AV_TIME_BASE;

    std::int64_t seekMin = inc > 0 ? curr + 2 : INT64_MIN;
    std::int64_t seekMax = inc < 0 ? curr - 2 : INT64_MAX;

    auto err = avformat_seek_file(pFormatCtx, -1, seekMin, req, seekMax, 0);
    if(err < 0)
    {
        char buffer[128];
        av_strerror(err, buffer, sizeof(buffer));
        printf("%s\n", buffer);
        return false;
    }
    return true;
}
clip::milliseconds clip::audio_convert_pts(std::int64_t pts) const
{
    const auto p  = pts * audioTimebase.num / double(audioTimebase.den);
    const auto ts = chrono::duration_cast<milliseconds>(chrono::duration<double>(p));
    return startsAt - startOffset + ts;
}
std::int64_t clip::audio_reconvert_pts(const milliseconds& pts) const
{
    const auto regTs = chrono::duration_cast<chrono::duration<double>>(pts - startsAt + startOffset);
    return regTs.count() * audioTimebase.den / (double)audioTimebase.num;
}
clip::milliseconds clip::video_convert_pts(std::int64_t pts) const
{
    const auto p  = pts * vidTimebase.num / double(vidTimebase.den);
    const auto ts = chrono::duration_cast<milliseconds>(chrono::duration<double>(p));
    return startsAt - startOffset + ts;
}
std::int64_t clip::video_reconvert_pts(const milliseconds& pts) const
{
    const auto regTs = chrono::duration_cast<chrono::duration<double>>(pts - startsAt + startOffset);
    return regTs.count() * vidTimebase.den / (double)vidTimebase.num;
}
}
