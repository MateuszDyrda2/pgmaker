#pragma once

class application_base
{
  public:
    virtual ~application_base() = default;
    virtual void update()       = 0;
};
