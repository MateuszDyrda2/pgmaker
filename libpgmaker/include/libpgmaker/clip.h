#pragma once

#include "pg_types.h"
#include "video.h"

#include <chrono>
#include <memory>

#include <vector>

namespace libpgmaker {
struct clip_info
{
    std::string name;
    std::string path;
    double duration;
    unsigned int width, height;
};
class clip
{
  public:
    using milliseconds = std::chrono::milliseconds;

  public:
    clip() = default;
    clip(const std::shared_ptr<video>& vid, milliseconds startsAt);
    clip(const std::shared_ptr<video>& vid, const milliseconds& startsAt,
         const milliseconds& startOffset, const milliseconds& endOffset);
    ~clip();
    void change_video(const std::shared_ptr<video>& newVideo);
    void add_effect_video(const std::shared_ptr<video>& newVideo);
    void restore_video(const std::shared_ptr<video>& orignalVideo);

    const auto& get_info() const { return info; }
    auto get_vtimebase() const { return vidTimebase; }
    auto get_atimebase() const { return audioTimebase; }
    auto get_size() const { return size; }
    auto get_duration() const { return info.duration - startOffset - endOffset; }
    auto get_id() const { return clipId; }
    const auto& get_name() const { return name; }
    const auto& get_path() const { return path; }
    auto has_effect() const { return hasEffect; }
    auto get_clip_info() const { return clip_info{ name, info.path, get_duration().count() * 0.001, size.first, size.second }; }

    const auto& get_starts_at() const { return startsAt; }
    auto get_ends_at() const { return startsAt + get_duration(); }
    const auto& get_start_offset() const { return startOffset; }
    const auto& get_end_offset() const { return endOffset; }

    bool contains(const milliseconds& ts) const;

    bool get_packet(packet& pPacket);
    bool get_frame(packet& pPacket, AVFrame** frame);
    std::size_t get_audio_frame(packet* pPacket, std::vector<float>& b);
    void convert_frame(AVFrame* iFrame, frame** oFrame);

    void set_starts_at(const milliseconds& startsAt);
    void set_start_offset(const milliseconds& startOffset);
    void change_start_offset(const milliseconds& by);
    void set_end_offset(const milliseconds& endOffset);
    void change_end_offset(const milliseconds& by);

    bool seek(const milliseconds& ts);
    void flush();
    void reset();

    milliseconds audio_convert_pts(std::int64_t pts) const;
    std::int64_t audio_reconvert_pts(const milliseconds& pts) const;
    milliseconds video_convert_pts(std::int64_t pts) const;
    std::int64_t video_reconvert_pts(const milliseconds& pts) const;

  private:
    video_info info;
    std::string name;
    std::string path;
    bool hasEffect;
    std::uint64_t clipId;
    milliseconds startOffset;
    milliseconds endOffset;
    milliseconds startsAt;

    std::pair<uint32_t, uint32_t> size;
    AVFormatContext* pFormatCtx;
    AVCodecContext* pVideoCodecCtx;
    AVCodecContext* pAudioCodecCtx;
    int vsIndex, asIndex;
    SwsContext* swsCtx;
    SwrContext* swrCtx;
    AVRational vidTimebase;
    AVRational audioTimebase;
    std::uint32_t sampleRate;
    std::uint32_t nbChannels;
    std::int64_t vidCurrentStreamPos;
    std::int64_t vidCurrentTs;

    AVFrame* audioFrame;

  private:
    friend class channel;
    void open_input(const std::string& path);
    void close_input();
    bool open_codec(AVCodecParameters* codecParams, AVCodecContext** ctx);
    void fill_buffer();
    void seek_start();
    bool seek_impl(const milliseconds& localTs);
};
}
