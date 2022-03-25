cmake_minimum_required(VERSION 3.21)

set(libpgmaker_VERSION ${pgmaker_VERSION})

include(FetchContent)
include(ExternalProject)


set(FFMPEG_DIRECTORY "C:/ffmpeg/" CACHE STRING "Where to find ffmpeg" FORCE)
if(WIN32)
        set(PC_SWSCALE_INCLUDE_DIRS ${FFMPEG_DIRECTORY}include)
        set(PC_AVUTIL_INCLUDE_DIRS ${FFMPEG_DIRECTORY}include)
        set(PC_AVFORMAT_INCLUDE_DIRS ${FFMPEG_DIRECTORY}include)
        set(PC_AVCODEC_INCLUDE_DIRS ${FFMPEG_DIRECTORY}include)

        set(PC_SWSCALE_LIBRARY_DIRS ${FFMPEG_DIRECTORY}lib)
        set(PC_AVUTIL_LIBRARY_DIRS ${FFMPEG_DIRECTORY}lib)
        set(PC_AVFORMAT_LIBRARY_DIRS ${FFMPEG_DIRECTORY}lib)
        set(PC_AVCODEC_LIBRARY_DIRS ${FFMPEG_DIRECTORY}lib)
endif()

find_package(FFmpeg COMPONENTS AVCODEC AVFORMAT AVUTIL SWSCALE REQUIRED)


find_package(glm)
if(NOT ${glm_FOUND})
	FetchContent_Declare(
		glm
		GIT_REPOSITORY https://github.com/g-truc/glm.git
	)
	FetchContent_MakeAvailable(glm)
endif()
find_package(OpenAL)
if(NOT ${OpenAL_FOUND})
	FetchContent_Declare(
		OpenAL
		GIT_REPOSITORY https://github.com/kcat/openal-soft.git
	)
	FetchContent_MakeAvailable(OpenAL)
endif()
add_subdirectory(lib/glad)

project(libpgmaker VERSION ${libpgmaker_VERSION} LANGUAGES CXX C)

set(FFMPEG_LIBS
	${AVFORMAT_LIBRARIES}
	${AVCODEC_LIBRARIES}
	${AVUTIL_LIBRARIES}
	${SWSCALE_LIBRARIES}
)
set(FFMPEG_INCLUDE
	${AVFORMAT_INCLUDE_DIRS}
	${AVCODEC_INCLUDE_DIRS}
	${AVUTILS_INCLUDE_DIRS}
	${SWSCALE_INCLUDE_DIRS}
)
find_package(Boost COMPONENTS thread system REQUIRED)

set(libpgmaker_SRC
	src/video.cpp
	src/video_reader.cpp
	src/textureGL.cpp
	src/channel.cpp
)
add_library(libpgmaker ${libpgmaker_SRC})

target_include_directories(libpgmaker 
    PUBLIC
	include/
	PRIVATE
	${FFMPEG_INCLUDE}
	${Boost_INCLUDE_DIRS}
)
if(WIN32)
    set(GLM_LIB glm::glm)
    set(OPENAL_LIB OpenAL)
else()
    set(GLM_LIB glm)
    set(OPENAL_LIB ${OPENAL_LIBRARY})
endif()
target_link_libraries(libpgmaker
	PUBLIC
	${FFMPEG_LIBS}
    ${OPENAL_LIB}
	glad
    ${GLM_LIB}
	${Boost_LIBRARIES}
)

set_property(TARGET libpgmaker PROPERTY CXX_STANDARD 17)
set_property(TARGET libpgmaker PROPERTY CXX_STANDARD_REQUIRED ON)

option(libpgmaker_BUILDSANDBOX "Build sandbox for library" OFF)

if(libpgmaker_BUILDSANDBOX)
	find_package(glfw3)
	message("Building sandbox")
	if(NOT glfw3_FOUND)
		FetchContent_Declare(
			glfw
			GIT_REPOSITORY https://github.com/glfw/glfw.git
		)
		FetchContent_MakeAvailable(glfw)
	endif()
	add_executable(libpgmaker_sandbox sandbox/main.cpp)
	target_link_libraries(libpgmaker_sandbox PUBLIC libpgmaker glfw)
endif()
