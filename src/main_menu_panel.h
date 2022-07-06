#pragma once

#include <memory>
#include <vector>

#include "panel.h"

class main_menu_panel : public panel
{
  public:
    main_menu_panel();
    ~main_menu_panel();

    void draw() override;
};