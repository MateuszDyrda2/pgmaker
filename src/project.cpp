#include "project.h"

#include <fstream>
#include <iostream>
#include <libpgmaker/video_reader.h>

using namespace libpgmaker;
std::unique_ptr<project> project_manager::loadedProject;
project* project_manager::create_project(const std::string& path)
{
    // TODO: MEMORY LEAK!!!!
    loadedProject = std::make_unique<project>(path);
    return loadedProject.get();
}
project* project_manager::load_project(const std::string& path)
{
    std::filesystem::path realPath   = path;
    std::filesystem::path parentPath = realPath.parent_path();
    loadedProject                    = std::make_unique<project>(realPath, parentPath,
                                              parentPath / "tmp", parentPath / "assets");
    return loadedProject.get();
}
project* project_manager::get_current_project()
{
    return loadedProject.get();
}
project::project(const std::filesystem::path& path,
                 const std::filesystem::path& workingDirectory,
                 const std::filesystem::path& tmpDirectory,
                 const std::filesystem::path& assetDirectory):
    videos(),
    tl(),
    path(path),
    workingDirectory(workingDirectory),
    tmpDirectory(tmpDirectory), assetDirectory(assetDirectory)
{
    if(!std::filesystem::exists(path) || !std::filesystem::exists(workingDirectory)
       || !std::filesystem::exists(tmpDirectory) || !std::filesystem::exists(assetDirectory))
    {
        throw std::runtime_error("Project file corrupted");
    }
    name = path.stem().string();
    for(const auto& entry : std::filesystem::directory_iterator{ assetDirectory })
    {
        if(entry.is_regular_file())
        {
            auto vid = video_reader::load_file(entry.path().string())
                           .load_metadata()
                           .load_thumbnail()
                           .get();
            videos[vid->get_info().name] = std::move(vid);
        }
    }
    std::ifstream file(path);
    json j;
    file >> j;
    file.close();
    auto tljs = j.at("timeline");
    for(auto& chjs : tljs.items())
    {
        auto ch    = tl.add_channel();
        auto chann = chjs.value().at("channel");
        for(auto& cljs : chann.items())
        {
            auto clval       = cljs.value();
            auto endOffset   = clval.at("endOffset").get<int64_t>();
            auto startOffset = clval.at("startOffset").get<int64_t>();
            auto startsAt    = clval.at("startsAt").get<int64_t>();
            auto vidname     = clval.at("file").get<std::string>();

            if(auto vl = videos.find(vidname); vl == videos.end())
            {
                throw std::runtime_error("Video file not found: " + vidname);
            }
            else
            {
                tl.add_clip(
                    ch->get_index(),
                    vl->second,
                    std::chrono::milliseconds(startsAt),
                    std::chrono::milliseconds(startOffset),
                    std::chrono::milliseconds(endOffset));
            }
        }
    }
    size = { j.at("width").get<uint32_t>(), j.at("height").get<uint32_t>() };
    tl.set_size(size);
}
project::project(const std::filesystem::path& path):
    videos(),
    tl(),
    path(path)
{
    workingDirectory = path.parent_path();
    tmpDirectory     = workingDirectory / "tmp";
    assetDirectory   = workingDirectory / "assets";
    std::filesystem::create_directory(tmpDirectory);
    std::filesystem::create_directory(assetDirectory);
    name = path.stem().string();
    size = { 1920, 1080 };
}
project::~project()
{
}
void project::save()
{
    json j = *this;
    std::ofstream file(path);
    file << std::setw(4) << j;
    file.close();
}
void project::save_as(const std::string& path)
{
    this->path = path;
    json j     = *this;
    std::ofstream file(path);
    file << std::setw(4) << j;
    file.close();
}
void project::load_video(const std::string& path)
{
    namespace fs = std::filesystem;
    auto name    = fs::path(path).filename();
    auto dir     = assetDirectory / name;
    fs::copy_file(path, dir, fs::copy_options::update_existing);
    std::shared_ptr<video> vid;
    try
    {
        vid = video_reader::load_file(dir.string())
                  .load_metadata()
                  .load_thumbnail()
                  .get();
    }
    catch(const std::runtime_error& err)
    {
        std::cerr << err.what();
        return;
    }
    videos[vid->get_info().name] = std::move(vid);
}
video_manager& project::get_videos()
{
    return videos;
}
const video_manager& project::get_videos() const
{
    return videos;
}
libpgmaker::timeline& project::get_timeline()
{
    return tl;
}
const libpgmaker::timeline& project::get_timeline() const
{
    return tl;
}
void project::scan_assets()
{
    namespace fs = std::filesystem;
}

namespace libpgmaker {
void to_json(json& j, const std::unique_ptr<clip>& cl)
{
    j = json{ { "file", cl->get_name() },
              { "startsAt", cl->get_starts_at().count() },
              { "startOffset", cl->get_start_offset().count() },
              { "endOffset", cl->get_end_offset().count() } };
}
void to_json(json& j, const std::unique_ptr<channel>& ch)
{
    j = { { "channel", ch->get_clips() } };
}
void to_json(json& j, const timeline& tl)
{
    j = tl.get_channels();
}
}
void to_json(json& j, const project& pr)
{
    j = { { "timeline", pr.get_timeline() },
          { "width", pr.get_size().first },
          { "height", pr.get_size().second } };
}