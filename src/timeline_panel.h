#pragma once

#include "panel.h"

#include <libpgmaker/timeline.h>

class timeline_panel : public panel
{
  public:
    static constexpr float channelHeight = 60;
    static constexpr float channelMargin = 10;
    static constexpr float timemax       = 20000;

  public:
    timeline_panel();
    ~timeline_panel();
    void draw() override;

  private:
    bool wasMoved;
    std::size_t movedIndex;

  private:
    void draw_tools(libpgmaker::timeline& tl);
    void draw_timeline(libpgmaker::timeline& tl);
    void draw_channel(libpgmaker::timeline& tl,
                      const std::unique_ptr<libpgmaker::channel>& ch,
                      float xmin, float xmax,
                      ImVec2 channelPos,
                      ImVec2 canvasPos,
                      ImDrawList* drawList);
    void draw_clip(libpgmaker::timeline& tl, const std::unique_ptr<libpgmaker::clip>& cl,
                   std::size_t i,
                   std::size_t j,
                   float xmin, float xmax,
                   ImVec2 channelPos,
                   ImVec2 canvasPos,
                   ImDrawList* drawList);
};