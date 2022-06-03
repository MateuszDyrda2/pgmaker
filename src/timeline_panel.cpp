#include "timeline_panel.h"

using namespace libpgmaker;
timeline_panel::timeline_panel()
{
}
timeline_panel::~timeline_panel()
{
}
void timeline_panel::draw()
{
    auto proj = project_manager::get_current_project();
    auto& tl  = proj->get_timeline();
    ImGui::Begin("Timeline");
    {
        draw_tools(tl);
        draw_timeline(tl);
    }
    ImGui::End();
}
void timeline_panel::draw_tools(timeline& tl)
{
    auto regSize = ImGui::GetWindowSize().x / 2.f;
    ImGui::SetCursorPos(ImVec2(regSize - 70.f, 20.f));
    if(ImGui::Button("<<", ImVec2(40.f, 40.f)))
    {
        tl.seek(tl.get_timestamp() - std::chrono::milliseconds(3000));
    }
    ImGui::SetCursorPos(ImVec2(regSize - 20.f, 20.f));
    if(ImGui::Button(">", ImVec2(40.f, 40.f)))
    {
        // tl.set_paused(!tl.is_paused());
        tl.toggle_pause();
    }
    ImGui::SetCursorPos(ImVec2(regSize + 30.f, 20.f));
    if(ImGui::Button(">>", ImVec2(40.f, 40.f)))
    {
        tl.seek(tl.get_timestamp() + std::chrono::milliseconds(3000));
    }
}

void timeline_panel::draw_timeline(libpgmaker::timeline& tl)
{
    ImGui::BeginChild("TimelineContent", ImGui::GetContentRegionAvail());
    {
        auto& io                        = ImGui::GetIO();
        auto drawList                   = ImGui::GetWindowDrawList();
        auto canvasSize                 = ImGui::GetContentRegionAvail();
        auto canvasPos                  = ImGui::GetCursorScreenPos();
        static constexpr auto textWidth = 100.f;
        float xmin                      = canvasPos.x + textWidth;
        float xmax                      = canvasPos.x + canvasSize.x;
        float channelHeight = 60, channelMargin = 10;
        const auto timemax = 20000;

        /////////////////////////////////////////////
        // DRAW LINE
        /////////////////////////////////////////////
        // if(ImGui::IsItemActive())
        if(ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            auto mousePos = io.MouseClickedPos[0];
            if(mousePos.x >= xmin && mousePos.x <= xmax)
            {
                auto timePos = (mousePos.x - xmin) / (xmax - xmin) * timemax;
                tl.seek(std::chrono::milliseconds(std::int64_t(timePos)));
            }
        }
        const auto ts  = tl.get_timestamp().count();
        float xlinepos = xmin + (ts / double(timemax) * (xmax - xmin));

        /////////////////////////////////////////////

        for(const auto& ch : tl.get_channels())
        {
            draw_channel(tl, ch, xmin, xmax, canvasPos, canvasSize, drawList);
        }
        if(ImGui::Button("+", ImVec2(40, 40)))
        {
            tl.add_channel();
        }
        drawList->AddLine(
            { float(xlinepos), canvasPos.y },
            { float(xlinepos), canvasPos.y + canvasSize.y },
            0xFFFF0000,
            3.f);
    }
    ImGui::EndChild();
}
void timeline_panel::draw_channel(timeline& tl, const std::unique_ptr<channel>& ch, float xmin, float xmax,
                                  ImVec2 canvasPos, ImVec2 canvasSize, ImDrawList* drawList)
{
    auto index = ch->get_index();
    // Channel name
    std::string channName = "Channel " + std::to_string(index);
    ImGui::SetCursorPos(ImVec2(canvasPos.x, (channelHeight + channelMargin) * index));
    if(ImGui::Button(channName.c_str(), { xmin - canvasPos.x, channelHeight }))
        ;
    //
    ImGui::PushID(ch->get_id());
    ImGui::SetCursorPos(ImVec2(xmin - canvasPos.x, (channelHeight + channelMargin) * index));
    ImGui::InvisibleButton("", ImVec2(xmax - xmin, channelHeight));
    float spacing = index * (channelHeight + channelMargin);

    /*drawList->AddText(
        { xmin - 100, canvasPos.y + ((channelHeight + channelMargin) * index) },
        0xFFFFFFFF, channName.c_str());*/
    drawList->AddRectFilled(
        { xmin, canvasPos.y + ((channelHeight + channelMargin) * index) },
        { xmax, canvasPos.y + ((channelHeight + channelMargin) * index) + channelHeight },
        0xFF3D3837, 0);
    if(ImGui::BeginDragDropTarget())
    {
        if(const auto payload = ImGui::AcceptDragDropPayload("demo"))
        {
            auto data = *static_cast<std::shared_ptr<video>*>(payload->Data);
            tl.append_clip(index, data);
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::SetItemAllowOverlap();
    std::size_t j = 0;
    for(const auto& cl : ch->get_clips())
    {
        draw_clip(tl, cl, index, j, xmin, xmax, canvasPos, canvasSize, drawList);
        ++j;
    }
    ImGui::PopID();
}
void timeline_panel::draw_clip(libpgmaker::timeline& tl,
                               const std::unique_ptr<clip>& cl,
                               std::size_t i,
                               std::size_t j,
                               float xmin, float xmax,
                               ImVec2 canvasPos, ImVec2 canvasSize,
                               ImDrawList* drawList)
{
    ImGui::PushID(cl->get_id());

    auto starts    = cl->get_starts_at().count();
    auto ends      = starts + cl->get_duration().count();
    auto stDiv     = starts / double(timemax);
    auto edDiv     = ends / double(timemax);
    float clipXMin = xmin + stDiv * (xmax - xmin),
          clipXMax = xmin + edDiv * (xmax - xmin);
    ImGui::SetCursorPos({ clipXMin, (channelHeight + channelMargin) * i });
    ImGui::InvisibleButton("", { clipXMax - clipXMin, channelHeight });

    bool isItemActive    = ImGui::IsItemActive();
    bool isMouseDragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
    if(isItemActive && isMouseDragging)
    {
        wasMoved   = true;
        movedIndex = cl->get_id();
        auto delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
        clipXMin += delta.x;
        clipXMax += delta.x;
    }
    else if(wasMoved && !isMouseDragging && movedIndex == cl->get_id())
    {
        wasMoved   = false;
        auto delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
        // auto posdiff      = io.MouseDelta.x;
        float newClipXmin = clipXMin + delta.x;
        auto inDur        = ((newClipXmin - xmin) / (xmax - xmin)) * timemax;
        tl.move_clip(i, j, std::chrono::milliseconds(std::int64_t(inDur)));
    }
    drawList->AddRectFilled(
        { float(clipXMin), canvasPos.y + ((channelHeight + channelMargin) * i) },
        { float(clipXMax), canvasPos.y + ((channelHeight + channelMargin) * i) + channelHeight },
        0xFF0000AA, 2.5f);
    ImGui::PopID();
}
