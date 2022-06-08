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

void video_reader::copy_with_effect(
    const std::string& in,
    const std::string& out,
    const std::function<void(void*, void*, int, int)>& effect)
{
    video_copier cpr(effect);
    cpr.open_input(in);
    cpr.open_output(out);
    cpr.process();
}
video_reader::video_copier::video_copier(const std::function<void(void*, void*, int, int)>& effect):
    effect(effect)
{
    streamingParams.video_codec      = "libx264";
    streamingParams.codec_priv_key   = "x264-params";
    streamingParams.codec_priv_value = "keyint=60:min-keyint=60:scenecut=0:force-cfr=1";
    streamingParams.muxer_opt_key    = "movflags";
    streamingParams.muxer_opt_value  = "frag_keyframe+empty_moov+delay_moov+default_base_moof";
}
void video_reader::video_copier::open_input(const std::string& path)
{
    AVStream *videoStream, *audioStream;
    const AVCodec *videoCodec, *audioCodec;
    AVCodecContext *videoCodecCtx, *audioCodecCtx;
    int videoIndex, audioIndex;
    auto avfc = avformat_alloc_context();
    //
    if(avformat_open_input(&avfc, path.c_str(), NULL, NULL) != 0) throw runtime_error("asda");
    if(avformat_find_stream_info(avfc, NULL) < 0) throw runtime_error("asda");
    for(int i = 0; i < avfc->nb_streams; ++i)
    {
        if(avfc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = avfc->streams[i];
            videoIndex  = i;
            videoCodec  = avcodec_find_decoder(videoStream->codecpar->codec_id);
            //
            videoCodecCtx = avcodec_alloc_context3(videoCodec);
            //
            if(avcodec_parameters_to_context(videoCodecCtx, videoStream->codecpar) < 0) throw runtime_error("asdad");
            if(avcodec_open2(videoCodecCtx, videoCodec, NULL) < 0) throw runtime_error("ASDASD");
        }
        else if(avfc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStream = avfc->streams[i];
            audioIndex  = i;
            audioCodec  = avcodec_find_decoder(audioStream->codecpar->codec_id);
            //
            audioCodecCtx = avcodec_alloc_context3(audioCodec);
            //
            if(avcodec_parameters_to_context(audioCodecCtx, audioStream->codecpar) < 0) throw runtime_error("asdad");
            if(avcodec_open2(audioCodecCtx, audioCodec, NULL) < 0) throw runtime_error("ASDASD");
        }
    }
    inParams.avfm          = avfc;
    inParams.videoStream   = videoStream;
    inParams.audioStream   = audioStream;
    inParams.videoCodec    = videoCodec;
    inParams.audioCodec    = audioCodec;
    inParams.videoCodecCtx = videoCodecCtx;
    inParams.audioCodecCtx = audioCodecCtx;
    inParams.videoIndex    = videoIndex;
    inParams.audioIndex    = audioIndex;

    inParams.swsCtx = nullptr;
    inParams.swsCtx = sws_getCachedContext(inParams.swsCtx, inParams.videoCodecCtx->width, inParams.videoCodecCtx->height,
                                           inParams.videoCodecCtx->pix_fmt, inParams.videoCodecCtx->width, inParams.videoCodecCtx->height,
                                           AV_PIX_FMT_RGB0, SWS_BILINEAR, NULL, NULL, NULL);
}
void video_reader::video_copier::open_output(const std::string& path)
{
    avformat_alloc_output_context2(&outParams.avfm, NULL, NULL, path.c_str());
    prepare_video_encoder();
    prepare_audio_encoder();

    if(outParams.avfm->oformat->flags & AVFMT_GLOBALHEADER)
        outParams.avfm->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    if(!(outParams.avfm->oformat->flags & AVFMT_NOFILE))
    {
        if(avio_open(&outParams.avfm->pb, path.c_str(), AVIO_FLAG_WRITE) < 0) throw runtime_error("asdasd");
    }
}
void video_reader::video_copier::prepare_audio_encoder()
{
    outParams.audioStream = avformat_new_stream(outParams.avfm, NULL);
    avcodec_parameters_copy(outParams.audioStream->codecpar, inParams.audioStream->codecpar);
}
void video_reader::video_copier::prepare_video_encoder()
{
    AVRational inputFramerate = av_guess_frame_rate(inParams.avfm, inParams.videoStream, NULL);
    //
    outParams.videoStream   = avformat_new_stream(outParams.avfm, NULL);
    outParams.videoCodec    = avcodec_find_encoder_by_name(streamingParams.video_codec);
    outParams.videoCodecCtx = avcodec_alloc_context3(outParams.videoCodec);
    AVDictionary* dict      = NULL;
    av_opt_set(dict, "preset", "fast", 0);
    av_opt_set(dict, streamingParams.codec_priv_key, streamingParams.codec_priv_value, 0);

    outParams.videoCodecCtx->height              = inParams.videoCodecCtx->height;
    outParams.videoCodecCtx->width               = inParams.videoCodecCtx->width;
    outParams.videoCodecCtx->sample_aspect_ratio = inParams.videoCodecCtx->sample_aspect_ratio;
    if(outParams.videoCodec->pix_fmts)
        outParams.videoCodecCtx->pix_fmt = outParams.videoCodec->pix_fmts[0];
    else
        outParams.videoCodecCtx->pix_fmt = inParams.videoCodecCtx->pix_fmt;

    // outParams.videoCodecCtx->bit_rate       = 2'000'000;
    // outParams.videoCodecCtx->rc_min_rate    = 2'000'000;
    // outParams.videoCodecCtx->rc_max_rate    = 5'000'000;
    // outParams.videoCodecCtx->rc_buffer_size = 4'000'000;
    outParams.videoStream->time_base = outParams.videoCodecCtx->time_base = av_inv_q(inputFramerate);
    outParams.videoStream->r_frame_rate                                   = inParams.videoStream->r_frame_rate;
    // outParams.videoStream->time_base = outParams.videoCodecCtx->time_base = av_inv_q(inParams.videoCodecCtx->framerate);
    if(avcodec_open2(outParams.videoCodecCtx, outParams.videoCodec, &dict) < 0) throw runtime_error("ASDASD");
    avcodec_parameters_from_context(outParams.videoStream->codecpar, outParams.videoCodecCtx);
    outParams.swsCtx = nullptr;
    outParams.swsCtx = sws_getCachedContext(outParams.swsCtx, inParams.videoCodecCtx->width, inParams.videoCodecCtx->height,
                                            AV_PIX_FMT_RGB0, outParams.videoCodecCtx->width, outParams.videoCodecCtx->height,
                                            outParams.videoCodecCtx->pix_fmt, SWS_BILINEAR, NULL, NULL, NULL);
}
void video_reader::video_copier::process()
{
    inBuffer  = new uint8_t[inParams.videoCodecCtx->width * inParams.videoCodecCtx->height * 4];
    outBuffer = new uint8_t[inParams.videoCodecCtx->width * inParams.videoCodecCtx->height * 4];

    AVDictionary* muxer_opts = NULL;
    av_dict_set(&muxer_opts, streamingParams.muxer_opt_key, streamingParams.muxer_opt_value, 0);

    if(avformat_write_header(outParams.avfm, &muxer_opts) < 0) throw runtime_error("ASDASD");
    auto inputFrame  = av_frame_alloc();
    auto inputPacket = av_packet_alloc();

    while(av_read_frame(inParams.avfm, inputPacket) >= 0)
    {
        if(inParams.avfm->streams[inputPacket->stream_index]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            transcode_video(inputPacket, inputFrame);
            av_packet_unref(inputPacket);
            // remux(inputPacket, inParams.videoStream->time_base, outParams.videoStream->time_base);
        }
        else if(inParams.avfm->streams[inputPacket->stream_index]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            remux(inputPacket, inParams.audioStream->time_base, outParams.audioStream->time_base);
        }
    }
    encode_video(NULL);

    av_write_trailer(outParams.avfm);

    if(muxer_opts != nullptr)
        av_dict_free(&muxer_opts);
    if(inputFrame != nullptr)
        av_frame_free(&inputFrame);
    if(inputPacket != nullptr)
        av_packet_free(&inputPacket);

    avformat_close_input(&inParams.avfm);
    avformat_free_context(inParams.avfm);
    avformat_free_context(outParams.avfm);

    avcodec_free_context(&inParams.videoCodecCtx);
    avcodec_free_context(&inParams.audioCodecCtx);

    delete[] inBuffer;
    delete[] outBuffer;
}

void video_reader::video_copier::transcode_video(AVPacket* inputPacket, AVFrame* inputFrame)
{
    int response = avcodec_send_packet(inParams.videoCodecCtx, inputPacket);
    if(response < 0) throw runtime_error("ASDASD");
    //
    while((response = avcodec_receive_frame(inParams.videoCodecCtx, inputFrame)) >= 0)
    {
        encode_video(inputFrame);
    }

    if(response == AVERROR(EAGAIN))
        response = 0;

    if(response < 0 && response != AVERROR_EOF)
    {
        throw runtime_error("ASDA");
    }
    av_frame_unref(inputFrame);
}
void video_reader::video_copier::encode_video(AVFrame* inputFrame)
{
    AVFrame* outFrame = nullptr;
    if(inputFrame)
    {
        inputFrame->pict_type = AV_PICTURE_TYPE_NONE;

        std::uint8_t* dest[4] = { inBuffer, nullptr, nullptr, nullptr };
        int destLineSize[4]   = { int(inParams.videoCodecCtx->width) * 4, 0, 0, 0 };
        sws_scale(inParams.swsCtx, inputFrame->data, inputFrame->linesize,
                  0, inputFrame->height,
                  dest, destLineSize);

        ///////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////
        ///////////////// RGBA - apply filter /////////////////////////////
        effect(outBuffer, inBuffer, inParams.videoCodecCtx->width, inParams.videoCodecCtx->height);
        ///////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////
        static size_t nFrames = 0;
        outFrame              = av_frame_alloc();
        outFrame->width       = inputFrame->width;
        outFrame->height      = inputFrame->height;
        std::copy_n(inputFrame->linesize, 8, outFrame->linesize);
        // outFrame->linesize[0]  = inputFrame->linesize[0];
        outFrame->format       = AV_PIX_FMT_YUV420P10;
        outFrame->pkt_duration = outParams.videoCodecCtx->time_base.den / (double)outParams.videoCodecCtx->time_base.num / outParams.videoStream->r_frame_rate.num * outParams.videoStream->r_frame_rate.den;
        outFrame->pts          = outFrame->pkt_duration * (nFrames++) * outParams.videoStream->time_base.den / (outParams.videoStream->time_base.num * (outParams.videoStream->r_frame_rate.num / outParams.videoStream->r_frame_rate.den));
        if(av_frame_get_buffer(outFrame, 0) != 0)
        {
            throw runtime_error("ADA");
        }
        dest[0] = outBuffer;

        sws_scale(outParams.swsCtx, dest, destLineSize,
                  0, inParams.videoCodecCtx->height,
                  outFrame->data, outFrame->linesize);
    }

    int r = avcodec_send_frame(outParams.videoCodecCtx, outFrame);
    if(r < 0 && !(r == AVERROR_EOF && !outFrame))
    {
        throw runtime_error("asdadi");
    }

    auto outputPacket = av_packet_alloc();
    if(!outputPacket) throw runtime_error("ASDA");
    while(true)
    {
        r = avcodec_receive_packet(outParams.videoCodecCtx, outputPacket);

        if(r == AVERROR(EAGAIN))
        {
            break;
        }
        else if(r == AVERROR_EOF)
        {
            break;
        }
        else if(r < 0)
        {
            throw runtime_error("ASDASAD");
        }
        outputPacket->stream_index = inParams.videoIndex;
        // outputPacket->duration     = outParams.videoStream->time_base.den / (inParams.videoStream->r_frame_rate.num / inParams.videoStream->r_frame_rate.den);
        // outputPacket->duration = dur;
        // outputPacket->duration = (outParams.videoStream->time_base.den / (double)outParams.videoStream->time_base.num) / (inParams.videoStream->r_frame_rate.num / (double)inParams.videoStream->r_frame_rate.den);
        // av_packet_rescale_ts(outputPacket, inParams.videoStream->time_base, outParams.videoStream->time_base);

        r = av_interleaved_write_frame(outParams.avfm, outputPacket);
        if(r != 0) throw runtime_error("ASDASD");
    }
    av_packet_unref(outputPacket);
    av_packet_free(&outputPacket);
}
void video_reader::video_copier::remux(AVPacket* inputPacket, AVRational inTimebase, AVRational outTimebase)
{
    av_packet_rescale_ts(inputPacket, inTimebase, outTimebase);
    if(av_interleaved_write_frame(outParams.avfm, inputPacket) < 0) throw runtime_error("ASDA");
}
} // namespace libpgmaker
