cmake_minimum_required(VERSION 3.26)
project(ImgOpts CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

# Требуется libfmt-dev
# Поиск и подключение fmt
find_package(fmt REQUIRED)

# Требуется libvips-dev
# Поиск и подключение libvips
find_package(PkgConfig REQUIRED)
pkg_check_modules(VIPS REQUIRED vips vips-cpp)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0)

# Требуется imagemagick libmagick++-dev
# Поиск и подключение ImageMagick
find_package(ImageMagick REQUIRED COMPONENTS Magick++)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_subdirectory(src)
