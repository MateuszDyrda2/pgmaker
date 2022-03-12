cmake_minimum_required(VERSION 3.22)

set(pgmaker_VERSION_MAJOR 0)
set(pgmaker_VERSION_MINOR 1)
set(pgmaker_VERSION_PATCH 0)

set(pgmaker_VERSION ${pgmaker_VERSION_MAJOR}.${pgmaker_VERSION_MINOR}.${pgmaker_VERSION_PATCH})

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
project(PGMaker VERSION ${pgmaker_VERSION} LANGUAGES CXX C)

# QT SETUP
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.2 COMPONENTS Quick REQUIRED)

add_subdirectory(libpgmaker)

set(pgmaker_SRC
	src/main.cpp
        pgmaker.qrc
)

qt_add_executable(pgmaker ${pgmaker_SRC})
qt_add_qml_module(pgmaker URI PGMaker VERSION ${pgmaker_VERSION} QML_FILES src/qml/MainWindow.qml)

target_link_libraries(pgmaker PRIVATE libpgmaker Qt6::Quick)

set_property(TARGET pgmaker PROPERTY CXX_STANDARD 17)
set_property(TARGET pgmaker PROPERTY CXX_STANDARD_REQUIRED ON)
target_compile_definitions(pgmaker
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
