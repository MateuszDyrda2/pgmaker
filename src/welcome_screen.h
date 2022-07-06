#pragma once

#include "application_base.h"

class welcome_screen : public application_base
{
  public:
    welcome_screen();
    ~welcome_screen();

    void update() override;
};