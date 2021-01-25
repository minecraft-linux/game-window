#pragma once

#include "game_window_manager.h"


GameWindowManager::ProcAddrFunc dlsymGetProcAddress(const char*);


class SDL2WindowManager : public GameWindowManager {

public:
    SDL2WindowManager();

    ProcAddrFunc getProcAddrFunc() override;

    std::shared_ptr<GameWindow> createWindow(const std::string& title, int width, int height, GraphicsApi api) override;

    void addGamepadMappingFile(const std::string& path) override;

    void addGamePadMapping(const std::string &content) override;
};
