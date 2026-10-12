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
  src/hfe.cpp
  src/stb_impl.cpp)

# FFTW3 (Question 5). On Ubuntu: sudo apt install libfftw3-dev
find_path(FFTW3_INCLUDE_DIR fftw3.h REQUIRED)
find_library(FFTW3_LIBRARY fftw3 REQUIRED)

target_include_directories(pdi PRIVATE src external ${FFTW3_INCLUDE_DIR})
target_link_libraries(pdi PRIVATE ${FFTW3_LIBRARY} m)
target_compile_options(pdi PRIVATE -Wall -Wextra)

# Third-party code: do not report its warnings.
set_source_files_properties(src/stb_impl.cpp PROPERTIES COMPILE_OPTIONS "-w")
