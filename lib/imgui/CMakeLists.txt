project(imgui)

add_library(imgui 
	imgui_demo.cpp
	imgui.cpp
	imgui_draw.cpp
	imgui_impl_glfw.cpp
	imgui_impl_opengl3.cpp
	imgui_widgets.cpp
	imgui_tables.cpp
)

target_include_directories(imgui PUBLIC .)