#pragma once

#include "channel.h"
#include "project_settings.h"

#include <deque>

namespace libpgmaker {
class timeline
{
  public:
    using time_point   = channel::time_point;
    using milliseconds = channel::milliseconds;
    using duration     = channel::duration;

  public:
    /** @brief Create a timeline specifying the settings.
     * The settings can be changed at any moment
     * @param settings project settings
     */
    timeline(const project_settings& settings);
    ~timeline();
    /** @brief Add a channel to the timeline
     * @return a pointer to the newly added channel
     */
    channel* add_channel();
    /** @brief Removes a channel from the timeline
     * @param index index of the channel to be removed
     */
    void remove_channel(std::size_t index);
    /** @return Returns a channel with index specified */
    channel* get_channel(std::size_t index);
    channel* operator[](std::size_t index);
    const channel* get_channel(std::size_t index) const;
    const channel* operator[](std::size_t index) const;
    const std::deque<std::unique_ptr<channel>>& get_channels() const;
    std::deque<std::unique_ptr<channel>>& get_channels();
    /** @brief Move playback by specified delta.
     * Depending on the delta sometimes the returned frame will be
     * the same frame as the last one or some frames may be skipped
     * @param delta time in milliseconds since last time this function was called
     * @return pointer to the requested frame
     */
    frame* get_frame();
    bool set_paused(bool value);
    bool is_paused() { return paused; }
    void jump2(const milliseconds& ts);

    milliseconds get_timestamp() const;

  private:
    bool paused;
    time_point start;
    time_point pauseStarted;
    duration pausedOffset;
    duration startOffset;
    project_settings settings;
    std::deque<std::unique_ptr<channel>> channels;

    milliseconds ts;
    time_point tsChecked;

  private:
    void initialize_audio();
    void drop_audio();
    void rebuild();
};
} // namespace libpgmaker
