#pragma once

#include "window_base.h"
#include "NodeEditor/BlockEditor.h"

class cnode_editor : public window_base
{
  std::unique_ptr<BlockEditor> blockEditor;

  public:
    cnode_editor();
    ~cnode_editor();
    void draw() override;
};
