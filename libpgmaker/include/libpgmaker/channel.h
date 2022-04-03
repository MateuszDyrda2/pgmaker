#pragma once

#include "video.h"

#include <boost/lockfree/spsc_queue.hpp>

#include <chrono>
#include <list>
#include <optional>
#include <thread>

namespace libpgmaker {
struct frame
{
    std::uint8_t* data;
    std::uint32_t width, height;
};
struct clip
{
    std::weak_ptr<video> vid;
    std::chrono::milliseconds startOffset;
    std::chrono::milliseconds endOffset;
};
class channel
{
    template<class T, std::size_t S>
    using spsc_queue  = boost::lockfree::spsc_queue<T, boost::lockfree::capacity<S>>;
    using worker_type = std::thread;

  public:
    channel(/* args */);
    ~channel();

    std::optional<frame> get_frame(); // TODO: set playback speed and skip frame / return the same frame

    std::chrono::milliseconds get_lenght() const { return lenght; }

  private:
    std::list<clip> clips;
    std::list<clip>::iterator currentClip;
    spsc_queue<AVPacket*, 32> packetQueue;
    spsc_queue<frame, 32> frameQueue;
    worker_type decodeWorker;
    worker_type videoWorker;
    std::chrono::milliseconds lenght;

  private:
    void decoding_job();
    void video_job();
};
} // namespace libpgmaker
