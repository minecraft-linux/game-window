#include "window_manager_glfw.h"
#include "window_glfw.h"
#include "joystick_manager_glfw.h"
#include <stdexcept>

GLFWWindowManager::GLFWWindowManager() {
    // To create a default mapping for not mapped Gamepads
    // to avoid subtracting heads from buttons again
    glfwInitHint(GLFW_JOYSTICK_HAT_BUTTONS, 0);
    if (glfwInit() != GLFW_TRUE)
        throw std::runtime_error("glfwInit error");
    GLFWJoystickManager::init();
}

GameWindowManager::ProcAddrFunc GLFWWindowManager::getProcAddrFunc() {
    return (GameWindowManager::ProcAddrFunc) glfwGetProcAddress;
}

std::shared_ptr<GameWindow> GLFWWindowManager::createWindow(const std::string& title, int width, int height,
                                                             GraphicsApi api) {
    return std::shared_ptr<GameWindow>(new GLFWGameWindow(title, width, height, api));
}

void GLFWWindowManager::addGamepadMappingFile(const std::string &path) {
    GLFWJoystickManager::loadMappingsFromFile(path);
}

void GLFWWindowManager::addGamePadMapping(const std::string &content) {
    GLFWJoystickManager::loadMappings(content);
}

#ifndef FALLBACK_EGLUT
// Define this window manager as the used one
std::shared_ptr<GameWindowManager> GameWindowManager::createManager() {
    return std::shared_ptr<GameWindowManager>(new GLFWWindowManager());
}
#endif