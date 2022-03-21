cmake_minimum_required(VERSION 3.22)

set(libpgmaker_VERSION ${pgmaker_VERSION})

find_package(FFmpeg COMPONENTS AVCODEC AVDEVICE AVFORMAT AVUTIL SWSCALE REQUIRED)
find_package(glm REQUIRED)
find_package(OpenAL REQUIRED)
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

set(libpgmaker_SRC
	src/video.cpp
	src/video_reader.cpp
)
add_library(libpgmaker ${libpgmaker_SRC})

target_include_directories(libpgmaker 
	PUBLIC 
	include/
	PRIVATE
	${FFMPEG_INCLUDE}
)
target_link_libraries(libpgmaker
	PUBLIC
	${FFMPEG_LIBS}
	${OPENAL_LIBRARY}
	glad
	glm
)

set_property(TARGET libpgmaker PROPERTY CXX_STANDARD 17)
set_property(TARGET libpgmaker PROPERTY CXX_STANDARD_REQUIRED ON)

option(libpgmaker_BUILDSANDBOX "Build sandbox for library" ON)

if(libpgmaker_BUILDSANDBOX)
	message("Building sandbox")
	find_package(glfw3 REQUIRED)
	add_executable(libpgmaker_sandbox sandbox/main.cpp)
	target_link_libraries(libpgmaker_sandbox PUBLIC libpgmaker glfw)
endif()
