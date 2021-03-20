#include "joystick_manager.h"
#include <game_window_manager.h>
#include <sstream>

bool JoystickManager::handleMissingGamePadMapping(std::string name, std::string guid, int axescount, int buttonscount, int hatscount, std::function<bool(std::string mapping)> updateMapping) {
#ifndef NDEBUG
        std::stringstream errormsg;
        errormsg << "Missing Gamepad Mapping for controller '" << name << "'(" << guid << "). Please create a Gamepad Mapping for your gamepad.";
        std::ostringstream mapping;
        mapping << guid << "," << name;
        const char* btns[] = { "a", "b", "x", "y", "leftshoulder", "rightshoulder", "righttrigger", "lefttrigger", "back", "start", "leftstick", "rightstick", "guide", "dpleft", "dpdown", "dpright", "dpup" };
        
        const char* axes[] = { "leftx", "lefty", "rightx", "righty", "lefttrigger", "righttrigger" };
        if (axescount) {
            std::ostringstream submap;
            for (size_t i = 0; i < axescount && i < sizeof(axes) / sizeof(axes[0]); i++) {
                submap << "," << axes[i] << ":a" << i;
            }
            mapping << submap.str();
        }
        const char* hats[] = { "dpup", "dpright", "dpdown", "dpleft" };
        if (hatscount) {
            std::ostringstream submap;
            for (size_t i = 0; i < hatscount && i < sizeof(hats) / sizeof(hats[0]) / 4; i++) {
                for (size_t j = 0; j < 4; j++) {
                    submap << "," << hats[i*4 + j] << ":h" << i << "." << (1 << j);
                }
            }
            mapping << submap.str();
        }
        if (buttonscount) {
            std::ostringstream submap;
            for (size_t i = 0; i < buttonscount && i < sizeof(btns) / sizeof(btns[0]); i++) {
                submap << "," << btns[i] << ":b" << i;
            }
            mapping << submap.str();
        }
        auto mapstr = mapping.str();
        mapstr = mapstr + ",platform:Linux,\n" + mapstr + ",platform:Mac OS X,";
        bool _hasmapping = updateMapping(mapstr);

        if (!_hasmapping) {
            errormsg << " Failed to create a valid dummy mapping for this controller, you won't be able to use this controller.";
        } else {
            errormsg << " This Launcher has created an dummy Gamepad Mapping for you, you will have to create your own for best experience: '" << mapstr << "'";
        }

        GameWindowManager::getManager()->getErrorHandler()->onError("JoystickManager", errormsg.str());

        return _hasmapping;
#else
        return false;
#endif
}
