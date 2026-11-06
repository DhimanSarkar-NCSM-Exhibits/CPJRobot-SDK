cmake_minimum_required(VERSION 3.10)
project(poelidar_sdk_lib)

add_library(poelidar_sdk STATIC src/poelidar_impl.cpp)

target_include_directories(poelidar_sdk PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(UNIX)
    target_sources(poelidar_sdk PRIVATE src/net/linux/net.cpp)
    target_include_directories(poelidar_sdk PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/net/linux)
elseif(WIN32)
    target_sources(poelidar_sdk PRIVATE src/net/win32/net.cpp)
    target_include_directories(poelidar_sdk PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/net/win32)
    target_link_libraries(poelidar_sdk PUBLIC ws2_32)
else()
    message(FATAL_ERROR "Only Linux or Win32 are supported.")
endif()
