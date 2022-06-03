#include "main_menu_panel.h"

#include "command_handler.h"
#include <nfd.hpp>

#include <fstream>
#include <iostream>

#if defined(WIN32)
#    define NFD_FILTER_ITEM nfdu8filteritem_t
#else
#    define NFD_FILTER_ITEM nfdnfilteritem_t
#endif

using namespace libpgmaker;
main_menu_panel::main_menu_panel()
{
}
main_menu_panel::~main_menu_panel()
{
}
void main_menu_panel::draw()
{
    if(ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("Open Video"))
            {
                NFD::UniquePath outPath;
                auto result = NFD::OpenDialog(outPath, NULL, 0, NULL);
                if(result == NFD_OKAY)
                {
                    command_handler::send({ "OpenVideo", new std::string(outPath.get()) });
                }
            };
            if(ImGui::MenuItem("Create project"))
            {
                NFD::UniquePath outPath;
                NFD_FILTER_ITEM a = { "PGMaker projects", "pgproj" };
                auto result       = NFD::SaveDialog(outPath, &a, 1, NULL, "NewProject.pgproj");
                if(result == NFD_OKAY)
                {
                    command_handler::send({ "CreateProject", new std::string(outPath.get()) });
                }
            }
            if(ImGui::MenuItem("Save project"))
            {
                command_handler::send({ "SaveProject" });
            }
            if(ImGui::MenuItem("Load project"))
            {
                NFD::UniquePath outPath;
                NFD_FILTER_ITEM a = { "PGMaker projects", "pgproj" };
                auto result       = NFD::OpenDialog(outPath, &a, 1, 0);
                if(result == NFD_OKAY)
                {
                    command_handler::send({ "LoadProject", new std::string(outPath.get()) });
                }
            }
            if(ImGui::MenuItem("Exit"))
            {
                command_handler::send({ "ExitApplication" });
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Edit"))
        {
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}