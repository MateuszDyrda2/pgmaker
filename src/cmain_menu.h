#pragma once

#include <libpgmaker/video_reader.h>
#include <libpgmaker/timeline.h>
#include <memory>
#include <vector>

#include "window_base.h"

class cmain_menu : public window_base
{
  public:
    cmain_menu(std::vector<std::shared_ptr<libpgmaker::video>>& videos, libpgmaker::timeline& tl);
    ~cmain_menu();

    void draw() override;

  private:
    std::vector<std::shared_ptr<libpgmaker::video>>& videos;
	libpgmaker::timeline& tl;
};