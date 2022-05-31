#pragma once

#include <glad/glad.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_opengl3_loader.h>
#include <imgui_internal.h>

class window_base
{
  public:
    virtual ~window_base() = default;
    virtual void draw()    = 0;
};
