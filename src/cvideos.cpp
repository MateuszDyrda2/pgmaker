#include "cvideos.h"

cvideos::cvideos(std::vector<std::shared_ptr<libpgmaker::video>>& videos):
    videos(videos)
{
}
cvideos::~cvideos()
{
}
void cvideos::draw()
{
    ImGui::Begin("Videos");
    {
        for(std::size_t i = 0; i < videos.size(); ++i)
        {
            if((i % 3) != 0)
            {
                ImGui::SameLine(0.f);
            }
            if(ImGui::ImageButton(reinterpret_cast<ImTextureID>(videos[i]->get_texture()),
                                  ImVec2(106.f, 60.f)))
            {
                // selectedVideo = videos[i].get();
            }
            if(ImGui::BeginDragDropSource())
            {
                ImGui::SetDragDropPayload("demo", &videos[i], sizeof(&videos[i]));
                ImGui::ImageButton(reinterpret_cast<ImTextureID>(videos[i]->get_texture()),
                                   ImVec2(106.f, 60.f));
                ImGui::EndDragDropSource();
            }
        }
    }
    ImGui::End();
}
