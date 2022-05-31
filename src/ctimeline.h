#pragma once

#include "window_base.h"

#include <libpgmaker/timeline.h>

class ctimeline : public window_base
{
  public:
    ctimeline(libpgmaker::timeline& tl);
    ~ctimeline();
    void draw() override;

  private:
    libpgmaker::timeline& tl;
};