#include "cnode_editor.h"
cnode_editor::cnode_editor()
{
    blockEditor = std::make_unique<BlockEditor>(300.f, 200.f, 5);
}
cnode_editor::~cnode_editor()
{
}
void cnode_editor::draw()
{
    if(ImGui::Begin("NodeEditor"))
    {
        auto drawList = ImGui::GetWindowDrawList();
        blockEditor->interact();
        blockEditor->draw(drawList);
    }
    ImGui::End();
}