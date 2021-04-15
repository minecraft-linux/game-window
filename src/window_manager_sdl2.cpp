#include "window_manager_sdl2.h"
#include "window_sdl2.h"

GameWindowManager::ProcAddrFunc dlsymGetProcAddress(const char* sym) {
    if (!sym)
        return NULL;
    void *eglFunc;

    // try official EGL method first
    eglFunc = (void*)SDL_GL_GetProcAddress(sym);

    return (GameWindowManager::ProcAddrFunc)eglFunc;
}

SDL2WindowManager::SDL2WindowManager() {
    /*
    nothing instantiated yet
    just a handle to:
      1. access the GL libraries via dlsymGetProcAddress()
      2. the future window
    */
}

GameWindowManager::ProcAddrFunc SDL2WindowManager::getProcAddrFunc() {
    return (GameWindowManager::ProcAddrFunc) dlsymGetProcAddress;
}

std::shared_ptr<GameWindow> SDL2WindowManager::createWindow(const std::string& title, int width, int height,
                                                             GraphicsApi api) {
    return std::shared_ptr<GameWindow>(new SDL2GameWindow(title, width, height, api));
}

void SDL2WindowManager::addGamepadMappingFile(const std::string &path) {
    printf("Loaded %d more controller mappings from %s\n", SDL_GameControllerAddMappingsFromFile(path.c_str()), path.c_str());
}

void SDL2WindowManager::addGamePadMapping(const std::string &content) {
    // NOOP
}

// Define this window manager as the used one
std::shared_ptr<GameWindowManager> GameWindowManager::createManager() {
    return std::shared_ptr<GameWindowManager>(new SDL2WindowManager());
}
