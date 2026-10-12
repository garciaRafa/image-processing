cmake_minimum_required(VERSION 3.16)
project(image_processing CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(pdi
  src/main.cpp
  src/image.cpp
  src/convolution.cpp
  src/histogram.cpp
  src/median.cpp
  src/stb_impl.cpp)

target_include_directories(pdi PRIVATE src external)
target_compile_options(pdi PRIVATE -Wall -Wextra)

# Third-party code: do not report its warnings.
set_source_files_properties(src/stb_impl.cpp PROPERTIES COMPILE_OPTIONS "-w")
