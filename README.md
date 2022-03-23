# pgmaker
Video editing software written in C++

## Building the project:
- Clone the project   
	``` git clone https://github.com/MateuszDyrda2/pgmaker.git pgmaker ```
### Required Dependencies
- CMake >= 3.22
- Qt6
- glm
- FFmpeg
- OpenAL
### On Windows
- Download [CMake ](https://cmake.org/download/)and optionally add it to PATH
- Get Qt6 following instructions on https://www.qt.io/product/qt6
- Get [glm](https://github.com/g-truc/glm), [FFmpeg](https://www.ffmpeg.org/download.html), [OpenAL](https://www.openal.org/downloads/)
### On Linux
- Ubuntu:  
``` $ sudo apt-get install cmake libglm-dev ffmpeg libopenal-dev```  
Follow https://doc.qt.io/qt-6/linux.html to install qt6

- Arch:  
``` $ sudo pacman -S cmake glm ffmpeg openal qt6-base```
### Fallback
- If you don't want to install those dependencies globally you can just run cmake without installing them (excluding cmake and qt). If glm, FFmpeg or OpenAL are not found on the system they will be pulled from github locally.
### Build
- Windows:
	+ Run CMake using command line:  
	``` 
	mkdir build
	cd build
	cmake ..
	``` 
	open the Visual Studio solution and build the project
	+ or just open the CMakeLists.txt file using Visual Studio
- Linux:
	+ Run CMake:
	```
	$ mkdir build;cd build;cmake ..; make
	```