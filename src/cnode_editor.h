#pragma once

#include "window_base.h"

class cnode_editor : public window_base
{
  public:
    cnode_editor();
    ~cnode_editor();
    void draw() override;
};
