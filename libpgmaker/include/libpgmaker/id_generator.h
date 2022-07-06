#pragma once

class id_generator
{
  public:
    inline static std::size_t get_next()
    {
        static std::size_t id = 0;
        return id++;
    }
};
