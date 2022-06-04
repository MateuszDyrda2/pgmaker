#pragma once

#include "panel.h"
#include <libpgmaker/clip.h>

class properties_panel : public panel
{
  public:
    properties_panel();
    ~properties_panel();

    void draw() override;

  private:
    libpgmaker::clip_info info;
};
