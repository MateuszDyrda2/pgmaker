#include "main_menu_panel.h"

#include "command_handler.h"
#include <nfd.h>

#include <fstream>
#include <iostream>

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
                command_handler::send("OpenVideo");
            };
            if(ImGui::MenuItem("Create project"))
            {
                command_handler::send("CreateProject");
            }
            if(ImGui::MenuItem("Save project"))
            {
                command_handler::send("SaveProject");
            }
            if(ImGui::MenuItem("Load project"))
            {
                command_handler::send("LoadProject");
            }
            if(ImGui::MenuItem("Exit"))
            {
                command_handler::send("ExitApplication");
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