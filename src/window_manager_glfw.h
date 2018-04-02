#pragma once

#include "game_window_manager.h"

class GLFWWindowManager : public GameWindowManager {

public:
    GLFWWindowManager();

    virtual ProcAddrFunc getProcAddrFunc();

    virtual std::shared_ptr<GameWindow> createWindow(const std::string& title, int width, int height, GraphicsApi api);

};