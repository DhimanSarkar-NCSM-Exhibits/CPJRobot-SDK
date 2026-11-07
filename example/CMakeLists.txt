cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER   gcc)
set(CMAKE_CXX_COMPILER g++)

project(LiDAR_getData CXX C)

add_subdirectory("../C_CPP_SDK" build)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE poelidar_sdk)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()