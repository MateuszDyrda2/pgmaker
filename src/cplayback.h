#pragma once

#include "window_base.h"

#include <libpgmaker/timeline.h>

#include <utility>

class cplayback : public window_base
{
  public:
    cplayback(libpgmaker::timeline& tl);
    ~cplayback();
    void draw() override;

  private:
    libpgmaker::timeline& tl;
    unsigned int texture;
    std::pair<std::uint32_t, std::uint32_t> textureSize{ 1080, 720 };
};
