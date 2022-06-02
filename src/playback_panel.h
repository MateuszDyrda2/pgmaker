#pragma once

#include "panel.h"

#include <libpgmaker/timeline.h>

#include <utility>

class playback_panel : public panel
{
  public:
    playback_panel();
    ~playback_panel();
    void draw() override;

  private:
    unsigned int texture;
    std::pair<std::uint32_t, std::uint32_t> textureSize{ 1080, 720 };
};
