cmake_minimum_required (VERSION 2.6)

project(eglut LANGUAGES CXX)

find_package(PNG REQUIRED)

set(GAMEWINDOW_SYSTEM_DEFAULT EGLUT)
if (APPLE)
    set(GAMEWINDOW_SYSTEM_DEFAULT GLFW)
endif()

set(GAMEWINDOW_SYSTEM ${GAMEWINDOW_SYSTEM_DEFAULT} CACHE STRING "The implementation to use for windows - EGLUT or GLFW")

set(GAMEWINDOW_SOURCES include/game_window.h include/game_window_manager.h src/game_window_manager.cpp)
set(GAMEWINDOW_SOURCES_EGLUT src/window_eglut.h src/window_eglut.cpp src/window_manager_eglut.cpp src/window_manager_eglut.h)
set(GAMEWINDOW_SOURCES_GLFW src/window_glfw.h src/window_glfw.cpp src/window_manager_glfw.cpp src/window_manager_glfw.h)

add_library(gamewindow ${GAMEWINDOW_SOURCES})
target_include_directories(gamewindow PUBLIC include/)

if (GAMEWINDOW_SYSTEM STREQUAL "EGLUT")
    target_sources(gamewindow PRIVATE ${GAMEWINDOW_SOURCES_EGLUT})
    target_link_libraries(gamewindow PUBLIC eglut)
elseif (GAMEWINDOW_SYSTEM STREQUAL "GLFW")
    target_sources(gamewindow PRIVATE ${GAMEWINDOW_SOURCES_GLFW})
    target_link_libraries(gamewindow PUBLIC glfw3)
endif()