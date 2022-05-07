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
class channel
{
    template<class T, std::size_t S>
    using spsc_queue  = spsc_queue<T, S>;
    using worker_type = std::thread;

  public:
    channel(/* args */);
    ~channel();

    frame* get_frame(const std::chrono::milliseconds& delta);
    bool add_clip(const std::shared_ptr<video>& vid, const std::chrono::milliseconds& at);
    void rebuild();
    void jump2(const std::chrono::milliseconds& ts);

    std::chrono::milliseconds get_lenght() const { return lenght; }

  private:
    std::list<std::unique_ptr<clip>> clips;
    std::list<std::unique_ptr<clip>>::iterator currentClip;
    spsc_queue<packet*, 32> videoPacketQueue;
    spsc_queue<packet*, 32> audioPacketQueue;
    spsc_queue<frame*, 32> frameQueue;
    frame* prevFrame;
    frame* nextFrame;
    std::chrono::milliseconds timestamp;
    std::chrono::milliseconds lenght;
    worker_type decodeWorker;
    worker_type videoWorker;
    worker_type audioWorker;
    std::atomic_bool stopped;

    PaStream* audioStream;

  private:
    void init_audio();
    void drop_audio();
    void decoding_job();
    void video_job();
    void audio_decode_frame();
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
