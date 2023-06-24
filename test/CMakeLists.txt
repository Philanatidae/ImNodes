cmake_minimum_required(VERSION 3.20)

project(imnodes-test)

set(CMAKE_CXX_STANDARD 17 CACHE INTERNAL "CMAKE_CXX_STANDARD")
set(CMAKE_C_STANDARD 99 CACHE INTERNAL "CMAKE_C_STANDARD")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(deps)

add_library(imnodes STATIC
    ../ImNodes.cpp
    ../ImNodesEz.cpp
)
target_link_libraries(imnodes PUBLIC imgui)
target_include_directories(imnodes SYSTEM PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

add_executable(main
    src/main.cpp
)

if(APPLE)
endif()

target_link_libraries(main PUBLIC
    epoxy
    glfw
    imgui
    imnodes
)

