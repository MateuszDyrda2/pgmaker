#pragma once

#include "window_base.h"

#include <memory>
#include <vector>

#include <libpgmaker/video_reader.h>

class cvideos : public window_base
{
  public:
    cvideos(std::vector<std::shared_ptr<libpgmaker::video>>& videos);
    ~cvideos();
    void draw() override;

  private:
    std::vector<std::shared_ptr<libpgmaker::video>>& videos;
};
