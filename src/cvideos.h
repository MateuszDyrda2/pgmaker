#pragma once

#include "window_base.h"

#include <memory>
#include <vector>

#include <libpgmaker/video_reader.h>

class cvideos : public window_base
{
  public:
    cvideos();
    ~cvideos();
    void draw() override;
};
