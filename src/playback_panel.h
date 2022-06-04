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
};
