#pragma once

#include "panel.h"

#include <memory>
#include <vector>

#include <libpgmaker/video_reader.h>

class videos_panel : public panel
{
  public:
    videos_panel();
    ~videos_panel();
    void draw() override;
};
