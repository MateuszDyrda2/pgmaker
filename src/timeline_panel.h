#pragma once

#include "panel.h"

#include <libpgmaker/timeline.h>

class timeline_panel : public panel
{
  public:
    timeline_panel();
    ~timeline_panel();
    void draw() override;
};