#include "properties_panel.h"

#include "command_handler.h"

using namespace libpgmaker;
properties_panel::properties_panel():
    info{}
{
    command_handler::listen(
        "DisplayInfo",
        std::function(
            [this](command& c) {
                auto cl = reinterpret_cast<clip_info*>(c.data);
                info    = *cl;
            }));
}
properties_panel::~properties_panel() { }
void properties_panel::draw()
{
    ImGui::Begin("Properties");
    {
        ImGui::Text("Name: %s", info.name.c_str());
        ImGui::Text("Path: %s", info.path.c_str());
        ImGui::Text("Duration: %lfs", info.duration);
        ImGui::Text("Resolution: %dx%d", info.width, info.height);
    }
    ImGui::End();
}