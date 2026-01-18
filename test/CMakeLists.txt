cmake_minimum_required(VERSION 3.16)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/math)

add_executable(main_app main.cpp)

# Link the main executable to the three libraries
target_link_libraries(main_app  PRIVATE  ffilter)

# Include current directory for GUI headers
target_include_directories(main_app PRIVATE 
    ${CMAKE_SOURCE_DIR}/math
)

configure_file(sinus.bin ${CMAKE_CURRENT_BINARY_DIR}/sinus.bin COPYONLY)
configure_file(filter.bin ${CMAKE_CURRENT_BINARY_DIR}/filter.bin COPYONLY)