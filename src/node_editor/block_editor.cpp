#include "block_editor.h"

#include <algorithm>

#include "project.h"

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
void block_editor::remove_connection(size_t channelIdx, size_t clipHandle)
{
    auto res = std::find_if(
        connections.begin(), connections.end(),
        [&](auto& c) {
            return c.channelIdx == channelIdx && c.clipHandle == clipHandle;
        });
    if(res != connections.end())
    {
        for(auto& ef : res->effects)
        {
            ef->attachedTo = nullptr;
        }
        connections.erase(res);
    }
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
bool effect_block::interact(bool& remove, std::queue<command>& pendingCommands)
{
    ImGui::SetCursorPos(pos);
    ImGui::InvisibleButton("", block_editor::nodeSize);
    auto isActive        = ImGui::IsItemActive();
    auto isMouseDragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
    if(ImGui::BeginPopupContextItem("##popup", 1))
    {
        if(ImGui::MenuItem("Remove"))
        {
            if(attachedTo) attachedTo->remove_effect(this, pendingCommands);
            remove = true;
        }
        ImGui::EndPopup();
    }
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
                if(attachedTo)
                {
                    if(currentItem != effect::effectNames[0])
                    {
                        if(i == 0)
                        {
                            pendingCommands.push(
                                { "RemoveEffects",
                                  new std::pair(
                                      attachedTo->channelIdx, attachedTo->clipHandle) });
                        }
                        else if(currentItem != effect::effectNames[i])
                        {
                            pendingCommands.push(
                                { "ChangeEffect",
                                  new std::tuple(
                                      attachedTo->channelIdx, attachedTo->clipHandle, effect::effect_type(i)) });
                        }
                    }
                    else
                    {
                        pendingCommands.push(
                            { "AttachEffect",
                              new std::tuple(
                                  attachedTo->channelIdx, attachedTo->clipHandle, effect::effect_type(i)) });
                    }
                }
                currentItem = effect::effectNames[i];
            }
            if(isSelected)
            {
                ImGui::SetItemDefaultFocus();
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
bool connection::is_contained(ImVec2 pos)
{
    return (pos.x >= (in.pos.x + block_editor::nodeSize.x) && pos.x <= out.pos.x)
           && (pos.y >= in.pos.y && pos.y <= (in.pos.y + block_editor::nodeSize.y));
}
void connection::recalc_effects()
{
    auto [begin, end] = ImVec2(in.pos.x + block_editor::nodeSize.x, out.pos.x - block_editor::nodeSize.x);
    auto space        = end - begin;
    auto spacing      = space / (effects.size() + 1);
    size_t i          = 1;
    for(auto& f : effects)
    {
        f->pos = {
            begin + spacing * i,
            in.pos.y
        };
        ++i;
    }
}
void connection::add_effect(effect_block* ef, std::queue<command>& pendingCommands)
{
    ef->attachedTo = this;
    effects.push_back(ef);
    ef->color = 0xFFAA2200;
    recalc_effects();
    if(ef->currentItem != effect::effectNames[0])
    {
        int found = 0;
        for(size_t i = 1; i < IM_ARRAYSIZE(effect::effectNames); ++i)
        {
            if(ef->currentItem == effect::effectNames[i])
            {
                found = i;
                break;
            }
        }
        if(found != 0)
        {
            pendingCommands.push({ "AttachEffect",
                                   new std::tuple(
                                       channelIdx, clipHandle, effect::effect_type(found)) });
        }
    }
}
void connection::remove_effect(effect_block* ef, std::queue<command>& pendingCommands)
{
    ef->color      = 0xFFFF0000;
    ef->attachedTo = nullptr;
    effects.erase(std::remove(effects.begin(), effects.end(), ef));
    recalc_effects();
    if(ef->currentItem != effect::effectNames[0])
    {
        pendingCommands.push(
            { "RemoveEffects",
              new std::pair(channelIdx, clipHandle) });
        for(auto& f : effects)
        {
            size_t i = 1;
            for(; i < IM_ARRAYSIZE(effect::effectNames); ++i)
            {
                if(effect::effectNames[i] == f->currentItem) break;
            }
            if(f->currentItem != effect::effectNames[0])
            {
                pendingCommands.push(
                    { "AttachEffect",
                      new std::tuple(channelIdx, clipHandle, effect::effect_type(i)) });
            }
        }
    }
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
    if(ImGui::BeginPopupModal("Effect loading"))
    {
        auto proj = project_manager::get_current_project();
        if(proj->get_effect_complete().wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
        {
            proj->get_effect_complete().get();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    else
    {
        if(!pendingCommands.empty())
        {
            command_handler::send(pendingCommands.front());
            pendingCommands.pop();
            ImGui::OpenPopup("Effect loading");
        }
        for(auto iter = effects.begin(); iter != effects.end();)
        {
            ImGui::PushID(&(*iter));
            bool remove = false;
            if(iter->interact(remove, pendingCommands))
            {
                auto res = find_if(
                    connections.begin(), connections.end(),
                    [iter](auto& con) {
                        return con.is_contained(ImVec2{ iter->pos.x + block_editor::nodeSize.x * 0.5f,
                                                        iter->pos.y + block_editor::nodeSize.y * 0.5f });
                    });
                if(res != connections.end())
                {
                    if(iter->attachedTo && iter->attachedTo != &(*res)) iter->attachedTo->remove_effect(&(*iter), pendingCommands);

                    res->add_effect(&(*iter), pendingCommands);
                }
                else
                {
                    if(iter->attachedTo) iter->attachedTo->remove_effect(&(*iter), pendingCommands);
                }
            }
            if(remove)
            {
                auto next = std::next(iter);
                effects.erase(iter);
                iter = next;
            }
            else
                ++iter;
            ImGui::PopID();
        }
        ImGui::SetCursorPos({ size.x * 0.5f, size.y - 50.f });
        if(ImGui::Button("Add effect", { 100.f, 50.f }))
        {
            add_effect();
        }
    }
}