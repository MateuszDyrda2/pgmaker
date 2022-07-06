/** @file */

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
    /** @brief Default constructor */
    clip() = default;
    /** @brief Creates a clip with a video that has a choosen start time
     * @param vid video the clip is created from
     * @param startsAt the time the clips starts at in millseconds
     */
    clip(const std::shared_ptr<video>& vid, milliseconds startsAt);
    /** @brief Creates a clip with a video that has a choosen start time
     * @param vid video the clip is created from
     * @param startsAt the time the clips starts at in millseconds
     * @param startOffset offset into the video the clip starts at
     * @param endOffset offset into the video the clip ends at
     */
    clip(const std::shared_ptr<video>& vid, const milliseconds& startsAt,
         const milliseconds& startOffset, const milliseconds& endOffset);
    ~clip();
    /** @brief Change the video the clip represents
     * @param newVideo new video to set
     */
    void change_video(const std::shared_ptr<video>& newVideo);
    /** @brief Set the video with an effect applied to it
     * @param newVideo video with an effect
     */
    void add_effect_video(const std::shared_ptr<video>& newVideo);
    /** @brief Remove all effects from the video
     * @param originalVideo video without effects
     */
    void restore_video(const std::shared_ptr<video>& orignalVideo);
    /** @return video metadata */
    const auto& get_info() const { return info; }
    /** @return timebase of the video */
    auto get_vtimebase() const { return vidTimebase; }
    /** @return timebase of the audio */
    auto get_atimebase() const { return audioTimebase; }
    /** @return size of the clip */
    auto get_size() const { return size; }
    /** @return duration of the clip in milliseconds */
    auto get_duration() const { return info.duration - startOffset - endOffset; }
    /** @return id of the clip */
    auto get_id() const { return clipId; }
    /** @return name of the video */
    const auto& get_name() const { return name; }
    /** @return path of the video */
    const auto& get_path() const { return path; }
    /** @return does the video have any effects applied to it */
    auto has_effect() const { return hasEffect; }
    /** @return information about the clip */
    auto get_clip_info() const { return clip_info{ name, info.path, get_duration().count() * 0.001, size.width, size.height }; }
    /** @return time the clip starts at on timeline in milliseconds */
    const auto& get_starts_at() const { return startsAt; }
    /** @return time the clip ends at on timeline in milliseconds */
    auto get_ends_at() const { return startsAt + get_duration(); }
    /** @return offset into the video the clip has */
    const auto& get_start_offset() const { return startOffset; }
    /** @return offset into the video from the end the clip has */
    const auto& get_end_offset() const { return endOffset; }
    /** @brief Returns whether the clip is played during the ts
     * @param ts timestamp to check
     * @return true if the ts is contained withing the clip
     */
    bool contains(const milliseconds& ts) const;
    /** @brief Reads another packet from the stream
     * @param[out] pPacket packet read
     * @return false if the stream ended
     */
    bool get_packet(packet& pPacket);
    /** @brief Decodes a packet into an output frame
     * @param pPacket packet to decode
     * @param[in,out] frame frame decoded
     * @return true if the frame was succesfully decoded
     */
    bool get_frame(packet& pPacket, AVFrame** frame);
    /** @brief Decodes a packet into an audio frame
     * @param pPacket packet to decode
     * @param[in, out] b buffer to place decoded data into
     * @return size of the data decoded
     */
    std::size_t get_audio_frame(packet* pPacket, std::vector<float>& b);
    /** @brief Rescales frame and converts to RGBA
     * @param iFrame input frame
     * @param[in, out] oFrame output frame
     */
    void convert_frame(AVFrame* iFrame, frame** oFrame);
    /** @brief Change when the clip starts at
     * @param startsAt time on the timeline the clip should start at
     */
    void set_starts_at(const milliseconds& startsAt);
    /** @brief Change offset into the video
     * @param startOffset new offset
     */
    void set_start_offset(const milliseconds& startOffset);
    /** @brief Change offset into the video by
     * @param by value to change the offset by
     */
    void change_start_offset(const milliseconds& by);
    /** @brief Change offset from the end of the video
     * @param endOffset new offset
     */
    void set_end_offset(const milliseconds& endOffset);
    /** @brief Change offset from the end of the video by
     * @param by value to change the offset by
     */
    void change_end_offset(const milliseconds& by);
    /** @brief Seek into the video
     * @param ts timestamp to seek to
     * @return true if the seek was succesfull
     */
    bool seek(const milliseconds& ts);
    /** @brief Flush the buffers */
    void flush();
    /** @brief Reset the state of the decoder */
    void reset();
    /** @brief Convert the audio pts into milliseconds
     * @param pts pts to convert
     * @return duration in millseconds
     */
    milliseconds audio_convert_pts(std::int64_t pts) const;
    /** @brief Convert duration in millseconds into audio pts
     * @param pts duration to convert
     * @return audio pts
     */
    std::int64_t audio_reconvert_pts(const milliseconds& pts) const;
    /** @brief Convert the video pts into milliseconds
     * @param pts pts to convert
     * @return duration in millseconds
     */
    milliseconds video_convert_pts(std::int64_t pts) const;
    /** @brief Convert duration in millseconds into video pts
     * @param pts duration to convert
     * @return video pts
     */
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

    pixel_size size;
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
