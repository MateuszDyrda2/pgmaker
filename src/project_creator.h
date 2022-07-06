#pragma once

#include "application_base.h"

class project_creator : public application_base
{
  public:
    project_creator();
    ~project_creator();

    void update() override;

  private:
    float framerate;
    int width, height;
};
