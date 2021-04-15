#pragma once

#include <game_window.h>
#include <EGL/egl.h>
#include <SDL2/SDL.h>


struct bgra_pixel {
    // channels in framebuffer order
    char b;
    char g;
    char r;
    char a;
};

class SDL2GameWindow : public GameWindow {

private:

    struct framebuffer_state {
        int fd;
        bgra_pixel *mmap;
        int mmap_size;
        int w;
        int h;
        bgra_pixel *frontbuff;
        bgra_pixel *backbuff;
    } fb;

    struct egl_state {
        EGLDisplay display;
        EGLContext context;
        EGLSurface surface;
    } egl;

    struct cursor_state {
        bgra_pixel **img;
        int size;
        bool hidden;
        bool inuse;
        int x;
        int y;
    } cursor;

    struct gamepad_state {
        int count;
    } gamepad;

    SDL2GameWindow *currentGameWindow;

    void abortMsg(const char *msg);
    void initFrameBuffer();
    void initEGL();
    void clearColour(float shade);
    void initSDL();
    void initCursor();
    void drawCursor();
    void handleControllerDeviceEvent(SDL_ControllerDeviceEvent *cdeviceevent);
    void handleControllerAxisEvent(SDL_ControllerAxisEvent *caxisevent);
    void handleControllerButtonEvent(SDL_ControllerButtonEvent *cbuttonevent);
    void handleMouseMotionEvent(SDL_MouseMotionEvent *motionevent);
    void handleMouseWheelEvent(SDL_MouseWheelEvent *wheelevent);
    void handleMouseClickEvent(SDL_MouseButtonEvent *clickevent);
    void handleKeyboardEvent(SDL_KeyboardEvent *event);
    KeyCode getKeyMinecraft(int keyCode);

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
