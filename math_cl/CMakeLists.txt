cmake_minimum_required(VERSION 3.16)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ПЕРВОЕ И САМОЕ ВАЖНОЕ: Находим OpenCL
find_package(OpenCL REQUIRED)

if(OpenCL_FOUND)
    message(STATUS "OpenCL found:")
    message(STATUS "  Includes: ${OpenCL_INCLUDE_DIRS}")
    message(STATUS "  Libraries: ${OpenCL_LIBRARIES}")
else()
    message(FATAL_ERROR "OpenCL not found! Install opencl-headers and ocl-icd-opencl-dev")
endif()

# Создаем библиотеку
add_library(ffilter_cl SHARED fastFilter_cl.cpp fastFilter_cl.h)

# Создаем исполняемый файл
add_executable(ff_cl main.cpp)

# Связываем библиотеку с исполняемым файлом
target_link_libraries(ff_cl PRIVATE ffilter_cl)

# Подключаем OpenCL к БИБЛИОТЕКЕ
target_include_directories(ffilter_cl PRIVATE ${OpenCL_INCLUDE_DIRS})
target_link_libraries(ffilter_cl PRIVATE ${OpenCL_LIBRARIES})

# Для ff_cl тоже нужно OpenCL, если он использует OpenCL напрямую
target_include_directories(ff_cl PRIVATE ${OpenCL_INCLUDE_DIRS})
target_link_libraries(ff_cl PRIVATE ${OpenCL_LIBRARIES})
# Не нужно линковать ff_cl с OpenCL, если он использует только ffilter_cl

# Для Linux добавляем дополнительные библиотеки
if(UNIX AND NOT APPLE)
    target_link_libraries(ffilter_cl PRIVATE pthread dl)
endif()

# Для macOS
if(APPLE)
    # В macOS OpenCL встроен в систему
    find_library(OPENCL_LIB OpenCL)
    if(OPENCL_LIB)
        target_link_libraries(ffilter_cl PRIVATE ${OPENCL_LIB})
    endif()
endif()

configure_file(covolution.cl ${CMAKE_CURRENT_BINARY_DIR}/covolution.cl COPYONLY)
configure_file(add.cl ${CMAKE_CURRENT_BINARY_DIR}/add.cl COPYONLY)

configure_file(sinus.bin ${CMAKE_CURRENT_BINARY_DIR}/sinus.bin COPYONLY)
configure_file(filter.bin ${CMAKE_CURRENT_BINARY_DIR}/filter.bin COPYONLY)