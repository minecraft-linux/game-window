cmake_minimum_required (VERSION 2.6)

project(eglut LANGUAGES CXX)

include(BuildSettings.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

set(GAMEWINDOW_SOURCES include/game_window.h include/game_window_manager.h src/game_window_manager.cpp src/game_window_error_handler.cpp src/joystick_manager.cpp)
set(GAMEWINDOW_SOURCES_LINUX_GAMEPAD src/joystick_manager_linux_gamepad.cpp src/joystick_manager_linux_gamepad.h src/window_with_linux_gamepad.cpp src/window_with_linux_gamepad.h)
set(GAMEWINDOW_SOURCES_EGLUT src/window_eglut.h src/window_eglut.cpp src/window_manager_eglut.cpp src/window_manager_eglut.h)
set(GAMEWINDOW_SOURCES_GLFW src/window_glfw.h src/window_glfw.cpp src/window_manager_glfw.cpp src/window_manager_glfw.h src/joystick_manager_glfw.cpp src/joystick_manager_glfw.h)
set(GAMEWINDOW_SOURCES_SDL2 src/window_sdl2.h src/window_sdl2.cpp src/window_manager_sdl2.cpp src/window_manager_sdl2.h)

add_library(gamewindow ${GAMEWINDOW_SOURCES})
target_include_directories(gamewindow PUBLIC include/)

if (GAMEWINDOW_SYSTEM STREQUAL "EGLUT")
    target_sources(gamewindow PRIVATE ${GAMEWINDOW_SOURCES_EGLUT} ${GAMEWINDOW_SOURCES_LINUX_GAMEPAD})
    target_link_libraries(gamewindow PUBLIC eglut linux-gamepad)
elseif (GAMEWINDOW_SYSTEM STREQUAL "GLFW")
    target_sources(gamewindow PRIVATE ${GAMEWINDOW_SOURCES_GLFW})
    target_link_libraries(gamewindow PUBLIC glfw3)
elseif (GAMEWINDOW_SYSTEM STREQUAL "SDL2")
    find_package(EGL REQUIRED)
    message(STATUS "Found EGL")
    find_package(SDL2 REQUIRED)
    message(STATUS "Found SDL2: -I" ${SDL2_INCLUDE_DIRS} " " ${SDL2_LIBRARIES})
    target_sources(gamewindow PRIVATE ${GAMEWINDOW_SOURCES_SDL2})
    target_link_libraries(gamewindow ${EGL_LIBRARIES} ${SDL2_LIBRARIES})
endif()
