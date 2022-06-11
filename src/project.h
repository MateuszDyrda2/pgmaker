#pragma once

#include <memory>
#include <string>

#include "video_manager.h"
#include <libpgmaker/effect.h>

#include <filesystem>
#include <libpgmaker/timeline.h>
#include <nlohmann/json.hpp>

class project;
class project_manager
{
  public:
    static project* create_project(const std::string& path);
    static project* load_project(const std::string& path);
    static project* get_current_project();

  private:
    static std::unique_ptr<project> loadedProject;
};

class project
{
  public:
    project(const std::filesystem::path& path);
    project(const std::filesystem::path& path,
            const std::filesystem::path& workingDirectory,
            const std::filesystem::path& tmpDirectory,
            const std::filesystem::path& assetDirectory);
    ~project();

    void save();
    void save_as(const std::string& path);
    void load_video(const std::string& path);

    video_manager& get_videos();
    const video_manager& get_videos() const;

    libpgmaker::timeline& get_timeline();
    const libpgmaker::timeline& get_timeline() const;

    void add_effect(size_t channel, size_t clip, libpgmaker::effect::effect_type type);

    const auto& get_size() const { return size; }

  private:
    video_manager videos;
    libpgmaker::timeline tl;

    std::string name;
    std::filesystem::path path;
    std::filesystem::path workingDirectory;
    std::filesystem::path tmpDirectory;
    std::filesystem::path assetDirectory;

    std::pair<std::uint32_t, std::uint32_t> size;
    std::size_t framerate;

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
