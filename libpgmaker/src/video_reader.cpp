#include <libpgmaker/video_reader.h>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>

#include <glad/glad.h>

#include <libpgmaker/clip.h>

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
    effect* ef)
{
    video_copier cpr(ef);
    cpr.open_input(in);
    cpr.open_output(out);
    cpr.process();
}
video_reader::video_copier::video_copier(effect* ef):
    ef(ef)
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    if(!(handle = glfwCreateWindow(100, 100, "slave0", NULL, NULL)))
    {
        throw runtime_error("Could not create a slave window");
    }
    glfwMakeContextCurrent(handle);

    streamingParams.video_codec      = "libx264";
    streamingParams.codec_priv_key   = "x264-params";
    streamingParams.codec_priv_value = "keyint=60:min-keyint=60:scenecut=0:force-cfr=1";
    streamingParams.muxer_opt_key    = "movflags";
    streamingParams.muxer_opt_value  = "frag_keyframe+empty_moov+delay_moov+default_base_moof";
}
video_reader::video_copier::~video_copier()
{
    glfwDestroyWindow(handle);
}
void video_reader::video_copier::open_input(const std::string& path)
{
    AVStream *videoStream, *audioStream;
    const AVCodec *videoCodec, *audioCodec;
    AVCodecContext *videoCodecCtx, *audioCodecCtx;
    int videoIndex, audioIndex;
    auto avfc = avformat_alloc_context();
    //
    int err = avformat_open_input(&avfc, path.c_str(), NULL, NULL);
    if(err != 0) throw runtime_error("asda");
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

    inParams.swsCtx  = nullptr;
    inParams.swsCtx  = sws_getCachedContext(inParams.swsCtx, inParams.videoCodecCtx->width, inParams.videoCodecCtx->height,
                                            inParams.videoCodecCtx->pix_fmt, inParams.videoCodecCtx->width, inParams.videoCodecCtx->height,
                                            AV_PIX_FMT_GBRAPF32, SWS_BILINEAR, NULL, NULL, NULL);
    inBuffer         = av_frame_alloc();
    inBuffer->width  = inParams.videoCodecCtx->width;
    inBuffer->height = inParams.videoCodecCtx->height;
    inBuffer->format = AV_PIX_FMT_GBRAPF32;
    // inBuffer->linesize[0] = 4 * sizeof(float) * inBuffer->width;
    if(av_frame_get_buffer(inBuffer, 0) != 0)
    {
        throw runtime_error("Failed to alloc buffer");
    }

    outBuffer         = av_frame_alloc();
    outBuffer->width  = inParams.videoCodecCtx->width;
    outBuffer->height = inParams.videoCodecCtx->height;
    // outBuffer->linesize[0] = 4 * sizeof(float) * outBuffer->width;
    outBuffer->format = AV_PIX_FMT_GBRAPF32;
    if(av_frame_get_buffer(outBuffer, 0) != 0)
    {
        throw runtime_error("Failed to alloc buffer");
    }
    auto width = inParams.videoCodecCtx->width, height = inParams.videoCodecCtx->height;
    ef->prepare(width, height);
}
void video_reader::video_copier::open_output(const std::string& path)
{
    auto realPath = std::filesystem::path(path);
    if(std::filesystem::exists(realPath)) std::filesystem::remove(realPath);

    avformat_alloc_output_context2(&outParams.avfm, NULL, NULL, path.c_str());
    prepare_video_encoder();
    prepare_audio_encoder();

    // outParams.avfm->avoid_negative_ts = AVFMT_AVOID_NEG_TS_MAKE_NON_NEGATIVE;

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
    outParams.videoCodec    = avcodec_find_encoder_by_name(streamingParams.video_codec.c_str());
    outParams.videoCodecCtx = avcodec_alloc_context3(outParams.videoCodec);
    av_opt_set(outParams.videoCodecCtx->priv_data, "preset", "fast", 0);
    av_opt_set(outParams.videoCodecCtx->priv_data, streamingParams.codec_priv_key.c_str(), streamingParams.codec_priv_value.c_str(), 0);

    outParams.videoCodecCtx->height              = inParams.videoCodecCtx->height;
    outParams.videoCodecCtx->width               = inParams.videoCodecCtx->width;
    outParams.videoCodecCtx->sample_aspect_ratio = inParams.videoCodecCtx->sample_aspect_ratio;
    if(outParams.videoCodec->pix_fmts)
        outParams.videoCodecCtx->pix_fmt = outParams.videoCodec->pix_fmts[0];
    else
        outParams.videoCodecCtx->pix_fmt = inParams.videoCodecCtx->pix_fmt;

    /////
    outParams.videoCodecCtx->max_b_frames = 1;
    outParams.videoCodecCtx->gop_size     = 12;

    outParams.videoStream->time_base = outParams.videoCodecCtx->time_base = av_inv_q(inputFramerate);
    // outParams.videoStream->r_frame_rate                                   = inParams.videoStream->r_frame_rate;
    outParams.videoStream->r_frame_rate = inputFramerate;
    outParams.videoCodecCtx->framerate  = inputFramerate;
    if(avcodec_open2(outParams.videoCodecCtx, outParams.videoCodec, NULL) < 0) throw runtime_error("ASDASD");
    avcodec_parameters_from_context(outParams.videoStream->codecpar, outParams.videoCodecCtx);
    outParams.swsCtx = nullptr;
    outParams.swsCtx = sws_getCachedContext(outParams.swsCtx, inParams.videoCodecCtx->width, inParams.videoCodecCtx->height,
                                            AV_PIX_FMT_GBRAPF32, outParams.videoCodecCtx->width, outParams.videoCodecCtx->height,
                                            outParams.videoCodecCtx->pix_fmt, SWS_BILINEAR, NULL, NULL, NULL);
}
void video_reader::video_copier::process()
{
    AVDictionary* muxer_opts = NULL;
    // av_dict_set(&muxer_opts, streamingParams.muxer_opt_key.c_str(), streamingParams.muxer_opt_value.c_str(), 0);

    if(avformat_write_header(outParams.avfm, &muxer_opts) < 0) throw runtime_error("ASDASD");
    auto inputFrame  = av_frame_alloc();
    auto inputPacket = av_packet_alloc();

    while(av_read_frame(inParams.avfm, inputPacket) >= 0)
    {
        if(inParams.avfm->streams[inputPacket->stream_index]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            transcode_video(inputPacket, inputFrame);
            av_packet_unref(inputPacket);
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

    sws_freeContext(inParams.swsCtx);
    sws_freeContext(outParams.swsCtx);

    avformat_close_input(&inParams.avfm);
    avformat_free_context(inParams.avfm);
    avformat_free_context(outParams.avfm);

    avcodec_free_context(&inParams.videoCodecCtx);
    avcodec_free_context(&inParams.audioCodecCtx);

    ef->cleanup();
    delete ef;
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
        sws_scale(inParams.swsCtx, inputFrame->data, inputFrame->linesize,
                  0, inputFrame->height,
                  inBuffer->data, inBuffer->linesize);

        ///////////////////////////////////////////////////////////////////
        ///////////////// RGBA - apply filter /////////////////////////////
        ef->process(inBuffer, outBuffer);

        ///////////////////////////////////////////////////////////////////
        outFrame = av_frame_alloc();
        av_frame_copy_props(outFrame, inputFrame);
        outFrame->width  = inputFrame->width;
        outFrame->height = inputFrame->height;
        std::copy_n(inputFrame->linesize, 8, outFrame->linesize);
        outFrame->format       = AV_PIX_FMT_YUV420P10;
        outFrame->pkt_duration = outParams.videoCodecCtx->time_base.den / (double)outParams.videoCodecCtx->time_base.num / outParams.videoStream->r_frame_rate.num * outParams.videoStream->r_frame_rate.den;
        outFrame->pts          = outFrame->pkt_duration * (nFrames++) * outParams.videoStream->time_base.den / (outParams.videoStream->time_base.num * (outParams.videoStream->r_frame_rate.num / outParams.videoStream->r_frame_rate.den));
        if(av_frame_get_buffer(outFrame, 0) != 0)
        {
            throw runtime_error("ADA");
        }
        sws_scale(outParams.swsCtx, outBuffer->data, outBuffer->linesize,
                  0, outBuffer->height,
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
void video_reader::export_clip(clip* c, const std::string& path)
{
    AVFormatContext *inAvfm, *outAvfm;
    AVStream *inVideoStream, *outVideoStream, *inAudioStream, *outAudioStream;
    const AVCodec *inVideoCodec, *outVideoCodec, *inAudioCodec, *outAudioCodec;
    AVCodecContext *inVideoCodecCtx, *outVideoCodecCtx, *inAudioCodecCtx, *outAudioCodecCtx;
    SwsContext *inSwsContext, *outSwsContext;
    int videoIndex = -1, audioIndex = -1;
    auto inPath      = c->get_info().path;
    auto startOffset = c->get_start_offset();
    auto endOffset   = c->get_end_offset();

    ////////////////////////////////////////////////////////////////////////
    // input
    ////////////////////////////////////////////////////////////////////////
    inAvfm  = avformat_alloc_context();
    int err = avformat_open_input(&inAvfm, inPath.c_str(), NULL, NULL);
    if(err != 0) throw runtime_error("ASDAS");
    //
    err = avformat_open_input(&inAvfm, path.c_str(), NULL, NULL);
    if(err != 0) throw runtime_error("asda");
    if(avformat_find_stream_info(inAvfm, NULL) < 0) throw runtime_error("asda");
    for(int i = 0; i < inAvfm->nb_streams; ++i)
    {
        if(inAvfm->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            inVideoStream = inAvfm->streams[i];
            videoIndex    = i;
            inVideoCodec  = avcodec_find_decoder(inVideoStream->codecpar->codec_id);
            //
            inVideoCodecCtx = avcodec_alloc_context3(inVideoCodec);
            //
            if(avcodec_parameters_to_context(inVideoCodecCtx, inVideoStream->codecpar) < 0) throw runtime_error("asdad");
            if(avcodec_open2(inVideoCodecCtx, inVideoCodec, NULL) < 0) throw runtime_error("ASDASD");
        }
        else if(inAvfm->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            inAudioStream = inAvfm->streams[i];
            audioIndex    = i;
            inAudioCodec  = avcodec_find_decoder(inAudioStream->codecpar->codec_id);
            //
            inAudioCodecCtx = avcodec_alloc_context3(inAudioCodec);
            //
            if(avcodec_parameters_to_context(inAudioCodecCtx, inAudioStream->codecpar) < 0) throw runtime_error("asdad");
            if(avcodec_open2(inAudioCodecCtx, inAudioCodec, NULL) < 0) throw runtime_error("ASDASD");
        }
        if(audioIndex != -1 && videoIndex != -1) break;
    }
    ////////////////////////////////////////////////////////////////////////
    // output
    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////
    avformat_alloc_output_context2(&outAvfm, NULL, NULL, path.c_str());
    ///
    AVRational inputFramerate = av_guess_frame_rate(inAvfm, inVideoStream, NULL);
    //
    outVideoStream   = avformat_new_stream(outAvfm, NULL);
    outVideoCodec    = avcodec_find_encoder_by_name("libx264");
    outVideoCodecCtx = avcodec_alloc_context3(outVideoCodec);
    av_opt_set(outVideoCodecCtx->priv_data, "preset", "fast", 0);
    av_opt_set(outVideoCodecCtx->priv_data, "x264-params", "keyint=60:min-keyint=60:scenecut=0:force-cfr=1", 0);

    outVideoCodecCtx->height              = inVideoCodecCtx->height;
    outVideoCodecCtx->width               = inVideoCodecCtx->width;
    outVideoCodecCtx->sample_aspect_ratio = inVideoCodecCtx->sample_aspect_ratio;
    if(outVideoCodec->pix_fmts)
        outVideoCodecCtx->pix_fmt = outVideoCodec->pix_fmts[0];
    else
        outVideoCodecCtx->pix_fmt = inVideoCodecCtx->pix_fmt;

    /////
    outVideoCodecCtx->max_b_frames = 1;
    outVideoCodecCtx->gop_size     = 12;

    outVideoStream->time_base = outVideoCodecCtx->time_base = av_inv_q(inputFramerate);
    outVideoStream->r_frame_rate                            = inputFramerate;
    outVideoCodecCtx->framerate                             = inputFramerate;
    if(avcodec_open2(outVideoCodecCtx, outVideoCodec, NULL) < 0) throw runtime_error("ASDASD");
    avcodec_parameters_from_context(outVideoStream->codecpar, outVideoCodecCtx);

    ///
    outAudioStream = avformat_new_stream(outAvfm, NULL);
    avcodec_parameters_copy(outAudioStream->codecpar, inAudioStream->codecpar);

    if(outAvfm->oformat->flags & AVFMT_GLOBALHEADER)
        outAvfm->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    if(!(outAvfm->oformat->flags & AVFMT_NOFILE))
    {
        if(avio_open(&outAvfm->pb, path.c_str(), AVIO_FLAG_WRITE) < 0) throw runtime_error("asdasd");
    }
    //////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
    if(avformat_write_header(outAvfm, NULL) < 0) throw runtime_error("ASDASD");
    auto inputFrame  = av_frame_alloc();
    auto inputPacket = av_packet_alloc();

    size_t nFrames    = 0;
    auto encode_video = [&](AVFrame* inputFrame) {
        AVFrame* outFrame = nullptr;
        if(inputFrame)
        {
            inputFrame->pict_type = AV_PICTURE_TYPE_NONE;

            ///////////////////////////////////////////////////////////////////
            outFrame = av_frame_alloc();
            av_frame_copy_props(outFrame, inputFrame);
            outFrame->width  = inputFrame->width;
            outFrame->height = inputFrame->height;
            std::copy_n(inputFrame->linesize, 8, outFrame->linesize);
            outFrame->format       = AV_PIX_FMT_YUV420P10;
            outFrame->pkt_duration = outVideoCodecCtx->time_base.den / (double)outVideoCodecCtx->time_base.num
                                     / outVideoStream->r_frame_rate.num * outVideoStream->r_frame_rate.den;
            outFrame->pts = outFrame->pkt_duration * (nFrames++)
                            * outVideoStream->time_base.den
                            / (outVideoStream->time_base.num
                               * (outVideoStream->r_frame_rate.num / outVideoStream->r_frame_rate.den));

            if(av_frame_get_buffer(outFrame, 0) != 0)
                throw runtime_error("ADA");
        }

        int r = avcodec_send_frame(outVideoCodecCtx, outFrame);

        if(r < 0 && !(r == AVERROR_EOF && !outFrame)) throw runtime_error("asdadi");

        auto outputPacket = av_packet_alloc();
        if(!outputPacket) throw runtime_error("ASDA");
        while(true)
        {
            r = avcodec_receive_packet(outVideoCodecCtx, outputPacket);

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
            outputPacket->stream_index = videoIndex;

            r = av_interleaved_write_frame(outAvfm, outputPacket);
            if(r != 0) throw runtime_error("ASDASD");
        }
        av_packet_unref(outputPacket);
        av_packet_free(&outputPacket);
    };

    auto transcode_video = [&](AVPacket* inputPacket, AVFrame* inputFrame) {
        int response = avcodec_send_packet(inVideoCodecCtx, inputPacket);
        if(response < 0) throw runtime_error("ASDASD");
        //
        while((response = avcodec_receive_frame(inVideoCodecCtx, inputFrame)) >= 0)
        {
            encode_video(inputFrame);
        }
        if(response == AVERROR(EAGAIN)) response = 0;
        if(response < 0 && response != AVERROR_EOF) throw runtime_error("ASDA");
        av_frame_unref(inputFrame);
    };
    while(av_read_frame(inAvfm, inputPacket) >= 0)
    {
        if(inAvfm->streams[inputPacket->stream_index]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            transcode_video(inputPacket, inputFrame);
            av_packet_unref(inputPacket);
        }
        else if(inAvfm->streams[inputPacket->stream_index]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            av_packet_rescale_ts(inputPacket, inAudioStream->time_base, outAudioStream->time_base);
            if(av_interleaved_write_frame(outAvfm, inputPacket) < 0) throw runtime_error("ASDA");
        }
    }
    encode_video(NULL);

    av_write_trailer(outAvfm);

    avformat_close_input(&inAvfm);
    avformat_free_context(inAvfm);
    avformat_free_context(outAvfm);

    avcodec_free_context(&inVideoCodecCtx);
    avcodec_free_context(&inAudioCodecCtx);

    avcodec_free_context(&outVideoCodecCtx);
    avcodec_free_context(&outAudioCodecCtx);
}
} // namespace libpgmaker
