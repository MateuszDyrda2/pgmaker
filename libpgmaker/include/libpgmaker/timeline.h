#pragma once

#include "channel.h"
#include "project_settings.h"

#include <deque>

namespace libpgmaker {
class timeline
{
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
    /** @brief Move playback by specified delta.
     * Depending on the delta sometimes the returned frame will be
     * the same frame as the last one or some frames may be skipped
     * @param delta time in milliseconds since last time this function was called
     * @return pointer to the requested frame
     */
    std::shared_ptr<frame> tick_frame(const std::chrono::milliseconds& delta);

  private:
    project_settings settings;
    std::deque<std::unique_ptr<channel>> channels;
    std::chrono::milliseconds timestamp;
};
} // namespace libpgmaker
