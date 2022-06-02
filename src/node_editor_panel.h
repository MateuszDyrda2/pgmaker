#pragma once

#include "NodeEditor/BlockEditor.h"
#include "panel.h"

class node_editor_panel : public panel
{
  public:
    node_editor_panel();
    ~node_editor_panel();
    void draw() override;

  private:
    std::unique_ptr<BlockEditor> blockEditor;
};
