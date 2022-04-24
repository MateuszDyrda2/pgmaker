#pragma once

#include "clip.h"
#include "pg_types.h"
#include "spsc_queue.h"

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
    std::list<clip> clips;
    std::list<clip>::iterator currentClip;
    spsc_queue<packet*, 32> packetQueue;
    spsc_queue<frame*, 32> frameQueue;
    frame* prevFrame;
    frame* nextFrame;
    std::chrono::milliseconds timestamp;
    std::chrono::milliseconds lenght;
    worker_type decodeWorker;
    worker_type videoWorker;
    std::atomic_bool finished;
    std::atomic_bool stopped;
    std::mutex videoMtx;
    std::mutex decodingMtx;
    std::condition_variable videoCvar;
    std::condition_variable decodingCvar;

  private:
    void decoding_job();
    void video_job();
    void recalculate_lenght();
};
} // namespace libpgmaker
