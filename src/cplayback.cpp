#include "cplayback.h"

using namespace libpgmaker;
cplayback::cplayback()
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 textureSize.first, textureSize.second, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, 0);
}
cplayback::~cplayback()
{
}
void cplayback::draw()
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

        glBindTexture(GL_TEXTURE_2D, texture);
        auto frame = tl.next_frame();
        if(frame)
        {
            if(frame->size != textureSize)
            {
                textureSize = frame->size;
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSize.first, textureSize.second,
                             0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            }
            else
            {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                                textureSize.first, textureSize.second, GL_RGBA,
                                GL_UNSIGNED_BYTE, frame->data.data());
            }
        }
        ImGui::Image(reinterpret_cast<ImTextureID>(texture), size);
        ////////////////////////
        // ImGui::EndChild();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}
