#pragma once

#include "game_window_manager.h"

class GLFWFallbackEGLUTWindowManager : public GameWindowManager {
    std::shared_ptr<GameWindowManager> manager;
public:
    GLFWFallbackEGLUTWindowManager();

    ProcAddrFunc getProcAddrFunc() override;

    std::shared_ptr<GameWindow> createWindow(const std::string& title, int width, int height, GraphicsApi api) override;

    void addGamepadMappingFile(const std::string& path) override;

    void addGamePadMapping(const std::string &content) override;
};