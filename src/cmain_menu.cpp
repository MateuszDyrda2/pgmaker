#include "cmain_menu.h"

#include <nfd.h>

#include "serializer.h"

#include <fstream>
#include <iostream>

using namespace libpgmaker;
cmain_menu::cmain_menu(std::vector<std::shared_ptr<libpgmaker::video>>& videos, timeline& tl):
    videos(videos), tl(tl)
{
}
cmain_menu::~cmain_menu()
{
}
void cmain_menu::draw()
{
    if(ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("OpenVideo"))
            {
                nfdchar_t* outPath;
                auto result = NFD_OpenDialog(&outPath, NULL, 0, NULL);
                if(result == NFD_OKAY)
                {
                    printf("%s\n", outPath);
                    auto vid = video_reader::load_file(outPath)
                                   .load_metadata()
                                   .load_thumbnail()
                                   .get();

                    videos.push_back(std::move(vid));
                    NFD_FreePath(outPath);
                }
            };
            if(ImGui::MenuItem("Save project"))
            {
                nfdchar_t* outPath;
                auto result = NFD_SaveDialog(&outPath, NULL, 0, NULL, "project.pgproj");
                if(result == NFD_OKAY)
                {
                    serializer s{ .videos = videos, .tl = tl };
                    json j = s;
                    std::ofstream file(outPath);
                    file << std::setw(4) << j;
                    file.close();
                    NFD_FreePath(outPath);
                }
            }
            if(ImGui::MenuItem("Load project"))
            {
                nfdchar_t* outPath;
                nfdnfilteritem_t a = { "PGMaker projects", "pgproj" };
                auto result        = NFD_OpenDialog(&outPath, &a, 1, 0);
                if(result == NFD_OKAY)
                {
                    std::ifstream file(outPath);
                    json j;
                    file >> j;
                    auto tljs = j.at("timeline");
                    auto vdjs = j.at("videos");
                    tl        = timeline(project_settings());
                    videos    = std::vector<std::shared_ptr<video>>();
                    for(auto& vjs : vdjs.items())
                    {
                        auto vval        = vjs.value();
                        std::string path = vval.at("path").get<std::string>();
                        auto vid         = video_reader::load_file(path)
                                       .load_metadata()
                                       .load_thumbnail()
                                       .get();
                        videos.emplace_back(std::move(vid));
                    }
                    for(auto& chjs : tljs.items())
                    {
                        auto ch = tl.add_channel();
                        for(auto& cljs : chjs.value().items())
                        {
                            auto clval          = cljs.value();
                            int64_t endOffset   = clval.at("endOffset").get<int64_t>();
                            int64_t startOffset = clval.at("startOffset").get<int64_t>();
                            int64_t startsAt    = clval.at("startsAt").get<int64_t>();
                            std::string file    = clval.at("file").get<std::string>();

                            auto res = std::find_if(
                                videos.begin(), videos.end(),
                                [&](auto& vid) {
                                    return vid->get_info().name == file;
                                });
                            if(res == videos.end())
                            {
                                throw std::runtime_error("Video file not found: " + file);
                            }
                            tl.add_clip(0, *res,
                                        std::chrono::milliseconds(startsAt));
                        }
                    }

                    //                    auto d = j.get<deserializer>();
                    //                    tl     = std::move(d.tl);
                    //                    videos = std::move(d.videos);
                    //                    for(auto& ch : tl.get_channels())
                    //                    {
                    //                        ch->set_timeline(&tl);
                    //                        for(auto& cl : ch->get_clips())
                    //                        {
                    //                            auto res = std::find_if(
                    //                                videos.begin(), videos.end(),
                    //                                [&](auto& vid) {
                    //                                    return vid->get_info().name == cl->get_name();
                    //                                });
                    //                            if(res == videos.end())
                    //                            {
                    //                                throw std::runtime_error("Cannot find video in project");
                    //                            }
                    //                            cl->assign_video(*res);
                    //                        }
                    //                    }

                    NFD_FreePath(outPath);
                }
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Edit"))
        {
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}