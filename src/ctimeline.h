#pragma once

#include "window_base.h"

#include <libpgmaker/timeline.h>

class ctimeline : public window_base
{
  public:
    ctimeline();
    ~ctimeline();
    void draw() override;
};