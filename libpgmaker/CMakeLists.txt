cmake_minimum_required(VERSION 3.16)

set(libpgmaker_VERSION ${pgmaker_VERSION})


if(WIN32)	
	set(FFMPEG_DIRECTORY "C:/ffmpeg/" CACHE STRING "Where to find ffmpeg" FORCE)
	find_library(
		SWSCALE_LIBRARIES
		NAMES swscale libswscale
		PATHS ${FFMPEG_DIRECTORY}lib/
		REQUIRED
	)
	find_path(
		SWSCALE_INCLUDE_DIRS
		NAMES swscale.h 
		PATHS ${FFMPEG_DIRECTORY}include/
		PATH_SUFFIXES libswscale
		REQUIRED
	)

	find_library(
		AVUTIL_LIBRARIES
		NAMES avutil libavutil
		PATHS ${FFMPEG_DIRECTORY}lib/
		REQUIRED
	)
	find_path(
		AVUTIL_INCLUDE_DIRS
		NAMES avutil.h
		PATHS ${FFMPEG_DIRECTORY}include/
		PATH_SUFFIXES libavutil
		REQUIRED
	)

	find_library(
		AVFORMAT_LIBRARIES
		NAMES avformat libavformat
		PATHS ${FFMPEG_DIRECTORY}lib/
		REQUIRED
	)
	find_path(
		AVFORMAT_INCLUDE_DIRS
		NAMES avformat.h
		PATHS ${FFMPEG_DIRECTORY}include/
		PATH_SUFFIXES libavformat
		REQUIRED
	)

	find_library(
		AVCODEC_LIBRARIES
		NAMES avcodec libavcodec
		PATHS ${FFMPEG_DIRECTORY}lib/
		REQUIRED
	)
	find_path(
		AVCODEC_INCLUDE_DIRS
		NAMES avcodec.h
		PATHS ${FFMPEG_DIRECTORY}include/
		PATH_SUFFIXES libavcodec
		REQUIRED
	)

	find_library(
		SWRESAMPLE_LIBRARIES
		NAMES swresample libswresample
		PATHS ${FFMPEG_DIRECTORY}lib/
		REQUIRED
	)
	find_path(
		SWRESAPLE_INCLUDE_DIRS
		NAMES swresample.h
		PATHS ${FFMPEG_DIRECTORY}include/
		PATH_SUFFIXES libswresample
		REQUIRED
	)
	message("Finding FFMPEG manually")
	set(BUILD_SHARED_LIBS OFF)
else()
	find_package(FFmpeg COMPONENTS AVCODEC AVFORMAT AVUTIL SWSCALE SWRESAMPLE)
endif()


add_subdirectory(lib/glad)
add_subdirectory(lib/glm)
add_subdirectory(lib/portaudio)

project(libpgmaker VERSION ${libpgmaker_VERSION} LANGUAGES CXX C)

set(FFMPEG_LIBS
	${AVFORMAT_LIBRARIES}
	${AVCODEC_LIBRARIES}
	${AVUTIL_LIBRARIES}
	${SWSCALE_LIBRARIES}
	${SWRESAMPLE_LIBRARIES}
)
set(FFMPEG_INCLUDE
	"${FFMPEG_DIRECTORY}/include/"
)

include(FetchContent)
FetchContent_Declare(
	readerwriterqueue
	GIT_REPOSITORY		https://github.com/cameron314/readerwriterqueue 
	GIT_TAG				master
)
FetchContent_MakeAvailable(readerwriterqueue)

set(libpgmaker_SRC
	src/video_reader.cpp
	src/textureGL.cpp
	src/clip.cpp
	src/channel.cpp
	src/timeline.cpp
	src/video.cpp
	src/preview.cpp
)
add_library(libpgmaker STATIC ${libpgmaker_SRC})

target_include_directories(libpgmaker 
    PUBLIC
	include/
	PUBLIC
	${FFMPEG_INCLUDE}
)
if(WIN32)
    set(GLM_LIB glm::glm)
else()
    set(GLM_LIB glm)
endif()


target_link_libraries(libpgmaker
	PUBLIC
	${FFMPEG_LIBS}
	PortAudio
	glad
    ${GLM_LIB}
	readerwriterqueue
)


set_property(TARGET libpgmaker PROPERTY CXX_STANDARD 17)
set_property(TARGET libpgmaker PROPERTY CXX_STANDARD_REQUIRED ON)

option(libpgmaker_BUILDSANDBOX "Build sandbox for library" OFF)

if(libpgmaker_BUILDSANDBOX)
	find_package(glfw3 REQUIRED)
	message("Building sandbox")
	add_executable(libpgmaker_sandbox sandbox/main.cpp)
	target_link_libraries(libpgmaker_sandbox PUBLIC libpgmaker glfw)
endif()

if(WIN32)

#	add_custom_command(TARGET libpgmaker POST_BUILD
#		COMMAND ${CMAKE_COMMAND} -E 
#			copy ${FFMPEG_DIRECTORY}/bin/avcodec-59.dll 
#			${FFMPEG_DIRECTORY}/bin/avformat-59.dll
#			${FFMPEG_DIRECTORY}/bin/avutil-57.dll
#			${FFMPEG_DIRECTORY}/bin/swresample-4.dll 
#			${FFMPEG_DIRECTORY}/bin/swscale-6.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
#	)
	file(GLOB FFMPEG_DDLS
	"${FFMPEG_DIRECTORY}/bin/*.dll"
	)
	add_custom_command(TARGET libpgmaker POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
			copy ${FFMPEG_DDLS}
			${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	)
endif()
