#pragma once

#include <memory>
#include <vector>

#include "window_base.h"

class cmain_menu : public window_base
{
  public:
    cmain_menu();
    ~cmain_menu();

    void draw() override;
};