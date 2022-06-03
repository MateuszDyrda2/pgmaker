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
        while(nbTextures-- > 0)
        {
            auto& tex = textures[nbTextures];
            ImGui::SetCursorPos({ (size.x - tex.size.first) * 0.5f,
                                  (size.y - tex.size.second) * 0.5f });
            ImGui::Image(
                reinterpret_cast<ImTextureID>(textures[nbTextures].handle),
                { textures[nbTextures].size.first, textures[nbTextures].size.second });
        }
        ////////////////////////
        // ImGui::EndChild();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}
