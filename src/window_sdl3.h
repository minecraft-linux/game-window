#pragma once
#if !defined(GAMEWINDOW_NO_X11_LOCK) && !defined(GAMEWINDOW_X11_LOCK) && !defined(__APPLE__)
#define GAMEWINDOW_X11_LOCK
#endif

#include <game_window.h>
#include <mutex>
#include <SDL3/SDL.h>

class SDL3GameWindow : public GameWindow {

private:
    SDL_Window* window;
    SDL_GLContext context;
    double lastMouseX = 0.0, lastMouseY = 0.0;
    int windowedX = -1, windowedY = -1;
    // width and height in content pixels
    int width = -1, height = -1;
    // width and height in window coordinates = pixels / relativeScale
    int windowedWidth = -1, windowedHeight = -1;
    int relativeScale;
    bool resized = false;
    bool focused = true;
    bool warnedButtons = false;

    static KeyCode getKeyMinecraft(int keyCode);

public:

    SDL3GameWindow(const std::string& title, int width, int height, GraphicsApi api);

    ~SDL3GameWindow() override;

    void setIcon(std::string const& iconPath) override;

    void makeCurrent(bool active) override;

    int getRelativeScale() const;

    void setRelativeScale();

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
