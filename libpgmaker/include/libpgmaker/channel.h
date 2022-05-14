#pragma once

#include "clip.h"
#include "pg_types.h"
#include "spsc_queue.h"

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
    template<class T, std::size_t S>
    using spsc_queue   = spsc_queue<T, S>;
    using worker_type  = std::thread;
    using time_point   = std::chrono::high_resolution_clock::time_point;
    using duration     = std::chrono::high_resolution_clock::duration;
    using milliseconds = std::chrono::milliseconds;

    static constexpr std::size_t nbChannels = 2;
    static constexpr std::size_t nbFrames   = 1024;
    static constexpr std::size_t sampleRate = 48000;

  public:
    channel(const timeline& tl);
    ~channel();

    frame* get_frame(const duration& timestamp);
    bool add_clip(const std::shared_ptr<video>& vid, const milliseconds& at);
    void jump2(const milliseconds& ts);

    milliseconds get_lenght() const { return lenght; }
    bool set_paused(bool value);

  private:
    std::list<std::unique_ptr<clip>> clips;
    std::list<std::unique_ptr<clip>>::iterator currentClip;
    spsc_queue<packet*, 32> videoPacketQueue;
    spsc_queue<packet*, 32> audioPacketQueue;
    spsc_queue<frame*, 32> frameQueue;
    frame* prevFrame;
    frame* nextFrame;
    milliseconds lenght;
    worker_type decodeWorker;
    worker_type videoWorker;
    // worker_type audioWorker;
    std::atomic_bool stopped;
    std::atomic_bool paused;
    std::vector<float> silentBuffer;
    audio_frame lastAudioFrame;
    std::vector<float> audioBuffer;
    PaStream* audioStream;
    std::int64_t audioDiffCum;
    std::size_t audioAvgCount;

    const timeline& tl;

  private:
    void stop();
    void start();
    void init_audio();
    void drop_audio();
    void decoding_job();
    void video_job();
    void recalculate_lenght();
    static int pa_stream_callback(
        const void* input,
        void* output,
        unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData);
};
} // namespace libpgmaker
