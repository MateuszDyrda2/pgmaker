#include "playback_panel.h"

#include "command_handler.h"

using namespace libpgmaker;
playback_panel::playback_panel()
{
}
playback_panel::~playback_panel()
{
}
void playback_panel::draw()
{
    auto proj         = project_manager::get_current_project();
    auto& tl          = proj->get_timeline();
    auto rFramerate   = std::chrono::duration<float>(proj->get_r_framerate());
    auto rProjectSize = proj->get_r_size();
    auto projectSize  = proj->get_size();

    float videoOptHeight = 60.f;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.f, 0.f });
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.f, 0.f });
    auto& st = ImGui::GetStyle();
    if(ImGui::Begin("Playback"))
    {
        auto vppos  = ImGui::GetWindowPos();
        auto vpsize = ImGui::GetContentRegionAvail();
        ImGui::BeginChild("VideoRender", { vpsize.x, vpsize.y - videoOptHeight });
        {
            ////////////////////////
            auto pos     = ImGui::GetWindowPos();
            auto size    = ImGui::GetContentRegionAvail();
            auto xScaler = size.x * rProjectSize.first;
            auto yScaler = size.y * rProjectSize.second;

            auto&& [textures, nbTextures] = tl.next_frame();
            while(nbTextures-- > 0)
            {
                auto& tex       = textures[nbTextures];
                auto texScaledW = tex.size.width * xScaler;
                auto texScaledH = tex.size.height * yScaler;
                ImGui::SetCursorPos({ (size.x - texScaledW) * 0.5f,
                                      (size.y - texScaledH) * 0.5f });
                ImGui::Image(
                    reinterpret_cast<ImTextureID>(textures[nbTextures].handle),
                    { texScaledW, texScaledH });
            }
            ////////////////////////
        }
        ImGui::EndChild();
        ImGui::BeginChild("VideoOpts", { vpsize.x, videoOptHeight });
        {
            constexpr auto buttonSize    = 40.f;
            constexpr auto buttonSpacing = 10.f;
            const auto hVpWidth          = vpsize.x * 0.5f;
            ImGui::SetCursorPosX(hVpWidth - (buttonSize * 1.5f + buttonSpacing));
            if(ImGui::Button("<<", { buttonSize, buttonSize }))
                command_handler::send({ "TimelineSeek",
                                        new std::chrono::milliseconds(
                                            tl.get_timestamp() - std::chrono::milliseconds(3000)) });
            ImGui::SameLine();
            ImGui::SetCursorPosX(hVpWidth - (buttonSize * 0.5f));
            if(ImGui::Button(">", { buttonSize, buttonSize }))
                command_handler::send({ "TimelinePause" });

            ImGui::SameLine();
            ImGui::SetCursorPosX(hVpWidth + (buttonSize * 0.5f + buttonSpacing));
            if(ImGui::Button(">>", { buttonSize, buttonSize }))
                command_handler::send({ "TimelineSeek",
                                        new std::chrono::milliseconds(
                                            tl.get_timestamp() - std::chrono::milliseconds(3000)) });
        }
        ImGui::EndChild();
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
}
