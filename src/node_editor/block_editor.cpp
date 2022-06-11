#include "block_editor.h"

#include <algorithm>

#include "command_handler.h"
#include <libpgmaker/effect.h>

using namespace std;
using namespace libpgmaker;
block_editor::block_editor()
{
}
block_editor::~block_editor()
{
}
void block_editor::add_connection(size_t channelIdx, size_t clipHandle)
{
    connection c;
    float rowY   = 100.f + connections.size() * (nodeSize.y + nodeYMargin);
    c.in.pos     = { 100.f, rowY };
    c.in.color   = 0xFF0000FF;
    c.out.pos    = { 800.f, rowY };
    c.out.color  = 0xFF00FF00;
    c.channelIdx = channelIdx;
    c.clipHandle = clipHandle;
    connections.push_back(c);
}
ImVec2 pos2reg(ImVec2 wp, ImVec2 p)
{
    return ImVec2(p.x - wp.x, p.y - wp.y);
}
ImVec2 pos2gl(ImVec2 wp, ImVec2 p)
{
    return ImVec2(p.x + wp.x, p.y + wp.y);
}
void block::draw(ImDrawList* drawList, ImVec2 windowPos)
{
    drawList->AddRectFilled(pos2gl(windowPos, pos),
                            pos2gl(windowPos, { pos.x + block_editor::nodeSize.x, pos.y + block_editor::nodeSize.y }),
                            color, 3.5f);
    drawList->AddRect(pos2gl(windowPos, pos),
                      pos2gl(windowPos, { pos.x + block_editor::nodeSize.x, pos.y + block_editor::nodeSize.y }),
                      0xFFFFFFFF, 3.5f, 0, 3.f);
}
bool effect_block::interact()
{
    ImGui::SetCursorPos(pos);
    ImGui::InvisibleButton("", block_editor::nodeSize);
    auto isActive        = ImGui::IsItemActive();
    auto isMouseDragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
    ImGui::SetItemAllowOverlap();
    ImGui::SetCursorPos({ pos.x + 20, pos.y + (block_editor::nodeSize.y * 0.5f) - 10 });
    ImGui::SetNextItemWidth(block_editor::nodeSize.x - 40.f);
    if(ImGui::BeginCombo("##combo", currentItem))
    {
        for(size_t i = 0; i < IM_ARRAYSIZE(effect::effectNames); ++i)
        {
            bool isSelected = (currentItem == effect::effectNames[i]);
            if(ImGui::Selectable(effect::effectNames[i], &isSelected))
            {
                currentItem = effect::effectNames[i];
            }
            if(isSelected)
            {
                ImGui::SetItemDefaultFocus();
                if(attachedTo)
                {
                    command_handler::send(
                        { "AttachEffect",
                          new std::tuple(
                              attachedTo->channelIdx, attachedTo->clipHandle, effect::effect_type(i)) });
                }
            }
        }
        ImGui::EndCombo();
    }
    if(isActive && isMouseDragging)
    {
        isBeingDragged = true;
        auto delta     = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
        auto realDelta = ImVec2{ delta.x - lastDelta.x, delta.y - lastDelta.y };
        lastDelta      = delta;
        pos.x += realDelta.x;
        pos.y += realDelta.y;
    }
    else if(isBeingDragged)
    {
        isBeingDragged = false;
        lastDelta      = { 0.f, 0.f };
        return true;
    }
    return false;
}
void connection::draw(ImDrawList* drawList, ImVec2 windowPos, ImVec2 windowSize)
{
    drawList->AddLine(
        pos2gl(windowPos, { in.pos.x + block_editor::nodeSize.x, in.pos.y + (block_editor::nodeSize.y * 0.5f) }),
        pos2gl(windowPos, { out.pos.x, in.pos.y + (block_editor::nodeSize.y * 0.5f) }),
        0xFFA75DD9, 4.f);
    in.draw(drawList, windowPos);
    out.pos.x = windowSize.x - 100.f - block_editor::nodeSize.x;
    out.draw(drawList, windowPos);
}
void connection::interact(size_t& i, std::list<effect_block>& freeEffects)
{
}
bool connection::is_contained(ImVec2 pos)
{
    return (pos.x >= (in.pos.x + block_editor::nodeSize.x) && pos.x <= out.pos.x)
           && (pos.y >= in.pos.y && pos.y <= (in.pos.y + block_editor::nodeSize.y));
}
void connection::add_effect(effect_block* ef)
{
    ef->attachedTo = this;
    effects.push_back(ef);
    ef->pos = {
        in.pos.x + (out.pos.x - in.pos.x) * 0.5f,
        in.pos.y
    };
    ef->color = 0xFFAA2200;
}
void connection::remove_effect(effect_block* ef)
{
    ef->color      = 0xFFFF0000;
    ef->attachedTo = nullptr;
    effects.erase(std::remove(effects.begin(), effects.end(), ef));
}
void block_editor::add_effect()
{
    effect_block ef;
    ef.pos   = { 500.f, 10.f };
    ef.color = 0xFFFF0000;
    effects.push_back(ef);
}
void block_editor::draw()
{
    auto pos      = ImGui::GetWindowPos();
    auto size     = ImGui::GetContentRegionAvail();
    auto drawList = ImGui::GetWindowDrawList();

    for(auto& connection : connections)
    {
        connection.draw(drawList, pos, size);
    }
    for(auto& ef : effects)
    {
        ef.draw(drawList, pos);
    }
    for(auto& ef : effects)
    {
        ImGui::PushID(&ef);
        if(ef.interact())
        {
            auto res = find_if(
                connections.begin(), connections.end(),
                [&ef](auto& con) {
                    return con.is_contained(ImVec2{ ef.pos.x + block_editor::nodeSize.x * 0.5f,
                                                    ef.pos.y + block_editor::nodeSize.y * 0.5f });
                });
            if(res != connections.end())
            {
                if(ef.attachedTo && ef.attachedTo != &(*res)) ef.attachedTo->remove_effect(&ef);

                res->add_effect(&ef);
            }
            else
            {
                if(ef.attachedTo) ef.attachedTo->remove_effect(&ef);
            }
        }
        ImGui::PopID();
    }
    ImGui::SetCursorPos({ size.x * 0.5f, size.y - 50.f });
    if(ImGui::Button("Add effect", { 100.f, 50.f }))
    {
        add_effect();
    }
}