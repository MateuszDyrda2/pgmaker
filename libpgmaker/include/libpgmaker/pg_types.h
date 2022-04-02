#pragma once

#include <chrono>
#include <cstdint>
#include <memory>

namespace libpgmaker {
struct resolution
{
    std::uint32_t width, height;
};
struct frame
{
    resolution size;
    std::unique_ptr<std::uint8_t[]> data;
    std::chrono::milliseconds timestamp;
};

} // namespace libpgmaker
