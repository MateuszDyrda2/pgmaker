#pragma once

#include "pg_types.h"
#include "video.h"

#include <chrono>
#include <memory>

#include <vector>

namespace libpgmaker {
class clip
{
  public:
    clip(const std::shared_ptr<video>& vid, std::chrono::milliseconds startsAt);
    ~clip();

    const std::chrono::milliseconds& get_starts_at() const { return startsAt; }
    const std::chrono::milliseconds& get_start_offset() const { return startOffset; }
    const std::chrono::milliseconds& get_end_offset() const { return endOffset; }
    AVRational get_timebase() const { return timebase; }
    std::uint32_t get_width() const { return width; }
    std::uint32_t get_height() const { return height; }
    std::chrono::milliseconds get_duration() const { return vid->get_info().duration - startOffset - endOffset; }

    void move_to(const std::chrono::milliseconds& startsAt);
    void change_start_offset(const std::chrono::milliseconds& startOffset);
    void change_end_offset(const std::chrono::milliseconds& endOffset);
    void reset();
    bool get_packet(AVPacket** pPacket);
    bool get_frame(AVPacket* pPacket, AVFrame** frame);
    int get_audio_frame(AVPacket* pPacket, std::vector<float>& buff);
    void convert_frame(AVFrame* iFrame, frame** oFrame);
    void convert_audio_frame(AVFrame* iFrame, audio_frame** oFrame);

  private:
    std::shared_ptr<video> vid;
    std::chrono::milliseconds startOffset;
    std::chrono::milliseconds endOffset;
    std::chrono::milliseconds startsAt;

    std::uint32_t width, height;
    AVFormatContext* pFormatCtx;
    AVCodecContext* pVideoCodecCtx;
    AVCodecContext* pAudioCodecCtx;
    int vsIndex, asIndex;
    SwsContext* swsCtx;
    SwrContext* swrCtx;
    AVRational timebase;
    std::uint32_t sampleRate;
    std::uint32_t nbChannels;

  private:
    friend class channel;
    void open_input(const std::string& path);
    bool open_codec(AVCodecParameters* codecParams, AVCodecContext** ctx);
    void fill_buffer();
    void seek_start();
};
}
