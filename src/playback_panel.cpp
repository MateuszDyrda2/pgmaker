#include "playback_panel.h"

using namespace libpgmaker;
playback_panel::playback_panel()
{
}
playback_panel::~playback_panel()
{
}
void playback_panel::draw()
{
    auto proj = project_manager::get_current_project();
    auto& tl  = proj->get_timeline();
    auto documentFlags =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground
        | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
    if(ImGui::Begin("Playback", NULL, documentFlags))
    {
        // ImGui::BeginChild("VideoRender");

        ////////////////////////
        auto pos  = ImGui::GetWindowPos();
        auto size = ImGui::GetWindowSize();
        tl.set_size({ size.x, size.y });

        auto&& [textures, nbTextures] = tl.next_frame();
        for(int it = nbTextures - 1; it >= 0; --it)
        {
            ImGui::Image(
                reinterpret_cast<ImTextureID>(textures[it].handle),
                { textures[it].size.first, textures[it].size.second });
        }
        ////////////////////////
        // ImGui::EndChild();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}
