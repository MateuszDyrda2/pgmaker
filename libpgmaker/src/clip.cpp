#include <libpgmaker/clip.h>

#include <cassert>

namespace libpgmaker {
clip::clip(const std::shared_ptr<video>& vid, std::chrono::milliseconds startsAt):
    vid(vid), startsAt(startsAt)
{
    assert(vid);
    if(!open_input(vid->get_path()))
        return;
    if(!fill_buffer())
        return;
}
clip::~clip()
{
}
bool clip::open_input(const std::string& path)
{
    if(!pixelFormat = avformat_alloc_context())
    {
        return false;
    }
    if(avformat_open_input(&pFormatCtx, path.c_str(), NULL, NULL) != 0)
    {
        return false;
    }
    if(avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        return false;
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
    if(!vsIndex == -1 || asIndex == -1)
    {
        return false;
    }
    const AVCodec* pCodec = avcodec_find_decoder(codecParams->codec_id);
    if(!pCodec)
    {
        return false;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if(avcodec_parameters_to_context(pCodecCtx, codecParams) < 0)
    {
        return false;
    }
    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        return false;
    }
    swsCtx = sws_getContext(width, height, pCodecCtx->pix_fmt,
                            width, height, AV_PIX_FMT_RGB0,
                            SWS_BILINEAR, NULL, NULL, NULL);
    if(!swsCtx)
    {
        return false;
    }
    return true;

    timebase = pFormatCtx->streams[vsIndex]->time_base;
}
bool clip::fill_buffer()
{
    static constexpr std::size_t bufferSize = 32;
    firstFrames.resize(bufferSize);

    for(std::size_t i = 0; i < bufferSize; ++i)
    {
        buffers[i] = new unsigned char[width * height * 4];
    }
    AVPacket* pPacket = av_packet_alloc();
    if(!pPacket)
    {
        return false;
    }
    AVFrame* pFrame = av_frame_alloc();
    if(!pFrame)
    {
        return false;
    }
    for(std::size_t i = 0; i < bufferSize; ++i)
    {
        while(av_read_frame(pFormatCtx, pPacket) >= 0)
        {
            if(pPacket->stream_index == vsIndex)
            {
                if(avcodec_send_packet(pCodecCtx, pPacket) < 0)
                    return false;
                if(int response = avcodec_receive_frame(pCodecCtx, pFrame);
                   response == AVERROR(EAGAIN) || response == AVERROR_EOF)
                {
                    av_packet_unref(pPacket);
                }
                else if(response < 0)
                    return false;
                else
                {
                    av_packet_unref(pPacket);
                    break;
                }
            }
        }
        std::uint8_t* buff     = new std::uint8_t[width * height * 4];
        unsigned char* dest[4] = { buff, NULL, NULL, NULL };
        int destLineSize[4]    = { int(width) * 4, 0, 0, 0 };
        sws_scale(swsCtx, pFrame->data, pFrame->linesize, 0, pFrame->height, dest, destLineSize);
        auto spts    = pFrame->pts * (double)timebase.num / (double)timebase.den;
        auto sptsDur = std::chrono::duration_cast<
            std::chrono::millisecond>(std::chrono::duration<double>(spts));
        firstFrames[i] = new frame{ { width, height },
                                    std::unique_ptr<std::uint8_t[]>(buff),
                                    sptsDur };
    }
    // Cleanup
    av_frame_free(&pFrame);
    av_packet_free(&pPacket);
    return true;
}
}

