#include "ctimeline.h"

using namespace libpgmaker;
ctimeline::ctimeline()
{
}
ctimeline::~ctimeline()
{
}
void ctimeline::draw()
{
    auto proj = project_manager::get_current_project();
    auto& tl  = proj->get_timeline();
    ImGui::Begin("Timeline");
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

            std::size_t index = 0;
            for(const auto& ch : tl.get_channels())
            {
                ImGui::PushID(index);
                ImGui::InvisibleButton("", ImVec2(canvasSize.x - canvasPos.x, channelHeight));
                float spacing = index * (channelHeight + channelMargin);
                drawList->AddText(canvasPos, 0xFFFFFFFF, "Channel 1");
                drawList->AddRectFilled(
                    { xmin, canvasPos.y },
                    { xmax, canvasPos.y + channelHeight },
                    0xFF3D3837, 0);
                if(ImGui::BeginDragDropTarget())
                {
                    if(const auto payload = ImGui::AcceptDragDropPayload("demo"))
                    {
                        auto data = *static_cast<std::shared_ptr<video>*>(payload->Data);
                        tl.append_clip(0, data);
                    }
                    ImGui::EndDragDropTarget();
                }

                ImGui::SetItemAllowOverlap();
                std::size_t j        = 0;
                static bool wasMoved = false;
                for(const auto& cl : ch->get_clips())
                {
                    auto starts    = cl->get_starts_at().count();
                    auto ends      = starts + cl->get_duration().count();
                    auto stDiv     = starts / double(timemax);
                    auto edDiv     = ends / double(timemax);
                    float clipXMin = xmin + stDiv * (xmax - xmin),
                          clipXMax = xmin + edDiv * (xmax - xmin);
                    ImGui::SetCursorPos({ xmin, 0.f });
                    ImGui::InvisibleButton("aaa", { clipXMax - clipXMin, channelHeight });
                    if(ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                    {
                        wasMoved   = true;
                        auto delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                        clipXMin += delta.x;
                        clipXMax += delta.x;
                    }
                    else if(wasMoved)
                    {
                        wasMoved   = false;
                        auto delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                        // auto posdiff      = io.MouseDelta.x;
                        float newClipXmin = clipXMin + delta.x;
                        auto inDur        = ((newClipXmin - xmin) / (xmax - xmin)) * timemax;
                        tl.move_clip(index, j, std::chrono::milliseconds(std::int64_t(inDur)));
                    }
                    drawList->AddRectFilled(
                        { float(clipXMin), canvasPos.y },
                        { float(clipXMax), canvasPos.y + channelHeight },
                        0xFF0000AA, 2.5f);
                    ++j;
                }
                ImGui::PopID();
                ++index;
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
    }
    ImGui::EndChild();
    ImGui::End();
}
