#include "main_application.h"

main_application::main_application()
{
}
main_application::~main_application()
{
}
void main_application::update()
{
    mainMenu.draw();
    ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
    ImGuiWindowFlags windowFlags      = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    ////
    const auto vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);
    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ////
    if(dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
    {
        windowFlags |= ImGuiWindowFlags_NoBackground;
    }
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));

    ImGui::Begin("MainDockSpace", nullptr, windowFlags);
    {
        ImGui::PopStyleVar(3);
        auto dockspaceId = ImGui::GetID("MyDockspace");
        if(ImGui::DockBuilderGetNode(dockspaceId) == NULL)
        {
            initialize_layout(dockspaceId);
        }
        else
        {
            ImGui::DockSpace(dockspaceId, ImVec2(0, 0), dockspaceFlags);

            timeline.draw();
            videos.draw();
            properties.draw();
            nodeEditor.draw();
            playback.draw();
        }
    }
    ImGui::End();
}

void main_application::initialize_layout(ImGuiID dockspaceId)
{
    ImGui::DockBuilderGetNode(dockspaceId);
    ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->Size);

    auto dockMainId  = dockspaceId;
    auto dockIdDown  = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Down, 0.2f, NULL, &dockMainId);
    auto dockIdLeft  = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Left, 0.2f, NULL, &dockMainId);
    auto dockIdRight = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Right, 0.25f, NULL, &dockMainId);

    ImGui::DockBuilderDockWindow("Timeline", dockIdDown);
    ImGui::DockBuilderDockWindow("Videos", dockIdLeft);
    ImGui::DockBuilderDockWindow("Properties", dockIdRight);
    ImGui::DockBuilderDockWindow("Playback", dockMainId);
    ImGui::DockBuilderDockWindow("NodeEditor", dockMainId);
    ImGui::DockBuilderFinish(dockspaceId);
}