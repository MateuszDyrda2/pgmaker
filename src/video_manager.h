#pragma once

#include <libpgmaker/video.h>

#include <memory>
#include <unordered_map>

class video_manager : private std::unordered_map<std::string, std::shared_ptr<libpgmaker::video>>
{
  public:
    using base_type       = std::unordered_map<std::string, std::shared_ptr<libpgmaker::video>>;
    using size_type       = base_type::size_type;
    using value_type      = typename base_type::value_type;
    using reference       = typename base_type::reference;
    using iterator        = typename base_type::iterator;
    using const_iterator  = typename base_type::const_iterator;
    using const_reference = typename base_type::const_reference;

  public:
    using base_type::at;
    using base_type::size;
    using base_type::unordered_map;
    using base_type::operator[];
    using base_type::begin;
    using base_type::end;
    using base_type::find;
};
