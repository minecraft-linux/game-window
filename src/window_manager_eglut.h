#pragma once

#include "game_window_manager.h"

class EGLUTWindowManager : public GameWindowManager {

public:
    EGLUTWindowManager();

    virtual ProcAddrFunc getProcAddrFunc();

    virtual std::shared_ptr<GameWindow> createWindow(const std::string& title, int width, int height, GraphicsApi api);

};