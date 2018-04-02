cmake_minimum_required (VERSION 2.6)

project(eglut LANGUAGES CXX)

find_package(PNG REQUIRED)

set(GAMEWINDOW_SYSTEM_DEFAULT EGLUT)
if (APPLE)
    set(GAMEWINDOW_SYSTEM_DEFAULT GLFW)
endif()

set(GAMEWINDOW_SYSTEM ${GAMEWINDOW_SYSTEM_DEFAULT} CACHE STRING "The implementation to use for windows - EGLUT or GLFW")

set(GAMEWINDOW_SOURCES include/game_window.h)
set(GAMEWINDOW_SOURCES_EGLUT src/window_eglut.h src/window_eglut.cpp)
set(GAMEWINDOW_SOURCES_GLFW src/window_glfw.h src/window_glfw.cpp)
set(GAMEWINDOW_LIBRARIES )

if (GAMEWINDOW_SYSTEM STREQUAL "EGLUT")
    set(GAMEWINDOW_SOURCES ${GAMEWINDOW_SOURCES} ${GAMEWINDOW_SOURCES_EGLUT})
    set(GAMEWINDOW_LIBRARIES ${GAMEWINDOW_LIBRARIES} eglut)
elseif (GAMEWINDOW_SYSTEM STREQUAL "GLFW")
    set(GAMEWINDOW_SOURCES ${GAMEWINDOW_SOURCES} ${GAMEWINDOW_SOURCES_GLFW})
    set(GAMEWINDOW_LIBRARIES ${GAMEWINDOW_LIBRARIES} glfw3)
endif()

add_library(gamewindow ${GAMEWINDOW_SOURCES})
target_include_directories(gamewindow PUBLIC include/)
target_link_libraries(gamewindow PUBLIC ${GAMEWINDOW_LIBRARIES})