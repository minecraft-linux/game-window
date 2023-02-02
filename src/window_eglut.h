#pragma once
#if !defined(GAMEWINDOW_NO_X11_LOCK) && !defined(GAMEWINDOW_X11_LOCK) && !defined(__APPLE__)
#define GAMEWINDOW_X11_LOCK
#endif

#include "window_with_linux_gamepad.h"

#include <mutex>

class EGLUTWindow : public WindowWithLinuxJoystick {

private:
    static EGLUTWindow* currentWindow;

    std::string title;
    int width, height;
    GraphicsApi graphicsApi;
    int winId = -1;
    bool cursorDisabled = false;
    bool moveMouseToCenter = false;
    int lastMouseX = -1, lastMouseY = -1;
    bool modCTRL = false;
    int pointerIds[16];

#ifdef GAMEWINDOW_X11_LOCK
    std::recursive_mutex x11_sync;
#endif

    static KeyCode getKeyMinecraft(int keyCode);

    static void _eglutIdleFunc();
    static void _eglutDisplayFunc();
    static void _eglutReshapeFunc(int w, int h);
    static void _eglutMouseFunc(int x, int y);
    static void _eglutMouseRawFunc(double x, double y);
    static void _eglutMouseButtonFunc(int x, int y, int btn, int action);
    static void _eglutTouchStartFunc(int id, double x, double y);
    static void _eglutTouchUpdateFunc(int id, double x, double y);
    static void _eglutTouchEndFunc(int id, double x, double y);
    static void _eglutKeyboardFunc(char str[5], int action);
    static void _eglutKeyboardSpecialFunc(int key, int action);
    static void _eglutPasteFunc(const char* str, int len);
    static void _eglutFocusFunc(int action);
    static void _eglutCloseWindowFunc();

    int obtainTouchPointer(int eglutId);
    void releaseTouchPointer(int ourId);

public:
    EGLUTWindow(const std::string& title, int width, int height, GraphicsApi api);

    ~EGLUTWindow() override;

    void setIcon(std::string const& iconPath) override;

    void makeCurrent(bool active) override;

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
