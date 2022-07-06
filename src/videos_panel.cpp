#include "videos_panel.h"

videos_panel::videos_panel()
{
}
videos_panel::~videos_panel()
{
}
void videos_panel::draw()
{
    auto proj                  = project_manager::get_current_project();
    const auto& videos         = proj->get_videos();
    const ImVec2 thumbnailSize = { 106.f, 60.f };
    const ImVec2 itemSpacing   = { 10.f, 10.f };
    ImGui::Begin("Videos");
    {
        auto windowSize = ImGui::GetContentRegionAvail();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, itemSpacing);
        std::size_t vidPerRow = windowSize.x / (thumbnailSize.x + itemSpacing.x);
        std::size_t line      = 0;
        if(vidPerRow > 0)
        {
            for(const auto& [name, vid] : videos)
            {
                if((line % vidPerRow) != 0) ImGui::SameLine(0.f);

                if(ImGui::ImageButton(reinterpret_cast<ImTextureID>(vid->get_texture()), thumbnailSize))
                {
                }
                if(ImGui::BeginDragDropSource())
                {
                    ImGui::SetDragDropPayload("demo", &vid, sizeof(&vid));
                    ImGui::ImageButton(reinterpret_cast<ImTextureID>(vid->get_texture()), thumbnailSize);
                    ImGui::EndDragDropSource();
                }
                ++line;
            }
        }
        ImGui::PopStyleVar();
    }
    ImGui::End();
}
