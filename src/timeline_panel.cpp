#include "timeline_panel.h"

#include "command_handler.h"
#include "events.h"

using namespace libpgmaker;
timeline_panel::timeline_panel():
    currentState(state::CLK), frontCut{ false },
    backCut{ false }, cutIndex{}, wasMoved{ false }
{
    auto proj = project_manager::get_current_project();
    auto& tl  = proj->get_timeline();
    command_handler::listen(
        "TimelineSeek",
        [&tl](command& c) {
            auto timepos = reinterpret_cast<std::chrono::milliseconds*>(c.data);
            tl.seek(*timepos);
        });
    command_handler::listen(
        "TimelinePause",
        [&tl](command& c) {
            tl.toggle_pause();
        });
    command_handler::listen(
        "TimelineAddChannel",
        [&tl](command& c) {
            tl.add_channel();
        });
    command_handler::listen(
        "TimelineAppendClip",
        [&tl](command& c) {
            auto&& [index, vid] = *reinterpret_cast<std::pair<std::size_t, std::shared_ptr<video>>*>(c.data);
            tl.append_clip(index, vid);
            event_handler::fire_event("TimelineClipAppended");
        });
    command_handler::listen(
        "TimelineMoveClip",
        [&tl](command& c) {
            auto&& [index, j, to] = *reinterpret_cast<
                std::tuple<std::size_t, std::size_t, std::chrono::milliseconds>*>(c.data);
            tl.move_clip(index, j, to);
        });
    command_handler::listen(
        "TimelineClipStartOffset",
        [&tl](command& c) {
            auto&& [i, j, by] = *reinterpret_cast<
                std::tuple<std::size_t, std::size_t, std::chrono::milliseconds>*>(c.data);
            tl.get_channel(i)->get_clip(j)->change_start_offset(by);
        });
    command_handler::listen(
        "TimelineClipEndOffset",
        [&tl](command& c) {
            auto&& [i, j, by] = *reinterpret_cast<
                std::tuple<std::size_t, std::size_t, std::chrono::milliseconds>*>(c.data);
            tl.get_channel(i)->get_clip(j)->change_end_offset(by);
        });
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
    auto&& [winWidth, winHeight] = ImGui::GetContentRegionAvail();
    auto regSize                 = ImGui::GetWindowSize().x * 0.5f;

    ImGui::SetCursorPos(ImVec2(regSize - 55.f, 20.f));
    if(ImGui::Button("CLK", ImVec2(30.f, 30.f)))
    {
        currentState = state::CLK;
    }
    ImGui::SetCursorPos(ImVec2(regSize - 15.f, 20.f));
    if(ImGui::Button("MOV", ImVec2(30.f, 30.f)))
    {
        currentState = state::MOV;
    }
    ImGui::SetCursorPos(ImVec2(regSize + 25.f, 20.f));
    if(ImGui::Button("CUT", ImVec2(30.f, 30.f)))
    {
        currentState = state::CUT;
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
        float xmin                      = textWidth;
        float xmax                      = canvasSize.x;
        float channelHeight = 60, channelMargin = 10;
        const auto timemax = 20000;
        auto xminAbs = xmin + canvasPos.x, xmaxAbs = xmax + canvasPos.x;

        /////////////////////////////////////////////
        // DRAW LINE
        /////////////////////////////////////////////
        if(currentState == state::CLK)
        {
            if(ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                auto mousePos = io.MouseClickedPos[0];
                if(mousePos.x >= xminAbs && mousePos.x <= xmaxAbs)
                {
                    auto timePos = (mousePos.x - xminAbs) / (xmax - xmin) * timemax;
                    command_handler::send({ "TimelineSeek", new std::chrono::milliseconds(std::int64_t(timePos)) });
                }
            }
        }
        /////////////////////////////////////////////

        for(const auto& ch : tl.get_channels())
        {
            ImVec2 channelPos = { 0, (channelHeight + channelMargin) * ch->get_index() };
            draw_channel(tl, ch, xmin, xmax, channelPos, canvasPos, drawList);
        }
        if(ImGui::Button("+", ImVec2(40, 40)))
        {
            command_handler::send({ "TimelineAddChannel" });
        }

        const auto ts  = tl.get_timestamp().count();
        float xlinepos = xminAbs + (ts / double(timemax) * (xmax - xmin));
        drawList->AddLine(
            { float(xlinepos), canvasPos.y },
            { float(xlinepos), canvasPos.y + canvasSize.y },
            0xFFFF0000,
            3.f);
    }
    ImGui::EndChild();
}
void timeline_panel::draw_channel(timeline& tl, const std::unique_ptr<channel>& ch,
                                  float xmin, float xmax,
                                  ImVec2 channelPos,
                                  ImVec2 canvasPos, ImDrawList* drawList)
{
    auto index = ch->get_index();
    ImGui::SetCursorPos(channelPos);
    if(ImGui::Button(("Channel " + std::to_string(index)).c_str(), { 100, channelHeight }))
        ;
    //
    ImGui::PushID(ch->get_id());
    ImGui::SetCursorPos(ImVec2(xmin - canvasPos.x, channelPos.y));
    ImGui::InvisibleButton("", ImVec2(xmax - xmin, channelHeight));

    drawList->AddRectFilled(
        { xmin + canvasPos.x, canvasPos.y + channelPos.y },
        { xmax + canvasPos.x, canvasPos.y + channelPos.y + channelHeight },
        0xFF3D3837, 0);
    if(ImGui::BeginDragDropTarget())
    {
        if(const auto payload = ImGui::AcceptDragDropPayload("demo"))
        {
            auto data = *static_cast<std::shared_ptr<video>*>(payload->Data);
            command_handler::send({ "TimelineAppendClip", new std::pair<std::size_t, std::shared_ptr<video>>(index, data) });
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::SetItemAllowOverlap();
    std::size_t j = 0;
    for(const auto& cl : ch->get_clips())
    {
        draw_clip(tl, cl, index, j, xmin, xmax, channelPos, canvasPos, drawList);
        ++j;
    }
    ImGui::PopID();
}
void timeline_panel::draw_clip(libpgmaker::timeline& tl,
                               const std::unique_ptr<clip>& cl,
                               std::size_t i,
                               std::size_t j,
                               float xmin, float xmax,
                               ImVec2 channelPos,
                               ImVec2 canvasPos,
                               ImDrawList* drawList)
{
    ImGui::PushID(cl->get_id());

    auto starts    = cl->get_starts_at().count();
    auto ends      = cl->get_ends_at().count();
    auto stDiv     = starts / double(timemax);
    auto edDiv     = ends / double(timemax);
    float clipXMin = xmin + stDiv * (xmax - xmin),
          clipXMax = xmin + edDiv * (xmax - xmin);
    ImGui::SetCursorPos({ clipXMin, (channelHeight + channelMargin) * i });
    ImGui::InvisibleButton("", { clipXMax - clipXMin, channelHeight });

    bool isItemActive    = ImGui::IsItemActive();
    bool isMouseDragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
    switch(currentState)
    {
    case state::CLK:
        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
        handle_clk(cl);
        break;
    case state::MOV:
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        handle_mov(cl, i, j, clipXMin, clipXMax, xmin, xmax);
        break;
    case state::CUT:
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        handle_cut(cl, i, j, canvasPos, xmin, xmax);
        break;
    }
    drawList->AddRectFilled(
        { canvasPos.x + clipXMin, canvasPos.y + channelPos.y },
        { canvasPos.x + clipXMax, canvasPos.y + channelPos.y + channelHeight },
        0xFF0000AA, 4.f, 0);
    drawList->AddRect(
        { canvasPos.x + clipXMin, canvasPos.y + channelPos.y },
        { canvasPos.x + clipXMax, canvasPos.y + channelPos.y + channelHeight },
        0xFF000000, 4.f, 0, 3.f);

    ImGui::PopID();
}
void timeline_panel::handle_clk(const std::unique_ptr<libpgmaker::clip>& cl)
{
    bool isItemActive = ImGui::IsItemActive();
    if(isItemActive)
    {
        command_handler::send({ "DisplayInfo", new clip_info(cl->get_clip_info()) });
    }
}
void timeline_panel::handle_mov(const std::unique_ptr<libpgmaker::clip>& cl,
                                std::size_t i, std::size_t j,
                                float& clipXMin, float& clipXMax,
                                float xmin, float xmax)
{
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
        clipXMin += delta.x;
        clipXMax += delta.x;
        auto inDur = ((newClipXmin - xmin) / (xmax - xmin)) * timemax;
        command_handler::send(
            { "TimelineMoveClip",
              new std::tuple<std::size_t, std::size_t, std::chrono::milliseconds>(i, j, std::int64_t(inDur)) });
    }
}
void timeline_panel::handle_cut(const std::unique_ptr<libpgmaker::clip>& cl,
                                std::size_t i, std::size_t j,
                                ImVec2 canvasPos,
                                float xmin, float xmax)
{
    bool isItemActive       = ImGui::IsItemActive();
    bool isMouseDragging    = ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.1f);
    auto&& [mousex, mousey] = ImGui::GetMousePos();
    auto&& [minx, miny]     = ImGui::GetItemRectMin();
    auto&& [maxx, maxy]     = ImGui::GetItemRectMax();

    if(isItemActive && isMouseDragging)
    {
        if(mousex <= minx + 30)
        {
            frontCut = true;
            cutIndex = cl->get_id();
        }
        else if(mousex >= maxx - 30)
        {
            backCut  = true;
            cutIndex = cl->get_id();
        }
    }
    else if(frontCut && !isMouseDragging && cutIndex == cl->get_id())
    {
        frontCut = false;
        auto ts  = std::chrono::milliseconds(
             std::int64_t(timemax * (mousex - (xmin + canvasPos.x)) / (xmax - xmin)));
        auto offset = ts - cl->get_starts_at();
        command_handler::send({ "TimelineClipStartOffset",
                                new std::tuple<std::size_t, std::size_t, std::chrono::milliseconds>(i, j, offset) });
    }
    else if(backCut && !isMouseDragging && cutIndex == cl->get_id())
    {
        backCut = false;
        auto ts = std::chrono::milliseconds(
            std::int64_t(timemax * (mousex - (xmin + canvasPos.x)) / (xmax - xmin)));
        auto offset = (cl->get_starts_at() + cl->get_duration()) - ts;
        command_handler::send({ "TimelineClipEndOffset",
                                new std::tuple<std::size_t, std::size_t, std::chrono::milliseconds>(i, j, offset) });
    }
}
