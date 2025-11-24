cmake_minimum_required(VERSION 3.16)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ffilter SHARED fastFilter.cpp fastFilter.h)

target_include_directories(ffilter
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}  # This allows #include "fastFilter.h"
)

# Add AVX flags to both compile and link commands
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ffilter  PRIVATE -mavx -mavx2)
    target_link_options(ffilter  PRIVATE -mavx -mavx2)  # Important for linking!
elseif(MSVC)
    target_compile_options(ffilter  PRIVATE /arch:AVX2)
endif()



# Link Boost libraries - only link components that have separate libraries
# target_link_libraries(ffilter PRIVATE Boost::random)