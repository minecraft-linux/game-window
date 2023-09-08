#include "window_manager_glfw.h"
#include "window_manager_eglut.h"
#include <stdexcept>

static bool ReadEnvFlag(const char* name, bool def = false) {
    auto val = getenv(name);
    if(!val) {
        return def;
    }
    std::string sval = val;
    return sval == "true" || sval == "1" || sval == "on";
}

GLFWFallbackEGLUTWindowManager::GLFWFallbackEGLUTWindowManager() {
    if(ReadEnvFlag("GAMEWINDOW_SYSTEM_EGLUT")) {
        manager = new EGLUTWindowManager();
    } else {
        try {
            manager = new GLFWWindowManager();
        } catch(...) {
            manager = new EGLUTWindowManager();
        }
    }
}

GameWindowManager::ProcAddrFunc GLFWFallbackEGLUTWindowManager::getProcAddrFunc() {
    return manager->getProcAddrFunc();
}

std::shared_ptr<GameWindow> GLFWFallbackEGLUTWindowManager::createWindow(const std::string& title, int width, int height,
                                                             GraphicsApi api) {
    return manager->createWindow(title, width, height, api);
}

void GLFWFallbackEGLUTWindowManager::addGamepadMappingFile(const std::string &path) {
    manager->addGamepadMappingFile(path);
}

void GLFWFallbackEGLUTWindowManager::addGamePadMapping(const std::string &content) {
    manager->addGamePadMapping(content);
}

// Define this window manager as the used one
std::shared_ptr<GameWindowManager> GameWindowManager::createManager() {
    return std::shared_ptr<GameWindowManager>(new GLFWFallbackEGLUTWindowManager());
}
