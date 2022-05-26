cmake_minimum_required(VERSION 3.16)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(pgmaker_VERSION_MAJOR 0)
set(pgmaker_VERSION_MINOR 1)
set(pgmaker_VERSION_PATCH 0)
set(pgmaker_VERSION ${pgmaker_VERSION_MAJOR}.${pgmaker_VERSION_MINOR}.${pgmaker_VERSION_PATCH})

project(pgmaker VERSION ${pgmaker_VERSION} LANGUAGES CXX)

find_package(glfw3 REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

add_subdirectory(libpgmaker)
add_subdirectory(lib/imgui)
add_subdirectory(lib/nativefiledialog)

add_executable(pgmaker src/main.cpp)

target_compile_definitions(pgmaker
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(pgmaker
	PUBLIC libpgmaker
	PUBLIC imgui
	PUBLIC glfw
	PUBLIC nfd)

target_include_directories(pgmaker PUBLIC src)
