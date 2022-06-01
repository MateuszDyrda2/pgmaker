#include "cmain_menu.h"

#include <nfd.h>

using namespace libpgmaker;
cmain_menu::cmain_menu(std::vector<std::shared_ptr<libpgmaker::video>>& videos):
    videos(videos)
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
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Edit"))
        {
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}