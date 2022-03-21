#include <libpgmaker/video_reader.h>

#include <stdexcept>

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
    if(avformat_open_input(&avFormatContext, path.c_str(), NULL, NULL) != 0)
    {
        throw runtime_error("Couldn't open input for file: " + path);
    }
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
}
} // namespace libpgmaker
