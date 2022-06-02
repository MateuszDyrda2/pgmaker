#include "welcome_screen.h"

#include "panel.h"

#include "command_handler.h"
#include <iostream>
#include <nfd.hpp>

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
            command_handler::send("CreateProject");
            command_handler::send("StartApplication");
            ImGui::CloseCurrentPopup();
        }
        if(ImGui::Button("Load project"))
        {
            command_handler::send("LoadProject");
            command_handler::send("StartApplication");
            ImGui::CloseCurrentPopup();
        }
        if(ImGui::Button("Exit"))
        {
            command_handler::send("ExitApplication");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}