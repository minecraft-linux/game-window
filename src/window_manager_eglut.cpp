#include "window_manager_eglut.h"
#include "window_eglut.h"
#include <EGL/egl.h>
#include <eglut.h>

EGLUTWindowManager::EGLUTWindowManager() {
    eglutInit(0, nullptr); // the args aren't really required and are troublesome to pass with this system
}

GameWindowManager::ProcAddrFunc EGLUTWindowManager::getProcAddrFunc() {
    return (GameWindowManager::ProcAddrFunc) eglGetProcAddress;
}

std::shared_ptr<GameWindow> EGLUTWindowManager::createWindow(const std::string& title, int width, int height,
                                                             GraphicsApi api) {
    return std::shared_ptr<GameWindow>(new EGLUTWindow(title, width, height, api));
}


// Define this window manager as the used one
std::shared_ptr<GameWindowManager> GameWindowManager::createManager() {
    return std::shared_ptr<GameWindowManager>(new EGLUTWindowManager());
}