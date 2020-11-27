#pragma once
#include <string>
#include <functional>

class JoystickManager {
public:
    static bool handleMissingGamePadMapping(std::string name, std::string guid, int axis, int buttons, int hats, std::function<bool(std::string mapping)> updateMapping);
};