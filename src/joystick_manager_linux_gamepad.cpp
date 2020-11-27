#include "joystick_manager_linux_gamepad.h"

#include <fstream>
#include <gamepad/joystick_manager_factory.h>
#include <gamepad/joystick.h>
#include "window_with_linux_gamepad.h"
#include <sstream>
#include <gamepad/gamepad_mapping.h>
#include "joystick_manager.h"
#include <game_window_manager.h>

LinuxGamepadJoystickManager LinuxGamepadJoystickManager::instance;

LinuxGamepadJoystickManager::LinuxGamepadJoystickManager() : joystickManager(gamepad::JoystickManagerFactory::create()),
                                                             gamepadManager(*joystickManager) {
    using namespace std::placeholders;
    gamepadManager.onGamepadConnected.add(std::bind(&LinuxGamepadJoystickManager::onGamepadState, this, _1, true));
    gamepadManager.onGamepadDisconnected.add(std::bind(&LinuxGamepadJoystickManager::onGamepadState, this, _1, false));
    gamepadManager.onGamepadButton.add(std::bind(&LinuxGamepadJoystickManager::onGamepadButton, this, _1, _2, _3));
    gamepadManager.onGamepadAxis.add(std::bind(&LinuxGamepadJoystickManager::onGamepadAxis, this, _1, _2, _3));

    loadMappingsFromFile("gamecontrollerdb.txt");
}

void LinuxGamepadJoystickManager::initialize() {
    if (!initialized) {
        initialized = true;
        joystickManager->initialize();
    }
}

void LinuxGamepadJoystickManager::update(WindowWithLinuxJoystick* window) {
    if (focusedWindow != window)
        return;

    initialize();
    joystickManager->poll();
}

void LinuxGamepadJoystickManager::loadMappingsFromFile(std::string const& path) {
    std::ifstream fs(path);
    if (!fs)
        return;
    std::string line;
    while (std::getline(fs, line)) {
        if (!line.empty() && line[0] != '#') {
            try {
                gamepadManager.addMapping(line);
            } catch (std::exception& e) {
                printf("Invalid mapping in %s: %s\n", path.c_str(), e.what());
            }
        }
    }
}

void LinuxGamepadJoystickManager::loadMappings(const std::string &content) {
    for (size_t i = 0; i < content.length(); ) {
        size_t j = content.find('\n', i);
        std::string line = content.substr(i,  j);
        gamepadManager.addMapping(line);
        if (j == std::string::npos) {
            // last line
            break;
        }
        i = j + 1;
    }
}

void LinuxGamepadJoystickManager::addWindow(WindowWithLinuxJoystick* window) {
    initialize();
    windows.insert(window);
    if (windows.size() == 1) {
        // First window created poll all joysticks for valid mappings etc.
        // Doing this earlier can cause unintenional errors if muliple mapping are added before the first window is created
        for (gamepad::Gamepad* gp : gamepads) {
            warnOnMissingGamePadMapping(gp);
            window->onGamepadState(gp->getIndex(), true);
        }
    } else {
        for (gamepad::Gamepad* gp : gamepads) {
            window->onGamepadState(gp->getIndex(), true);
        }
    }
}

void LinuxGamepadJoystickManager::removeWindow(WindowWithLinuxJoystick* window) {
    windows.erase(window);
}

void LinuxGamepadJoystickManager::onWindowFocused(WindowWithLinuxJoystick* window, bool focused) {
    if (focused)
        focusedWindow = window;
    else if (focusedWindow == window /* && !focused */)
        focusedWindow = nullptr;
}

void LinuxGamepadJoystickManager::onGamepadState(gamepad::Gamepad* gp, bool connected) {
    if (connected) {
        warnOnMissingGamePadMapping(gp);
        gamepads.insert(gp);
    }
    else
        gamepads.erase(gp);

    for (auto window : windows)
        window->onGamepadState(gp->getIndex(), connected);
}
void LinuxGamepadJoystickManager::warnOnMissingGamePadMapping(gamepad::Gamepad* gp) {
    if (gp->getMapping().mappings.empty()) {
        if (windows.empty()) {
            // No Warning before first window is created
            return;
        }
        if (!JoystickManager::handleMissingGamePadMapping("Unknown", gp->getJoystick().getGUID(), 4, 12, 1, [&](std::string mapping) {
            GameWindowManager::getManager()->addGamePadMapping(mapping);
            if (gp->getMapping().mappings.empty()) {
                // Update Gamepad, needed to add refresh mapping
                auto& joystick = (gamepad::Joystick&)gp->getJoystick();
                auto m = std::make_shared<gamepad::GamepadMapping>();
                m->parse(mapping);
                auto index = gp->getIndex();
                gp->~Gamepad();
                new (gp) gamepad::Gamepad(index, joystick, *m.get());
                unknownmappings.push_back(std::move(m));
            }
            return !gp->getMapping().mappings.empty();
        })) {
            // Default empty mapping
            return;
        }
    }
}

void LinuxGamepadJoystickManager::onGamepadButton(gamepad::Gamepad* gp, gamepad::GamepadButton btn, bool state) {
    if (focusedWindow != nullptr)
        focusedWindow->onGamepadButton(gp->getIndex(), mapButtonId(btn), state);
}

void LinuxGamepadJoystickManager::onGamepadAxis(gamepad::Gamepad* gp, gamepad::GamepadAxis axis, float value) {
    if (focusedWindow != nullptr)
        focusedWindow->onGamepadAxis(gp->getIndex(), mapAxisId(axis), value);
}

GamepadButtonId LinuxGamepadJoystickManager::mapButtonId(gamepad::GamepadButton id) {
    return (GamepadButtonId) id;
}

GamepadAxisId LinuxGamepadJoystickManager::mapAxisId(gamepad::GamepadAxis id) {
    return (GamepadAxisId) id;
}