cmake_minimum_required(VERSION 3.20)

project(projectorHdmi)

find_package(OpenCV REQUIRED)

file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

if(BUILD_DEVICE_SHARED)
    add_library(projectorHdmi SHARED)
    target_compile_definitions(projectorHdmi PUBLIC -DBUILD_SHARED_LIBS)
    target_compile_definitions(projectorHdmi PRIVATE -DDLL_EXPORTS)
else()
    add_library(projectorHdmi)
endif()

target_sources(projectorHdmi PRIVATE ${HEADERS} ${SOURCES})

target_include_directories(projectorHdmi
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    projectorHdmi
    PUBLIC
    ${OpenCV_LIBRARIES}
)
