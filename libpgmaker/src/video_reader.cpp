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
std::shared_ptr<video> video_reader::load_file(const std::string& path)
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
        avcodec_find_decoder(codecParams->codec_id);
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
    SwsContext* swsCtx = sws_getContext(width, height, pCodecCtx->pix_fmt,
                                        width, height, AV_PIX_FMT_RGB0,
                                        SWS_BILINEAR, NULL, NULL, NULL);

    video_state state;
    state.width            = width;
    state.height           = height;
    state.avFormatContext  = avFormatContext;
    state.avCodecContext   = pCodecCtx;
    state.videoStreamIndex = videoStream;
    state.audioStreamIndex = audioStream;
    state.swsScalerContext = swsCtx;
    state.timeBase         = avFormatContext->streams[videoStream]->time_base;

    video_info info;
    info.width  = width;
    info.height = height;

    return std::make_shared<video>(state, info);
}
} // namespace libpgmaker
