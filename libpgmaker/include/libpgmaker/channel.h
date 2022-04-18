#pragma once

#include "pg_types.h"
#include "video.h"

#include <boost/lockfree/spsc_queue.hpp>

#include <atomic>
#include <chrono>
#include <deque>
#include <list>
#include <optional>
#include <thread>

namespace libpgmaker {
struct clip
{
    std::shared_ptr<video> vid;
    std::chrono::milliseconds startOffset;
    std::chrono::milliseconds endOffset;
    std::chrono::milliseconds startsAfter;

    clip(const std::shared_ptr<video>& vid, const std::chrono::milliseconds& startsAfter):
        vid(vid),
        startOffset{ 0 }, endOffset{ 0 }, startsAfter(startsAfter)
    { }
};
class channel
{
    template<class T, std::size_t S>
    using spsc_queue  = boost::lockfree::spsc_queue<T, boost::lockfree::capacity<S>>;
    using worker_type = std::thread;

  public:
    channel(/* args */);
    ~channel();

    frame* get_frame(const std::chrono::milliseconds& delta); // TODO: set playback speed and skip frame / return the same frame
    bool add_clip(const std::shared_ptr<video>& vid, const std::chrono::milliseconds& at);

    std::chrono::milliseconds get_lenght() const { return lenght; }

  private:
    std::list<clip> clips;
    std::list<clip>::iterator currentClip;
    spsc_queue<AVPacket*, 32> packetQueue;
    spsc_queue<frame*, 32> frameQueue;
    frame* previousFrame;
    frame* nextFrame;
    std::chrono::milliseconds timestamp;
    worker_type decodeWorker;
    worker_type videoWorker;
    std::chrono::milliseconds lenght;
    std::atomic_bool finished;

  private:
    void decoding_job();
    void video_job();
};
} // namespace libpgmaker
