#include "window_with_linux_gamepad.h"
#include "joystick_manager_linux_gamepad.h"

WindowWithLinuxJoystick::WindowWithLinuxJoystick(std::string const& title, int width, int height, GraphicsApi api) :
        GameWindow(title, width, height, api) {
}

WindowWithLinuxJoystick::~WindowWithLinuxJoystick() {
    LinuxGamepadJoystickManager::instance.removeWindow(this);
}

void WindowWithLinuxJoystick::addWindowToGamepadManager() {
    LinuxGamepadJoystickManager::instance.addWindow(this);
    LinuxGamepadJoystickManager::instance.onWindowFocused(this, true);
}

void WindowWithLinuxJoystick::updateGamepad() {
    LinuxGamepadJoystickManager::instance.update(this);
}