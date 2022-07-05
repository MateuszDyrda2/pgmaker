#include "node_editor_panel.h"

#include "events.h"
#include "project.h"
#include <algorithm>

node_editor_panel::node_editor_panel()
{
    auto proj           = project_manager::get_current_project();
    std::size_t nbClips = 0;
    if(proj)
    {
        const auto& tl = proj->get_timeline();
        for(const auto& ch : tl.get_channels())
        {
            for(const auto& cl : ch->get_clips())
            {
                editor.add_connection(ch->get_id(), cl->get_id());
            }
        }
    }
    event_handler::subscribe(
        "TimelineClipAppended",
        std::function<void(size_t, size_t)>([this](size_t channelIdx, size_t clipHandle) {
            this->editor.add_connection(channelIdx, clipHandle);
        }));
}
node_editor_panel::~node_editor_panel()
{
}
void node_editor_panel::draw()
{
    if(ImGui::Begin("NodeEditor"))
    {
        editor.draw();
    }
    ImGui::End();
}