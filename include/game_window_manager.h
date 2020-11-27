#pragma once

#include "game_window.h"
#include "game_window_error_handler.h"
#include <memory>

class GameWindowManager {

private:
    static std::shared_ptr<GameWindowManager> instance;

    static std::shared_ptr<GameWindowManager> createManager();

    std::shared_ptr<GameWindowErrorHandler> errorhandler;

public:
    GameWindowManager() : errorhandler(std::make_shared<GameWindowErrorHandler>()) {}

    static std::shared_ptr<GameWindowManager> getManager();


    using AnyFunc = void* (*)();
    using ProcAddrFunc = AnyFunc (*)(const char*);


    virtual ProcAddrFunc getProcAddrFunc() = 0;

    virtual std::shared_ptr<GameWindow>
    createWindow(const std::string& title, int width, int height, GraphicsApi api) = 0;

    virtual void addGamepadMappingFile(const std::string& path) = 0;

    virtual void addGamePadMapping(const std::string &content) = 0;

    void setErrorHandler(std::shared_ptr<GameWindowErrorHandler> errorhandler) {
        if (!errorhandler) {
            this->errorhandler->onError("GameWindowManager", "errorhandler have to be an object");
            return;
        }
        this->errorhandler = std::move(errorhandler);
    }

    const std::shared_ptr<GameWindowErrorHandler>& getErrorHandler() { return errorhandler; }
};