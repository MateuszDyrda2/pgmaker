/** @file */
#pragma once

#include "clip.h"
#include "pg_types.h"
#include <readerwritercircularbuffer.h>
#include <readerwriterqueue.h>

#include <portaudio.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <list>
#include <mutex>
#include <optional>
#include <thread>

namespace libpgmaker {
class timeline;
class channel
{
  public:
    using packet_queue_t = moodycamel::ReaderWriterQueue<packet>;
    using frame_queue_t  = moodycamel::BlockingReaderWriterCircularBuffer<frame*>;
    using worker_type    = std::thread;
    using time_point     = std::chrono::high_resolution_clock::time_point;
    using duration       = std::chrono::high_resolution_clock::duration;
    using milliseconds   = std::chrono::milliseconds;

    static constexpr std::size_t NB_CHANNELS = 2;
    static constexpr std::size_t NB_FRAMES   = 1024;
    static constexpr std::size_t SAMPLE_RATE = 48000;
    static constexpr std::size_t MAX_PACKETS = 64;

  public:
    /** @brief Default constructor */
    channel() = default;
    /** @brief Creates a new channel assigning it to a timeline
     * @param tl timeline the channel is a part of
     * @param index index of the channel in the timeline
     */
    channel(const timeline* tl, std::size_t index);
    ~channel();
    /** @brief Returns a clips with a given handle
     * @param clipHandle handle of the clip in the channel
     * @return pointer to the clip
     */
    clip* get_clip(std::size_t clipHandle);
    /** @brief Returns a clips with a given handle
     * @param clipHandle handle of the clip in the channel
     * @return pointer to the clip
     */
    const clip* get_clip(std::size_t clipHandle) const;
    /** @brief Returns a clips with a given handle
     * @param clipHandle handle of the clip in the channel
     * @return pointer to the clip
     */
    clip* operator[](std::size_t clipHandle);
    /** @brief Returns a clips with a given handle
     * @param clipHandle handle of the clip in the channel
     * @return pointer to the clip
     */
    const clip* operator[](std::size_t clipHandle) const;
    /** @return deque of the clips in the channel.
     * The clips are in the order they are played */
    auto& get_clips() { return clips; }
    /** @return deque of the clips in the channel.
     * The clips are in the order they are played */
    const auto& get_clips() const { return clips; }
    /** @brief Returns a duration of the channel.
     * The duration is the moment the last clip of the channel ends.
     * @return duration in millseconds
     */
    const auto& get_duration() const { return lenght; }
    /** @return An index of the channel in the timeline */
    auto get_index() const { return index; }
    /** @return Id of the channel */
    auto get_id() const { return channelId; }
    /** @return Is the channel muted */
    auto get_muted() { return muted; }

    /** @brief Set the audio mute value
     * @param value should the audio be muted
     */
    void set_muted(bool value);

  private:
    friend class timeline;

    /** @brief Attach a new clip to the channel
     * @param vid the video the clip is taken from
     * @param at what time should the clip start playing
     * @return true if the clip was succesfully attached
     */
    bool add_clip(const std::shared_ptr<video>& vid, const milliseconds& at);
    /** @brief Attach a new clip to the channel
     * @param vid the video the clip is taken from
     * @param at what time should the clip start playing
     * @param startOffset offset into the clip from the start
     * @param endOffset offset into the clip from the end
     * @return true if the clip was succesfully attached
     */
    bool add_clip(const std::shared_ptr<video>& vid, const milliseconds& at,
                  const milliseconds& startOffset, const milliseconds& endOffset);
    /** @brief Appends a clip after the last clip of the channel
     * @param vid the video the clip is taken from
     * @return true if the clip was succesfully attached
     */
    bool append_clip(const std::shared_ptr<video>& vid);
    /** @brief Tries to move a clip with a given index
     * to the timestamp specified
     * @param handle index of the clip to be moved
     * @param to where the clip should be moved to
     * @note if the clip cannot be moved, the method will return early
     */
    void move_clip(std::size_t handle, const milliseconds& to);
    /** @brief Tries to change current ts of the video
     * @param ts the place the playback should resume from
     */
    void seek(const milliseconds& ts);
    /** @brief Get a frame with a specified timestamp.
     * Returns a last frame with a timestamp <= than ts passed.
     * @param timestamp requested timestamp
     * @return a pointer to the current frame
     */
    frame* next_frame(const duration& timestamp);
    frame* next_frame_blocking(const duration& timestamp);
    /** @brief Change whether a channel is paused
     * @param value value to set
     * @return previous value
     */
    bool set_paused(bool value);
    /*
    frame* next_frame_blocking(const duration& timestamp);
    bool next_audio_frame(const duration& timestamp, AVFrame* frame);
    */

  private:
    std::deque<std::unique_ptr<clip>> clips;
    std::deque<std::unique_ptr<clip>>::iterator currentClip;
    packet_queue_t videoPacketQueue;
    packet_queue_t audioPacketQueue;
    frame_queue_t frameQueue;
    frame* prevFrame;
    frame* nextFrame;
    milliseconds lenght;
    worker_type decodeWorker;
    worker_type videoWorker;
    std::atomic_bool stopped;
    std::atomic_bool paused;
    std::vector<float> silentBuffer;
    std::vector<float> audioBuffer;
    PaStream* audioStream;
    bool muted;

    const timeline* tl;
    std::size_t index;
    std::size_t channelId;

  private:
    void stop();
    void start();
    void init_audio();
    void drop_audio();
    void decoding_job();
    void video_job();
    void reorder_clips();
    void recalculate_lenght();
    int audio_stream_callback(
        void* output,
        unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo);
};
} // namespace libpgmaker
