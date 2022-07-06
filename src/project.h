#pragma once

#include <memory>
#include <string>

#include "video_manager.h"
#include <libpgmaker/effect.h>

#include <filesystem>
#include <future>
#include <libpgmaker/timeline.h>
#include <nlohmann/json.hpp>
#include <thread>

class project;
/** Class responsible for managing projects */
class project_manager
{
  public:
    /** @brief Creates a new project
     * @param path path to a project file
     * @param framerate project framerate
     * @param width width to be set for the project
     * @param height height to be set for the project
     * @return pointer to the newly created project
     */
    static project* create_project(const std::string& path, float framerate, std::uint32_t width, std::uint32_t height);
    /** @brief Loads a project from file
     * @param path path to a project file
     * @return pointer to the loaded project
     */
    static project* load_project(const std::string& path);
    /** @return Returns a currently set project */
    static project* get_current_project();

  private:
    static std::unique_ptr<project> loadedProject;
};

class project
{
  public:
    /** @brief Creates a new project with custom parameters set
     * @param path path to a project file
     * @param framerate framerate of the project
     * @param width width of the project in pixels
     * @param height height of the project in pixels
     */
    project(const std::filesystem::path& path, float framerate, std::uint32_t width, std::uint32_t height);
    /** @brief Create a project from a file
     * @param path path to a project file to load
     * @param workingDirectory working directory of the project
     * @param tmpDirectory project directory for storing temporary files
     * @param assetDirectory project directory for storing asset files
     */
    project(const std::filesystem::path& path,
            const std::filesystem::path& workingDirectory,
            const std::filesystem::path& tmpDirectory,
            const std::filesystem::path& assetDirectory);
    ~project();
    /** @brief Save the project to existing file */
    void save();
    /** @brief Save the project to a new file
     * @param path project file path
     */
    void save_as(const std::string& path);
    /** @brief Load video to the project
     * @param path path to the video
     */
    void load_video(const std::string& path);
    /** @return Get loaded videos */
    video_manager& get_videos();
    /** @return Get loaded videos */
    const video_manager& get_videos() const;
    /** @return Get timeline */
    libpgmaker::timeline& get_timeline();
    /** @return Get timeline */
    const libpgmaker::timeline& get_timeline() const;
    /** @return Get effect manager */
    auto& get_effect_mananger() { return em; }
    /** @return Get effect manager */
    const auto& get_effect_mananger() const { return em; }
    /** @brief Add effect to a clip
     * @param channelHandle handle to the channel of the clip
     * @param clipHandle handle of the clip
     * @param type of the effect to add
     */
    void add_effect(size_t channelHandle, size_t clipHandle, const std::string& type);
    /** @brief Remove all effects from the clip
     * @param channelHandle handle to the channel of the clip
     * @param clipHandle handle of the clip
     */
    void remove_effects(size_t channelHandle, size_t clipHandle);
    /** @return Get size of the project */
    const auto& get_size() const { return size; }
    /** @return get one-over size of the project */
    const auto& get_r_size() const { return rSize; }
    /** @return Get framerate of the project */
    auto get_framerate() const { return framerate; }
    /** @return Get one-over framerate of the project */
    auto get_r_framerate() const { return rFramerate; }
    /** @return Get state of the effect loading */
    auto& get_effect_complete() { return effectComplete; }

    void export2file(const std::string& path);

  private:
    video_manager videos;
    libpgmaker::timeline tl;
    libpgmaker::effect_manager em;

    std::string name;
    std::filesystem::path path;
    std::filesystem::path workingDirectory;
    std::filesystem::path tmpDirectory;
    std::filesystem::path assetDirectory;

    libpgmaker::pixel_size size;
    std::pair<float, float> rSize;
    float framerate;
    float rFramerate;

    std::future<bool> effectComplete;

  private:
    void scan_assets();
};
using json = nlohmann::json;
namespace libpgmaker {
void to_json(json& j, const std::unique_ptr<clip>& cl);
void to_json(json& j, const std::unique_ptr<channel>& ch);
void to_json(json& j, const timeline& tl);
}
void to_json(json& j, const project& pr);
