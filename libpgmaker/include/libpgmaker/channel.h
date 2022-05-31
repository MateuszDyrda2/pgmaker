#pragma once

#include "clip.h"
#include "pg_types.h"
//#include "spsc_queue.h"
//#include "tsqueue.h"
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
    // using spsc_queue   = spsc_queue<T, S>;
    //  using spsc_queue   = tsqueue<T, S>;
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
    channel(const timeline& tl);
    ~channel();

    bool add_clip(const std::shared_ptr<video>& vid, const milliseconds& at);
    bool append_clip(const std::shared_ptr<video>& vid);
    clip* get_clip(std::size_t index);
    const clip* get_clip(std::size_t index) const;
    clip* operator[](std::size_t index);
    const clip* operator[](std::size_t index) const;
    std::list<std::unique_ptr<clip>>& get_clips();
    const std::list<std::unique_ptr<clip>>& get_clips() const;
    milliseconds get_duration() const { return lenght; }

    void jump2(const milliseconds& ts);
    frame* next_frame(const duration& timestamp);

    bool set_paused(bool value);

  private:
    std::list<std::unique_ptr<clip>> clips;
    std::list<std::unique_ptr<clip>>::iterator currentClip;
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

    const timeline& tl;

  private:
    void stop();
    void start();
    void init_audio();
    void drop_audio();
    void decoding_job();
    void video_job();
    void recalculate_lenght();
    int audio_stream_callback(
        void* output,
        unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo);
};
} // namespace libpgmaker
