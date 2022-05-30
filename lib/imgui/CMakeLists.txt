project(imgui)

if(WIN32)
	add_library(imgui STATIC
		imgui_demo.cpp
		imgui.cpp
		imgui_draw.cpp
		imgui_impl_glfw.cpp
		imgui_impl_opengl3.cpp
		imgui_widgets.cpp
		imgui_tables.cpp
)
else()
	add_library(imgui
		imgui_demo.cpp
		imgui.cpp
		imgui_draw.cpp
		imgui_impl_glfw.cpp
		imgui_impl_opengl3.cpp
		imgui_widgets.cpp
		imgui_tables.cpp
)
endif()

target_include_directories(
	imgui 
	PUBLIC .
)
target_link_libraries(imgui PUBLIC glfw)