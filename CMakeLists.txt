cmake_minimum_required(VERSION 3.10)
project(Camera)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)
FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
  GIT_TAG        v3.0.0
)
FetchContent_MakeAvailable(cxxopts)

find_package(OpenCV REQUIRED)

include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${OpenCV_INCLUDE_DIRS}
)

set(SOURCES
  ${CMAKE_SOURCE_DIR}/src/double_buffered_camera.cpp
  ${CMAKE_SOURCE_DIR}/src/multi_buffered_camera.cpp
  ${CMAKE_SOURCE_DIR}/src/stereo_camera.cpp
)

add_executable(camera src/main.cpp ${SOURCES})
target_link_libraries(camera ${OpenCV_LIBS} cxxopts::cxxopts)