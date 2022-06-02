#include "node_editor_panel.h"

node_editor_panel::node_editor_panel()
{
    blockEditor = std::make_unique<BlockEditor>(300.f, 200.f, 5);
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