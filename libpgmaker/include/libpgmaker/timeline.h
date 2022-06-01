#pragma once

#include "channel.h"
#include "project_settings.h"

#include <deque>

namespace libpgmaker {
/** Class representation of a timeline containing channels. */
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
    /** @brief Removes a channel from the timeline
     * @param ch pointer to the channel to be removed
     */
    void remove_channel(channel* ch);
    /** @return Returns a channel with index specified */
    channel* get_channel(std::size_t index);
    /** @return Returns a channel with index specified */
    channel* operator[](std::size_t index);
    /** @return Returns a channel with index specified */
    const channel* get_channel(std::size_t index) const;
    /** @return Returns a channel with index specified */
    const channel* operator[](std::size_t index) const;
    /** @return Returns all channels */
    const std::deque<std::unique_ptr<channel>>& get_channels() const;
    /** @return Returns all channels */
    std::deque<std::unique_ptr<channel>>& get_channels();
    /** @brief Move playback forward and get the current frame.
     * @return pointer to the requested frame
     */
    frame* next_frame();
    bool add_clip(std::size_t ch, const std::shared_ptr<video>& vid, const milliseconds& at);
    void append_clip(std::size_t ch, const std::shared_ptr<video>& vid);
    void move_clip(std::size_t ch, std::size_t cl, const milliseconds& to);
    /** @brief Set the channel to be paused / unpaused
     * @param value should channel be paused
     */
    bool set_paused(bool value);
    /** @brief Toggle the channel paused.
     * If channel was paused it will be unpaused
     * and if it was playing it will be paused
     * @return previous value of paused
     */
    bool toggle_pause();
    /** @brief Jumps to the specified moment in the timeline
     * @param ts timestamp to be set
     */
    void seek(const milliseconds& ts);
    /** @return Current timestamp */
    milliseconds get_timestamp() const;
    /** @return Duration of the whole timeline */
    milliseconds get_duration() const;
    /** @return Whether the timeline is paused */
    bool get_paused() const { return paused; }

  private:
    bool paused;
    project_settings settings;
    std::deque<std::unique_ptr<channel>> channels;

    milliseconds ts;
    time_point tsChecked;

  private:
    void initialize_audio();
    void drop_audio();
    void rebuild();

    void stop();
    void start();
};
} // namespace libpgmaker
