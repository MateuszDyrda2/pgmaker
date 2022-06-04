#pragma once

#include "pg_types.h"

namespace libpgmaker {
struct project_settings
{
    inline static std::pair<std::uint32_t, std::uint32_t> size;
    inline static std::size_t framerate;
};

} // namespace libpgmaker
