project(Glad)

if(WIN32)
	add_library(glad STATIC src/glad.c)
else()
	add_library(glad src/glad.c)
endif()
target_include_directories(glad PUBLIC include/)