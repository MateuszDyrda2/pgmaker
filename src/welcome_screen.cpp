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
            /*
            nfdchar_t* outPath;
            nfdnfilteritem_t a = { "PGMaker projects", "pgproj" };
            auto result        = NFD_SaveDialog(&outPath, &a, 1, NULL, "NewProject.pgproj");
            if(result == NFD_OKAY)
            {
                try
                {
                    project_manager::create_project(outPath);
                    command_handler::send("StartApplication");
                }
                catch(const std::runtime_error& err)
                {
                    std::cerr << err.what();
                    NFD_FreePath(outPath);
                    ImGui::EndPopup();
                    return;
                }

                NFD_FreePath(outPath);
                ImGui::CloseCurrentPopup();
            }
            */
        }
        if(ImGui::Button("Load project"))
        {
            command_handler::send("LoadProject");
            command_handler::send("StartApplication");
            ImGui::CloseCurrentPopup();
            /*
            nfdchar_t* outPath;
            nfdnfilteritem_t a = { "PGMaker projects", "pgproj" };
            auto result        = NFD_OpenDialog(&outPath, &a, 1, 0);
            if(result == NFD_OKAY)
            {
                try
                {
                    project_manager::load_project(outPath);
                    command_handler::send("StartApplication");
                }
                catch(const std::runtime_error& err)
                {
                    std::cerr << err.what();
                    NFD_FreePath(outPath);
                    ImGui::EndPopup();
                    return;
                }
                ImGui::CloseCurrentPopup();
                NFD_FreePath(outPath);
            }
            */
        }
        if(ImGui::Button("Exit"))
        {
            command_handler::send("ExitApplication");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}