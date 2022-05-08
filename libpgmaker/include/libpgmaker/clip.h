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
    using milliseconds = std::chrono::milliseconds;

  public:
    clip(const std::shared_ptr<video>& vid, milliseconds startsAt);
    ~clip();

    const milliseconds& get_starts_at() const { return startsAt; }
    const milliseconds& get_start_offset() const { return startOffset; }
    const milliseconds& get_end_offset() const { return endOffset; }
    AVRational get_timebase() const { return timebase; }
    std::uint32_t get_width() const { return width; }
    std::uint32_t get_height() const { return height; }
    milliseconds get_duration() const { return vid->get_info().duration - startOffset - endOffset; }
    bool contains(const milliseconds& ts) const;

    void move_to(const milliseconds& startsAt);
    bool seek(const milliseconds& ts);
    void change_start_offset(const milliseconds& startOffset);
    void change_end_offset(const milliseconds& endOffset);
    void reset();
    bool get_packet(AVPacket** pPacket);
    bool get_frame(AVPacket* pPacket, AVFrame** frame);
    milliseconds get_audio_frame(AVPacket* pPacket, std::vector<float>& buff);
    void convert_frame(AVFrame* iFrame, frame** oFrame);

  private:
    std::shared_ptr<video> vid;
    milliseconds startOffset;
    milliseconds endOffset;
    milliseconds startsAt;

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
