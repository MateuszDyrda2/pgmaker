#pragma once

#include "pg_types.h"

namespace libpgmaker {
struct project_settings
{
    std::pair<std::uint32_t, std::uint32_t> size;
    std::size_t framerate;
};

} // namespace libpgmaker
