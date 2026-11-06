cmake_minimum_required(VERSION 3.10)
project(LiDAR_getData)

add_subdirectory(sdk_dir)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE poelidar_sdk)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()