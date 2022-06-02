#include "cvideos.h"

cvideos::cvideos()
{
}
cvideos::~cvideos()
{
}
void cvideos::draw()
{
    auto proj          = project_manager::get_current_project();
    const auto& videos = proj->get_videos();
    ImGui::Begin("Videos");
    {
        std::size_t line = 0;
        for(const auto& [name, vid] : videos)
        {
            if((line % 2) != 0) ImGui::SameLine(0.f);

            if(ImGui::ImageButton(reinterpret_cast<ImTextureID>(vid->get_texture()), ImVec2(106.f, 60.f)))
            {
            }
            if(ImGui::BeginDragDropSource())
            {
                ImGui::SetDragDropPayload("demo", &vid, sizeof(&vid));
                ImGui::ImageButton(reinterpret_cast<ImTextureID>(vid->get_texture()), ImVec2(106.f, 60.f));
                ImGui::EndDragDropSource();
            }
        }
    }
    ImGui::End();
}
