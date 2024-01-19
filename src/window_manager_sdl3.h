#pragma once

#include "game_window_manager.h"

class SDL3WindowManager : public GameWindowManager {

public:
    SDL3WindowManager();

    ProcAddrFunc getProcAddrFunc() override;

    std::shared_ptr<GameWindow> createWindow(const std::string& title, int width, int height, GraphicsApi api) override;

    void addGamepadMappingFile(const std::string& path) override;

    void addGamePadMapping(const std::string &content) override;
};