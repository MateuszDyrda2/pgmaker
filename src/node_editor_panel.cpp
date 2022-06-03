#include "node_editor_panel.h"

#include "project.h"

node_editor_panel::node_editor_panel()
{
    auto proj           = project_manager::get_current_project();
    std::size_t nbClips = 0;
    if(proj)
    {
        const auto& ch = proj->get_timeline().get_channels();
        for(const auto& c : ch)
        {
            nbClips += c->get_clips().size();
        }
    }
    blockEditor = std::make_unique<BlockEditor>(500.f, 200.f, nbClips);
}
node_editor_panel::~node_editor_panel()
{
}
void node_editor_panel::draw()
{
    if(ImGui::Begin("NodeEditor"))
    {
        auto drawList = ImGui::GetWindowDrawList();
        blockEditor->interact();
        blockEditor->draw(drawList);
    }
    ImGui::End();
}