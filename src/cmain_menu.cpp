#include "cmain_menu.h"

#include <nfd.h>

#include <fstream>
#include <iostream>

using namespace libpgmaker;
cmain_menu::cmain_menu()
{
}
cmain_menu::~cmain_menu()
{
}
void cmain_menu::draw()
{
    if(ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("Open Video"))
            {
                auto proj = project_manager::get_current_project();
                if(!proj) return;
                nfdchar_t* outPath;
                auto result = NFD_OpenDialog(&outPath, NULL, 0, NULL);
                if(result == NFD_OKAY)
                {
                    proj->load_video(outPath);
                    NFD_FreePath(outPath);
                }
            };
            if(ImGui::MenuItem("Create project"))
            {
                nfdchar_t* outPath;
                nfdnfilteritem_t a = { "PGMaker projects", "pgproj" };
                auto result        = NFD_SaveDialog(&outPath, &a, 1, NULL, "project.pgproj");
                if(result == NFD_OKAY)
                {
                    project_manager::create_project(outPath);
                    NFD_FreePath(outPath);
                }
            }
            if(ImGui::MenuItem("Save project"))
            {
                auto proj = project_manager::get_current_project();
                proj->save();
            }
            if(ImGui::MenuItem("Load project"))
            {
                nfdchar_t* outPath;
                nfdnfilteritem_t a = { "PGMaker projects", "pgproj" };
                auto result        = NFD_OpenDialog(&outPath, &a, 1, 0);
                if(result == NFD_OKAY)
                {
                    auto proj = project_manager::load_project(outPath);
                    NFD_FreePath(outPath);
                }
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