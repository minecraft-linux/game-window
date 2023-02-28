#include "joystick_manager_glfw.h"

#include <cstring>
#include <fstream>
#include "window_glfw.h"
#include "joystick_manager.h"
#include "game_window_manager.h"

std::unordered_set<GLFWGameWindow*> GLFWJoystickManager::windows;
GLFWGameWindow* GLFWJoystickManager::focusedWindow;
std::unordered_map<int, GLFWJoystickManager::JoystickInfo> GLFWJoystickManager::connectedJoysticks;
std::unordered_set<int> GLFWJoystickManager::userIds;

void GLFWJoystickManager::init() {
    glfwSetJoystickCallback(_glfwJoystickCallback);
    loadMappingsFromFile("gamecontrollerdb.txt");
}

void GLFWJoystickManager::loadMappingsFromFile(std::string const& path) {
    std::ifstream fs(path);
    if (!fs)
        return;
    std::string line;
    while (std::getline(fs, line)) {
        if (!line.empty())
            loadMappings(line);
    }
}

void GLFWJoystickManager::loadMappings(const std::string &content) {
    glfwUpdateGamepadMappings(content.c_str());
}

int GLFWJoystickManager::nextUnassignedUserId() {
    for (int i = 0; ; i++) {
        if (userIds.count(i) <= 0)
            return i;
    }
}

void GLFWJoystickManager::update(GLFWGameWindow* window) {
    if (focusedWindow != window)
        return;

    for (auto& j : connectedJoysticks) {
        GLFWgamepadstate state;
        glfwGetGamepadState(j.first, &state);
        for (int i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; i++) {
            if (state.buttons[i] != j.second.oldButtonStates[i]) {
                window->onGamepadButton(j.second.userId, mapButtonId(i), state.buttons[i] != 0);
            }
        }
        for (int i = 0; i <= GLFW_GAMEPAD_AXIS_LAST; i++) {
            float value = state.axes[i];
            switch(i) {
            case GLFW_GAMEPAD_AXIS_LEFT_TRIGGER:
            case GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER:
                value = value / 2.0f + 0.5f;
            break;
            }
            window->onGamepadAxis(j.second.userId, mapAxisId(i), value);
        }

        memcpy(j.second.oldButtonStates, state.buttons, GLFW_GAMEPAD_BUTTON_LAST + 1);
    }
}

void GLFWJoystickManager::update() {
    update(focusedWindow);
}

void GLFWJoystickManager::addWindow(GLFWGameWindow* window) {
    windows.insert(window);
    if (windows.size() == 1) {
        // First window created poll all joysticks for valid mappings etc.
        // Doing this earlier can cause unintenional errors if muliple mapping are added before the first window is created
        for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
            if (glfwJoystickPresent(i)) {
                _glfwJoystickCallback(i, GLFW_CONNECTED);
            }
        }
    } else {
        // Only newly added window gets the events
        for (auto& joystick : connectedJoysticks)
            window->onGamepadState(joystick.second.userId, true);
    }
}

void GLFWJoystickManager::removeWindow(GLFWGameWindow* window) {
    windows.erase(window);
}

void GLFWJoystickManager::onWindowFocused(GLFWGameWindow* window, bool focused) {
    if (focused)
        focusedWindow = window;
    else if (focusedWindow == window /* && !focused */)
        focusedWindow = nullptr;
}

void GLFWJoystickManager::_glfwJoystickCallback(int joystick, int action) {
    auto js = connectedJoysticks.find(joystick);
    int userId;
    if (action == GLFW_CONNECTED) {
        if (!glfwJoystickIsGamepad(joystick)) {
            if (windows.empty()) {
                // No Warning before first window is created
                return;
            }
            int axis, hats, buttons;
            if (!glfwGetJoystickAxes(joystick, &axis)) {
                axis = 0;
            }
            if (!glfwGetJoystickHats(joystick, &hats)) {
                hats = 0;
            }
            if (!glfwGetJoystickButtons(joystick, &buttons)) {
                buttons = 0;
            }
            if(!JoystickManager::handleMissingGamePadMapping(glfwGetJoystickName(joystick), glfwGetJoystickGUID(joystick), axis, buttons, hats, [&](std::string mapping) {
                GameWindowManager::getManager()->addGamePadMapping(mapping);
                return glfwJoystickIsGamepad(joystick);
            })) {
                // Default mapping failed
                return;
            }
        }

        if (js != connectedJoysticks.end())
            return;
        userId = nextUnassignedUserId();
        userIds.insert(userId);
        connectedJoysticks.insert({joystick, JoystickInfo(userId)});
    } else if (action == GLFW_DISCONNECTED) {
        if (js == connectedJoysticks.end())
            return;
        userId = js->second.userId;
        userIds.erase(userId);
        connectedJoysticks.erase(joystick);
    }

    for (GLFWGameWindow* window : windows)
        window->onGamepadState(userId, action == GLFW_CONNECTED);
}

GamepadButtonId GLFWJoystickManager::mapButtonId(int id) {
    switch (id) {
        case GLFW_GAMEPAD_BUTTON_A: return GamepadButtonId::A;
        case GLFW_GAMEPAD_BUTTON_B: return GamepadButtonId::B;
        case GLFW_GAMEPAD_BUTTON_X: return GamepadButtonId::X;
        case GLFW_GAMEPAD_BUTTON_Y: return GamepadButtonId::Y;
        case GLFW_GAMEPAD_BUTTON_BACK: return GamepadButtonId::BACK;
        case GLFW_GAMEPAD_BUTTON_START: return GamepadButtonId::START;
        case GLFW_GAMEPAD_BUTTON_GUIDE: return GamepadButtonId::GUIDE;
        case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER: return GamepadButtonId::LB;
        case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER: return GamepadButtonId::RB;
        case GLFW_GAMEPAD_BUTTON_LEFT_THUMB: return GamepadButtonId::LEFT_STICK;
        case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB: return GamepadButtonId::RIGHT_STICK;
        case GLFW_GAMEPAD_BUTTON_DPAD_UP: return GamepadButtonId::DPAD_UP;
        case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT: return GamepadButtonId::DPAD_RIGHT;
        case GLFW_GAMEPAD_BUTTON_DPAD_DOWN: return GamepadButtonId::DPAD_DOWN;
        case GLFW_GAMEPAD_BUTTON_DPAD_LEFT: return GamepadButtonId::DPAD_LEFT;
        default: return GamepadButtonId::UNKNOWN;
    }
}

GamepadAxisId GLFWJoystickManager::mapAxisId(int id) {
    switch (id) {
        case GLFW_GAMEPAD_AXIS_LEFT_X: return GamepadAxisId::LEFT_X;
        case GLFW_GAMEPAD_AXIS_LEFT_Y: return GamepadAxisId::LEFT_Y;
        case GLFW_GAMEPAD_AXIS_RIGHT_X: return GamepadAxisId::RIGHT_X;
        case GLFW_GAMEPAD_AXIS_RIGHT_Y: return GamepadAxisId::RIGHT_Y;
        case GLFW_GAMEPAD_AXIS_LEFT_TRIGGER: return GamepadAxisId::LEFT_TRIGGER;
        case GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER: return GamepadAxisId::RIGHT_TRIGGER;
        default: return GamepadAxisId::UNKNOWN;
    }
}