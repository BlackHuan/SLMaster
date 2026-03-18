cmake_minimum_required(VERSION 3.20)

project(dahengCamera)

set(DAHENG_SDK_PATH "C:/Program Files/Daheng Imaging/GalaxySDK/Development/VC SDK")

find_package(OpenCV REQUIRED)

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
source_group("HEADERS" FILES ${HEADERS})
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
source_group("SOURCES" FILES ${SOURCES})

if(BUILD_CAMERA_SHARED)
    add_library(dahengCamera SHARED)
    target_compile_definitions(dahengCamera PUBLIC -DBUILD_SHARED_LIBS)
    target_compile_definitions(dahengCamera PRIVATE -DDLL_EXPORTS)
else()
    add_library(dahengCamera)
endif()

target_sources(dahengCamera PUBLIC ${HEADERS} ${SOURCES})

target_include_directories(
    dahengCamera
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(
    dahengCamera
    PUBLIC
    ${DAHENG_SDK_PATH}/inc
)

target_link_libraries(
    dahengCamera
    PUBLIC
    ${OpenCV_LIBRARIES}
)

target_link_libraries(
    dahengCamera
    PRIVATE
    ${DAHENG_SDK_PATH}/lib/x64/GxIAPI.lib
    ${DAHENG_SDK_PATH}/lib/x64/DxImageProc.lib
)
