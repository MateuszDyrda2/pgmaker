#pragma once

#include "video.h"
#include <libpgmaker/pg_types.h>

#include <memory>
#include <string>

namespace libpgmaker {
class video_reader
{
  public:
    struct video_handle
    {
        video_handle& load_metadata();
        video_handle& load_thumbnail();
        std::shared_ptr<video> get();

        ~video_handle();

      private:
        AVFormatContext* pFormatContext;
        AVCodecContext* pCodecContext;
        int vsIndex;
        video_info info;
        std::unique_ptr<std::uint8_t[]> thumbnail;

      private:
        friend class video_reader;
        video_handle(AVFormatContext* ctx, const std::string& path);
    };

  public:
    static video_handle load_file(const std::string& path);
};
} // namespace libpgmaker
