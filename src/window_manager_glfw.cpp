#include "window_manager_glfw.h"
#include "window_glfw.h"
#include <EGL/egl.h>
#include <eglut.h>

GLFWWindowManager::GLFWWindowManager() {
    eglutInit(0, nullptr); // the args aren't really required and are troublesome to pass with this system
}

GameWindowManager::ProcAddrFunc GLFWWindowManager::getProcAddrFunc() {
    return (GameWindowManager::ProcAddrFunc) eglGetProcAddress;
}

std::shared_ptr<GameWindow> GLFWWindowManager::createWindow(const std::string& title, int width, int height,
                                                             GraphicsApi api) {
    return std::shared_ptr<GameWindow>(new GLFWWindow(title, width, height, api));
}


// Define this window manager as the used one
std::shared_ptr<GameWindowManager> GameWindowManager::createManager() {
    return std::shared_ptr<GameWindowManager>(new GLFWWindowManager());
}