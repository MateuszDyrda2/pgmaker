/** @file */

#pragma once

#include "channel.h"

#include <deque>

namespace libpgmaker {
/** Class representation of a timeline containing channels. */
class timeline
{
  public:
    struct texture_t
    {
        unsigned int handle;
        pixel_size size;
    };

  public:
    static constexpr std::size_t MAX_FRAMES = 4;

    using time_point   = channel::time_point;
    using milliseconds = channel::milliseconds;
    using duration     = channel::duration;

  public:
    /** @brief Create a timeline */
    timeline();
    timeline(timeline&& other) noexcept;
    timeline& operator=(timeline&& other) noexcept;
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
     *  @return vector containing textures and number of new textures
     */
    std::pair<const std::vector<texture_t>&, std::size_t> next_frame();
    std::pair<const std::vector<texture_t>&, std::size_t> next_frame_blocking(const milliseconds& ts);
    // void export_timeline(const std::string& path, int64_t framerate);
    /** @brief Add a clip to a timeline to a specified channel at time
     * @param channelHandle handle of the channel to add the clip to
     * @param vid video to take the clip from
     * @param at timestamp the clip should start at
     * @return true if the action was a success
     */
    bool add_clip(std::size_t channelHandle, const std::shared_ptr<video>& vid, const milliseconds& at);
    /** @brief Add a clip to a timeline to a specified channel at time
     * @param channelHandle handle of the channel to add the clip to
     * @param vid video to take the clip from
     * @param at timestamp the clip should start at
     * @param startOffset offset into the video the clip should start at
     * @param endOffset offset into the end of the video the clip should end at
     * @return true if the action was a success
     */
    bool add_clip(std::size_t channelHandle, const std::shared_ptr<video>& vid, const milliseconds& at,
                  const milliseconds& startOffset, const milliseconds& endOffset);
    /** @brief Append a clip onto the end of the channels clips
     * @param channelHandle handle of the channel to add the clip to
     * @param vid video to take the clip from
     */
    void append_clip(std::size_t channelHandle, const std::shared_ptr<video>& vid);
    /** @brief Move the clip in a channel to a specified timestamp
     * @param channelHandle handle of the channel containing the clip
     * @param clipHandle handle of the clip to move
     * @param to timestamp the clip should be moved to
     */
    void move_clip(std::size_t channelHandle, std::size_t clipHandle, const milliseconds& to);
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
    auto get_paused() const { return paused; }
    /** @brief Change the resolution of the project
     * @param size new resolution
     */
    void set_size(const pixel_size& size);
    /** @brief Stop all channels playback */
    void stop();
    /** @brief Restart all channels states and playback */
    void start();

  private:
    bool paused;
    std::deque<std::unique_ptr<channel>> channels;

    milliseconds ts;
    time_point tsChecked;

    pixel_size size;
    std::vector<texture_t> textures;

  private:
    void generate_vao();
    void generate_shaders();
    void generate_texture(std::size_t index);
    void texture_for_frame(std::size_t index, frame* f);
    void destroy_texture();

    void initialize_audio();
    void drop_audio();
    void rebuild();
};
} // namespace libpgmaker
