#include "welcome_screen.h"

#include "panel.h"

#include "command_handler.h"
#include <iostream>
#include <nfd.hpp>

#if defined(WIN32)
#    define NFD_FILTER_ITEM nfdu8filteritem_t
#else
#    define NFD_FILTER_ITEM nfdnfilteritem_t
#endif

welcome_screen::welcome_screen()
{
}
welcome_screen::~welcome_screen()
{
}
void welcome_screen::update()
{
    auto popupFlags = ImGuiPopupFlags_NoOpenOverExistingPopup;
    ImGui::OpenPopup("Welcome", popupFlags);

    auto windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                       | ImGuiWindowFlags_NoCollapse;

    if(ImGui::BeginPopupModal("Welcome", 0, windowFlags))
    {
        if(ImGui::Button("Create new project"))
        {
            command_handler::send({ "StartProjectCreation" });
            ImGui::CloseCurrentPopup();
        }
        if(ImGui::Button("Load project"))
        {
            NFD::UniquePath outPath;
            NFD_FILTER_ITEM a = { "PGMaker projects", "pgproj" };
            auto result       = NFD::OpenDialog(outPath, &a, 1, 0);
            if(result == NFD_OKAY)
            {
                command_handler::send({ "LoadProject", new std::string(outPath.get()) });
                command_handler::send({ "StartApplication" });
            }
            ImGui::CloseCurrentPopup();
        }
        if(ImGui::Button("Exit"))
        {
            command_handler::send({ "ExitApplication" });
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}