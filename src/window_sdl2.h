#pragma once

#include <game_window.h>
#include <SDL2/SDL.h>

class SDL2GameWindow : public GameWindow {

private:
    struct gamepad_state {
        int count;
    } gamepad;

    void abortMsg(const char *msg);
    void initSDL();
    void handleControllerDeviceEvent(SDL_ControllerDeviceEvent *cdeviceevent);
    void handleControllerAxisEvent(SDL_ControllerAxisEvent *caxisevent);
    void handleControllerButtonEvent(SDL_ControllerButtonEvent *cbuttonevent);
    void handleMouseMotionEvent(SDL_MouseMotionEvent *motionevent);
    void handleMouseWheelEvent(SDL_MouseWheelEvent *wheelevent);
    void handleMouseClickEvent(SDL_MouseButtonEvent *clickevent);
    void handleKeyboardEvent(SDL_KeyboardEvent *event);
    KeyCode getKeyMinecraft(int keyCode);
    bool captured;
    SDL_Window *window;
    SDL_GLContext context;

public:

    SDL2GameWindow(const std::string& title, int width, int height, GraphicsApi api);

    ~SDL2GameWindow() override;

    void setIcon(std::string const& iconPath) override;

    void show() override;

    void close() override;

    void pollEvents() override;

    void setCursorDisabled(bool disabled) override;

    void setFullscreen(bool fullscreen) override;

    void getWindowSize(int& width, int& height) const override;

    void setClipboardText(std::string const& text) override;

    void swapBuffers() override;

    void setSwapInterval(int interval) override;

};
