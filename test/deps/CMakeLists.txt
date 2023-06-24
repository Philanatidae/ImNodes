include(CPM)

macro(ff_AddRepoDepNoBuild dep_name dep_version dep_type dep_repo dep_tag)
    CPMAddPackage(
        NAME ${dep_name}
        VERSION ${dep_version}
        ${dep_type}_REPOSITORY ${dep_repo}
        ${dep_type}_TAG ${dep_tag}
        DOWNLOAD_ONLY YES
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${dep_name}
    )
    if(NOT ${dep_name}_ADDED)
        message(FATAL_ERROR "Could not add ${dep_name}.")
    endif()
endmacro()

#########################################
# GLFW
#########################################
ff_AddRepoDepNoBuild(glfw 3.3.8 GIT "https://github.com/glfw/glfw" 7482de6)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
if(WIN32)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
endif()
add_subdirectory(glfw)

###############################################
# libepoxy
###############################################
ff_AddRepoDepNoBuild(libepoxy-cmake 1.5.10-rev2 GIT "https://github.com/Philanatidae/libepoxy-cmake" a375152)

set(BUILD_SHARED_LIBS NO)
if(NOT ANDROID AND NOT APPLE AND NOT WIN32)
    # Linux
    set(EPOXY_ENABLE_GLX Yes)
    set(EPOXY_ENABLE_X11 Yes)
endif()
if(ANDROID)
    set(EPOXY_ENABLE_EGL Yes)
endif()
add_subdirectory(libepoxy-cmake)

#########################################
# imgui
#########################################
ff_AddRepoDepNoBuild(imgui 1.89.6 GIT "https://github.com/ocornut/imgui" 5319d1c)

add_library(imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/misc/cpp/imgui_stdlib.cpp
)
target_include_directories(imgui SYSTEM PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui>)
target_include_directories(imgui SYSTEM PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui/misc/cpp>)

