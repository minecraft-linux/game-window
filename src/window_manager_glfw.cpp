#include "window_manager_glfw.h"
#include "window_glfw.h"
#include "joystick_manager_glfw.h"
#include <stdexcept>

GLFWWindowManager::GLFWWindowManager() {
    if (glfwInit() != GLFW_TRUE)
        throw std::runtime_error("glfwInit error");
    GLFWJoystickManager::init();
}

GameWindowManager::ProcAddrFunc GLFWWindowManager::getProcAddrFunc() {
    return (GameWindowManager::ProcAddrFunc) glfwGetProcAddress;
}

std::shared_ptr<GameWindow> GLFWWindowManager::createWindow(const std::string& title, int width, int height,
                                                             GraphicsApi api) {
    return std::shared_ptr<GameWindow>(new GLFWGameWindow(title, width, height, api, classname));
}

void GLFWWindowManager::addGamepadMappingFile(const std::string &path) {
    GLFWJoystickManager::loadMappingsFromFile(path);
}

std::string GLFWWindowManager::getClassInstanceName() {
    return classname;
}

void GLFWWindowManager::setClassInstanceName(std::string classname) {
    this->classname = classname;
}

// Define this window manager as the used one
std::shared_ptr<GameWindowManager> GameWindowManager::createManager() {
    return std::shared_ptr<GameWindowManager>(new GLFWWindowManager());
}