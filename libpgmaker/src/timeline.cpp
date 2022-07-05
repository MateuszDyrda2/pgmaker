#include <libpgmaker/timeline.h>

#include <algorithm>
#include <cassert>

#include <glad/glad.h>

namespace libpgmaker {
using namespace std;
using namespace chrono_literals;
timeline::timeline():
    paused(false), size(),
    ts(0), tsChecked()
{
    rebuild();
    set_paused(true);
    initialize_audio();
}
timeline::timeline(timeline&& other) noexcept:
    paused(other.paused), size(other.size),
    ts(0), tsChecked(), channels(std::move(other.channels))
{
}
timeline& timeline::operator=(timeline&& other) noexcept
{
    if(this != &other)
    {
        paused    = other.paused;
        size      = other.size;
        ts        = milliseconds(0);
        tsChecked = time_point();
        channels  = std::move(other.channels);
    }
    return *this;
}
timeline::~timeline()
{
    auto s = textures.size();
    for(size_t i = 0; i < s; ++i)
    {
        destroy_texture();
    }
    drop_audio();
}
channel* timeline::add_channel()
{
    generate_texture(channels.size());
    return channels.emplace_back(make_unique<channel>(this, channels.size())).get();
}
void timeline::remove_channel(std::size_t index)
{
    destroy_texture();
    assert(index < channels.size());
    channels.erase(channels.begin() + index);
}
void timeline::remove_channel(channel* ch)
{
    destroy_texture();
    assert(ch != nullptr);
    auto toEr = find_if(channels.begin(), channels.end(),
                        [&](auto& item) { return item.get() == ch; });
    if(toEr == channels.end())
        return;

    channels.erase(toEr);
}
channel* timeline::get_channel(std::size_t index)
{
    assert(index < channels.size());
    return channels[index].get();
}
channel* timeline::operator[](std::size_t index)
{
    return channels[index].get();
}
const channel* timeline::get_channel(std::size_t index) const
{
    assert(index < channels.size());
    return channels[index].get();
}
std::deque<std::unique_ptr<channel>>& timeline::get_channels()
{
    return channels;
}
const std::deque<std::unique_ptr<channel>>& timeline::get_channels() const
{
    return channels;
}
const channel* timeline::operator[](std::size_t index) const
{
    return channels[index].get();
}
std::pair<const std::vector<timeline::texture_t>&, std::size_t> timeline::next_frame()
{
    if(channels.empty())
        return { textures, 0 };

    if(!paused)
    {
        auto now = chrono::high_resolution_clock::now();
        ts += chrono::duration_cast<milliseconds>(
            now - tsChecked);
        tsChecked = now;
    }
    vector<frame*> frames;
    for(auto& ch : channels)
    {
        auto frame = ch->next_frame(ts);
        if(frame) frames.push_back(frame);
    }

    for(size_t index = 0; index < frames.size(); ++index)
    {
        texture_for_frame(index, frames[index]);
    }

    return { textures, frames.size() };
}
std::pair<const std::vector<timeline::texture_t>&, std::size_t> timeline::next_frame_blocking(const milliseconds& ts)
{
    if(channels.empty()) return { textures, 0 };
    vector<frame*> frames;
    for(auto& ch : channels)
    {
        auto frame = ch->next_frame_blocking(ts);
        if(frame) frames.push_back(frame);
    }
    for(size_t index = 0; index < frames.size(); ++index)
    {
        texture_for_frame(index, frames[index]);
    }
    return { textures, frames.size() };
}
/*
void timeline::export_timeline(const std::string& path, int64_t framerate)
{
    seek(milliseconds(0));
    AVFormatContext* formatCtx{};
    AVStream *videoStream{}, *audioStream{};
    const AVCodec *videoCodec{}, *audioCodec{};
    AVCodecContext *videoCodecCtx{}, *audioCodecCtx{};
    SwsContext* swsCtx{};
    int videoIndex{ 0 }, audioIndex{ 1 };
    const char* video_codec      = "libx264";
    const char* audio_codec      = "aac";
    const char* codec_priv_key   = "x264-params";
    const char* codec_priv_value = "keyint=60:min-keyint=60:scenecut=0:force-cfr=1";
    const char* muxer_opt_key    = "movflags";
    const char* muxer_opt_value  = "frag_keyframe+empty_moov+delay_moov+default_base_moof";
    unsigned char* buffer        = new unsigned char[size.first * size.second * 4];

    // prepare opengl
    unsigned int outTex;
    glGenTextures(1, &outTex);
    glActiveTexture(GL_TEXTURE0 + channels.size());
    glBindTexture(GL_TEXTURE_2D, outTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.first, size.second, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    const char* shaderSource   = R"(

    )";
    unsigned int computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &shaderSource, nullptr);
    glCompileShader(computeShader);
    GLint ret = 0;
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &ret);
    if(ret == GL_FALSE)
    {
        GLint maxLenght = 0;
        glGetShaderiv(computeShader, GL_INFO_LOG_LENGTH, &maxLenght);
        std::vector<GLchar> errorLog(maxLenght);
        glGetShaderInfoLog(computeShader, maxLenght, &maxLenght, &errorLog[0]);
        printf("%s", errorLog.data());
        glDeleteShader(computeShader);
        throw runtime_error("Failed to compile shader");
    }
    unsigned int programID = glCreateProgram();
    glAttachShader(programID, computeShader);
    glLinkProgram(programID);
    glGetProgramiv(programID, GL_LINK_STATUS, &ret);
    if(ret == GL_FALSE)
    {
        GLint maxLenght = 0;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLenght);
        std::vector<GLchar> errorLog(maxLenght);
        glGetProgramInfoLog(programID, maxLenght, &maxLenght, &errorLog[0]);
        printf("%s", errorLog.data());
        glDeleteProgram(programID);
        throw runtime_error("Failed to link shader program");
    }
    glDeleteShader(computeShader);
    glUseProgram(programID);
    glActiveTexture(GL_TEXTURE0 + channels.size());
    glBindTexture(GL_TEXTURE_2D, outTex);
    glUniform1i(channels.size(), channels.size());
    glBindImageTexture(channels.size(), outTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA);
    for(size_t i = 0; i < textures.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i].handle);
        glUniform1i(i, i);
        glBindImageTexture(i, textures[i].handle, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA);
    }

    // open output
    ///////////////////////////////////////////////////////////////
    if(avformat_alloc_output_context2(&formatCtx, NULL, NULL, path.c_str()) <= 0) throw runtime_error("Failed to alloc output context");
    ///////////////////////////////////////////////////////////////

    // prepare video encoder
    ///////////////////////////////////////////////////////////////
    AVRational outputFramerate = { framerate, 1 };
    if(!(videoStream = avformat_new_stream(formatCtx, NULL))) throw runtime_error("Failed to open a video stream");
    if(!(videoCodec = avcodec_find_encoder_by_name(video_codec))) throw runtime_error("Failed to find encoder");
    if(!(videoCodecCtx = avcodec_alloc_context3(videoCodec))) throw runtime_error("Failed to alloc context to codec");
    if(av_opt_set(videoCodecCtx->priv_data, "preset", "fast", 0) != 0) throw runtime_error("Failed to set codec option");
    if(av_opt_set(videoCodecCtx->priv_data, codec_priv_key, codec_priv_value, 0) != 0) throw runtime_error("Failed to set codec option");

    videoCodecCtx->width               = size.first;
    videoCodecCtx->height              = size.second;
    videoCodecCtx->sample_aspect_ratio = { 0, 0 };
    videoCodecCtx->pix_fmt             = videoCodec->pix_fmts[0];
    videoCodecCtx->max_b_frames        = 1;
    videoCodecCtx->gop_size            = 12;
    videoStream->time_base = videoCodecCtx->time_base = av_inv_q(outputFramerate);
    videoStream->r_frame_rate = videoCodecCtx->framerate = outputFramerate;
    if(avcodec_open2(videoCodecCtx, videoCodec, NULL) < 0) throw runtime_error("Failed to open video codec");
    if(avcodec_parameters_from_context(videoStream->codecpar, videoCodecCtx) < 0) throw runtime_error("Failed to set codec parameters from context");
    if(!(swsCtx = sws_getCachedContext(swsCtx, videoCodecCtx->width, videoCodecCtx->height, AV_PIX_FMT_RGBA,
                                       videoCodecCtx->width, videoCodecCtx->height, videoCodecCtx->pix_fmt,
                                       SWS_BILINEAR, NULL, NULL, NULL))) throw runtime_error("Failed to allocate a swscaler context");
    ///////////////////////////////////////////////////////////////
    // prepare audio encoder
    if(!(audioStream = avformat_new_stream(formatCtx, NULL))) throw runtime_error("Failed to create audio stream");
    if(!(audioCodec = avcodec_find_encoder_by_name(audio_codec))) throw runtime_error("Failed to find audio encoder");
    if(!(audioCodecCtx = avcodec_alloc_context3(audioCodec))) throw runtime_error("Failed to alloc audio context");
    constexpr int OUTPUT_CHANNELS        = 2;
    constexpr int OUTPUT_BIT_RATE        = 196000;
    constexpr int SAMPLE_RATE            = 48000;
    audioCodecCtx->channel_layout        = OUTPUT_CHANNELS;
    audioCodecCtx->sample_rate           = SAMPLE_RATE;
    audioCodecCtx->channel_layout        = av_get_default_channel_layout(OUTPUT_CHANNELS);
    audioCodecCtx->sample_fmt            = audioCodec->sample_fmts[0];
    audioCodecCtx->bit_rate              = OUTPUT_BIT_RATE;
    audioCodecCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
    audioStream->time_base = audioCodecCtx->time_base = (AVRational){ 1, SAMPLE_RATE };
    if(avcodec_open2(audioCodecCtx, audioCodec, NULL) < 0) throw runtime_error("Failed to open audio codec");
    avcodec_parameters_from_context(audioStream->codecpar, audioCodecCtx);

    ///////////////////////////////////////////////////////////////
    if(formatCtx->oformat->flags & AVFMT_GLOBALHEADER)
        formatCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    if(!(formatCtx->oformat->flags & AVFMT_NOFILE))
    {
        if(avio_open(&formatCtx->pb, path.c_str(), AVIO_FLAG_WRITE) < 0) throw runtime_error("Failed to open a file");
    }
    if(avformat_write_header(formatCtx, NULL) < 0) throw runtime_error("Failed to write header to a file");

    milliseconds currentTs(0);
    milliseconds durationInc = chrono::duration_cast<milliseconds>(
        chrono::duration<double>(1.0 / framerate));
    std::vector<frame*> frames;
    size_t nFrames = 0;
    while(true)
    {
        // get frames
        //////////////////////////////////////////////////////////////////
        size_t nbFrames = 0;
        for(; nbFrames < channels.size(); ++nbFrames)
        {
            auto f = channels[nbFrames]->next_frame_blocking(currentTs);
            if(f) frames[nbFrames] = f;
        }
        //////////////////////////////////////////////////////////////////
        // opengl
        //////////////////////////////////////////////////////////////////
        for(size_t i = 0; i < nbFrames; ++i)
        {
            texture_for_frame(i, frames[i]);
        }
        glDispatchCompute(size.first, size.second, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glActiveTexture(GL_TEXTURE0 + channels.size());
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
        //////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////
        // write frame
        AVFrame* outFrame   = av_frame_alloc();
        outFrame->pict_type = AV_PICTURE_TYPE_NONE;
        outFrame->width     = size.first;
        outFrame->height    = size.second;
        outFrame->format    = videoCodecCtx->pix_fmt;
        if(av_frame_get_buffer(outFrame, 0) != 0) throw runtime_error("Could not get buffer for frame");
        uint8_t* out[4] = { (uint8_t*)(buffer), NULL, NULL, NULL };
        int linesize[4] = { size.first * 4, 0, 0, 0 };
        sws_scale(swsCtx, out, linesize, 0, size.second, outFrame->data, outFrame->linesize);
        outFrame->pkt_duration = videoCodecCtx->time_base.den / (double)videoCodecCtx->time_base.num
                                 / videoStream->r_frame_rate.num * videoStream->r_frame_rate.den;
        outFrame->pts = outFrame->pkt_duration * (nFrames++) * videoStream->time_base.den / (videoStream->time_base.num * (videoStream->r_frame_rate.num / videoStream->r_frame_rate.den));

        int r             = avcodec_send_frame(videoCodecCtx, outFrame);
        auto outputPacket = av_packet_alloc();
        while(true)
        {
            r = avcodec_receive_packet(videoCodecCtx, outputPacket);
            if(r == AVERROR(EAGAIN) || r == AVERROR_EOF)
                break;
            else if(r < 0)
                throw runtime_error("Error occured while encoding a packet");
            outputPacket->stream_index = videoIndex;
            r                          = av_interleaved_write_frame(formatCtx, outputPacket);
            if(r != 0) throw runtime_error("Failed to write packet");
        }
        //////////////////////////////////////////////////////////////////
        currentTs += durationInc;
    }
    // flush encoder

    if(!av_write_trailer(formatCtx) != 0) throw runtime_error("Failed to write trailer to a file");

    sws_freeContext(swsCtx);
    avformat_free_context(formatCtx);
    avcodec_free_context(&videoCodecCtx);
    avcodec_free_context(&audioCodecCtx);
}
*/
bool timeline::set_paused(bool value)
{
    if(paused == value) return value;

    auto old = paused;
    paused   = value;
    if(!value)
    {
        tsChecked = chrono::high_resolution_clock::now();
    }
    for(auto& ch : channels)
    {
        ch->set_paused(value);
    }
    return old;
}
bool timeline::toggle_pause()
{
    auto old = paused;

    paused ^= 1;
    for_each(channels.begin(), channels.end(),
             [&](auto& ch) { ch->set_paused(paused); });

    if(paused == false) tsChecked = chrono::high_resolution_clock::now();

    return old;
}
void timeline::initialize_audio()
{
    auto err = Pa_Initialize();
    if(err != paNoError)
    {
        throw runtime_error("PortAudio failed while initializing with: " + string(Pa_GetErrorText(err)));
    }
}
void timeline::drop_audio()
{
    channels.clear();
    auto err = Pa_Terminate();
    if(err != paNoError)
    {
        throw runtime_error("PortAudio failed while destroying with: " + string(Pa_GetErrorText(err)));
    }
}
void timeline::rebuild()
{
    using namespace chrono;
    ts        = 0ms;
    tsChecked = high_resolution_clock::now();
}
void timeline::seek(const milliseconds& ts)
{
    using namespace chrono;
    auto realTs = max(milliseconds(0), min(ts, get_duration()));

    this->ts = realTs;

    std::for_each(
        channels.begin(), channels.end(),
        [](auto& ch) { ch->stop(); });
    std::for_each(
        channels.begin(), channels.end(),
        [realTs](auto& ch) { ch->seek(realTs); });
    std::for_each(
        channels.begin(), channels.end(),
        [](auto& ch) { ch->start(); });

    tsChecked = high_resolution_clock::now();
}

timeline::milliseconds timeline::get_duration() const
{
    if(channels.empty()) return 0ms;

    auto max = max_element(
        channels.begin(), channels.end(),
        [](const auto& lhs, const auto& rhs) {
            return lhs->get_duration() < rhs->get_duration();
        });
    return (*max)->get_duration();
}
timeline::milliseconds timeline::get_timestamp() const
{
    using namespace chrono;
    if(paused)
        return ts;
    else
        return ts + duration_cast<milliseconds>(high_resolution_clock::now() - tsChecked);
}
bool timeline::add_clip(std::size_t ch, const std::shared_ptr<video>& vid, const milliseconds& at)
{
    assert(ch < channels.size());
    stop();
    auto res = channels[ch]->add_clip(vid, at);
    start();
    return res;
}

bool timeline::add_clip(std::size_t ch, const std::shared_ptr<video>& vid, const milliseconds& at,
                        const milliseconds& startOffset, const milliseconds& endOffset)
{
    assert(ch < channels.size());
    stop();
    auto res = channels[ch]->add_clip(vid, at, startOffset, endOffset);
    start();
    return res;
}
void timeline::append_clip(std::size_t ch, const std::shared_ptr<video>& vid)
{
    assert(ch < channels.size());
    stop();
    channels[ch]->append_clip(vid);
    start();
}
void timeline::move_clip(std::size_t ch, std::size_t cl, const milliseconds& to)
{
    assert(ch < channels.size());
    stop();
    channels[ch]->move_clip(cl, to);
    start();
}
void timeline::stop()
{
    set_paused(true);
    std::for_each(
        channels.begin(), channels.end(),
        [](auto& ch) {
            ch->stop();
        });
}
void timeline::set_size(const pixel_size& size)
{
    this->size = size;
}
void timeline::start()
{
    std::for_each(
        channels.begin(), channels.end(),
        [](auto& ch) {
            ch->start();
        });
}
void timeline::texture_for_frame(std::size_t index, frame* f)
{
    auto& tex = textures[index];
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, tex.handle);
    auto fsize = f->owner->get_size();
    if(fsize != tex.size)
    {
        tex.size = fsize;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.size.width, tex.size.height,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, f->data.data());
    }
    else
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                        tex.size.width, tex.size.height, GL_RGBA,
                        GL_UNSIGNED_BYTE, f->data.data());
    }
}
void timeline::generate_texture(std::size_t index)
{
    unsigned int texture;
    glActiveTexture(GL_TEXTURE0 + index);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 size.width, size.height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, 0);
    textures.push_back({ texture, size });
}
void timeline::destroy_texture()
{
    auto tex = textures.back().handle;
    glDeleteTextures(1, &tex);
    textures.pop_back();
}
} // namespace libpgmaker
