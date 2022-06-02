cmake_minimum_required(VERSION 3.16)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(pgmaker_VERSION_MAJOR 0)
set(pgmaker_VERSION_MINOR 1)
set(pgmaker_VERSION_PATCH 0)
set(pgmaker_VERSION ${pgmaker_VERSION_MAJOR}.${pgmaker_VERSION_MINOR}.${pgmaker_VERSION_PATCH})

if(WIN32)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

project(pgmaker VERSION ${pgmaker_VERSION} LANGUAGES CXX)

add_subdirectory(lib/glfw)
add_subdirectory(libpgmaker)
add_subdirectory(lib/imgui)
add_subdirectory(lib/nativefiledialog)
include(FetchContent)

set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_declare(
	json
	GIT_REPOSITORY		https://github.com/nlohmann/json.git 
	GIT_TAG 			master
)
FetchContent_MakeAvailable(json)


add_executable(pgmaker 
	src/main.cpp
	src/main_menu_panel.cpp
	src/videos_panel.cpp
	src/properties_panel.cpp
	src/timeline_panel.cpp
	src/playback_panel.cpp
	src/node_editor_panel.cpp
	src/project.cpp
	src/main_application.cpp
	src/welcome_screen.cpp
)

target_link_libraries(pgmaker
	PUBLIC 
	glad
	glfw ${GLFW_LIBRARIES}
	nfd
	libpgmaker
	imgui
	nlohmann_json::nlohmann_json
)

target_include_directories(
	pgmaker 
	PUBLIC src
)
