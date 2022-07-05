#include "project_creator.h"

#include "command_handler.h"
#include <nfd.hpp>

#include <imgui.h>
#include <string>
#include <utility>

#if defined(WIN32)
#    define NFD_FILTER_ITEM nfdu8filteritem_t
#else
#    define NFD_FILTER_ITEM nfdnfilteritem_t
#endif

project_creator::project_creator():
    framerate(30), width(1920), height(1080)
{
}
project_creator::~project_creator() { }
void project_creator::update()
{
    ImGui::OpenPopup("Create project");

    auto windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
    if(ImGui::BeginPopupModal("Create project", 0, windowFlags))
    {
        ImGui::InputFloat("Framerate", &framerate, 0.f, 0.f, "%.2f");
        ImGui::InputInt("Width", &width);
        ImGui::InputInt("Height", &height);
        if(ImGui::Button("Choose file"))
        {
            NFD::UniquePath outPath;
            NFD_FILTER_ITEM a = { "PGMaker projects", "pgproj" };
            auto result       = NFD::SaveDialog(outPath, &a, 1, NULL, "NewProject.pgproj");
            if(result == NFD_OKAY)
            {
                command_handler::send({ "CreateProject",
                                        new std::tuple(std::string(outPath.get()), framerate, width, height) });
                command_handler::send({ "StartApplication" });
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}