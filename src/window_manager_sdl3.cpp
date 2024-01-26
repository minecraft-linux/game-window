#include "window_manager_sdl3.h"
#include "window_sdl3.h"
#include <stdexcept>

#include <SDL3/SDL.h>

SDL3WindowManager::SDL3WindowManager() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD);
}

GameWindowManager::ProcAddrFunc SDL3WindowManager::getProcAddrFunc() {
    return (GameWindowManager::ProcAddrFunc) SDL_GL_GetProcAddress;
}

std::shared_ptr<GameWindow> SDL3WindowManager::createWindow(const std::string& title, int width, int height,
                                                             GraphicsApi api) {
    return std::shared_ptr<GameWindow>(new SDL3GameWindow(title, width, height, api));
}

void SDL3WindowManager::addGamepadMappingFile(const std::string &path) {
    SDL_AddGamepadMappingsFromFile(path.data());
}

void SDL3WindowManager::addGamePadMapping(const std::string &content) {
    SDL_AddGamepadMapping(content.data());
}

// Define this window manager as the used one
std::shared_ptr<GameWindowManager> GameWindowManager::createManager() {
    return std::shared_ptr<GameWindowManager>(new SDL3WindowManager());
}