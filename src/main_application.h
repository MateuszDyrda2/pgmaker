#pragma once

#include "application_base.h"

#include "main_menu_panel.h"
#include "node_editor_panel.h"
#include "playback_panel.h"
#include "properties_panel.h"
#include "timeline_panel.h"
#include "videos_panel.h"

class main_application : public application_base
{
  public:
    main_application();
    ~main_application();
    void update() override;

  private:
    main_menu_panel mainMenu;
    node_editor_panel nodeEditor;
    playback_panel playback;
    properties_panel properties;
    timeline_panel timeline;
    videos_panel videos;

  private:
    void initialize_layout(ImGuiID dockspaceId);
};
