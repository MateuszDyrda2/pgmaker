cmake_minimum_required(VERSION 3.16)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(pgmaker_VERSION_MAJOR 0)
set(pgmaker_VERSION_MINOR 1)
set(pgmaker_VERSION_PATCH 0)
set(pgmaker_VERSION ${pgmaker_VERSION_MAJOR}.${pgmaker_VERSION_MINOR}.${pgmaker_VERSION_PATCH})

project(layout VERSION ${pgmaker_VERSION} LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

find_package(Qt6 6.2 COMPONENTS Quick REQUIRED)

qt_add_executable(applayout
    src/main.cpp
    src/qtpreview.h
    src/qtrenderer.h
    src/qtpreview.cpp
    src/qtrenderer.cpp
    src/activevideos.h
    src/activevideos.cpp
    src/qttimeline.h
    src/qttimeline.cpp
    src/fbitem.h
    src/fbitem.cpp
)

qt_add_qml_module(applayout
    URI layout
    VERSION 1.0
    QML_FILES 
	"src/qml/main.qml"
    "src/qml/MyButton.qml"
)

set_target_properties(applayout PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

add_subdirectory(libpgmaker)
target_compile_definitions(applayout
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(applayout
    PRIVATE Qt6::Quick
	PUBLIC libpgmaker)

target_include_directories(libpgmaker PUBLIC src)
