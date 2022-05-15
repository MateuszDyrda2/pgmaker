#pragma once

#include "video.h"

#include <memory>
#include <string>

namespace libpgmaker {
class video_reader
{
  public:
    video_reader();
    ~video_reader();

    std::shared_ptr<video> load_file(const std::string& path);
};
} // namespace libpgmaker
