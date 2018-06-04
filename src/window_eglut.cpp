#include "window_eglut.h"
#include "joystick_manager_linux_gamepad.h"

#include <cstring>
#include <sstream>

extern "C" {
#include <eglut.h>
}

EGLUTWindow* EGLUTWindow::currentWindow;

EGLUTWindow::EGLUTWindow(const std::string& title, int width, int height, GraphicsApi api) :
        WindowWithLinuxJoystick(title, width, height, api), title(title), width(width), height(height), graphicsApi(api) {
}

EGLUTWindow::~EGLUTWindow() {
    if (currentWindow == this)
        currentWindow = nullptr;
    if (winId != -1)
        eglutDestroyWindow(winId);
}

void EGLUTWindow::show() {
    eglutInitWindowSize(width, height);
    if (graphicsApi == GraphicsApi::OPENGL_ES2)
        eglutInitAPIMask(EGLUT_OPENGL_ES2_BIT);
    winId = eglutCreateWindow(title.c_str(), iconPath.c_str());
    eglutIdleFunc(_eglutIdleFunc);
    eglutDisplayFunc(_eglutDisplayFunc);
    eglutReshapeFunc(_eglutReshapeFunc);
    eglutMouseFunc(_eglutMouseFunc);
    eglutMouseButtonFunc(_eglutMouseButtonFunc);
    eglutMouseRawFunc(_eglutMouseRawFunc);
    eglutKeyboardFunc(_eglutKeyboardFunc);
    eglutSpecialFunc(_eglutKeyboardSpecialFunc);
    eglutPasteFunc(_eglutPasteFunc);
    eglutFocusFunc(_eglutFocusFunc);
    eglutCloseWindowFunc(_eglutCloseWindowFunc);
    currentWindow = this;
}

void EGLUTWindow::close() {
    eglutDestroyWindow(currentWindow->winId);
    eglutFini();
    currentWindow->winId = -1;
}

void EGLUTWindow::runLoop() {
    addWindowToGamepadManager();
    eglutMainLoop();
}

void EGLUTWindow::setCursorDisabled(bool disabled) {
    cursorDisabled = disabled;
    eglutSetMousePointerLocked(disabled ? EGLUT_POINTER_LOCKED : EGLUT_POINTER_UNLOCKED);
}

void EGLUTWindow::setFullscreen(bool fullscreen) {
    if (eglutGet(EGLUT_FULLSCREEN_MODE) != (fullscreen ? EGLUT_FULLSCREEN : EGLUT_WINDOWED))
        eglutToggleFullscreen();
}

void EGLUTWindow::_eglutIdleFunc() {
    if (currentWindow == nullptr)
        return;
    currentWindow->updateGamepad();
    eglutPostRedisplay();
}

void EGLUTWindow::_eglutDisplayFunc() {
    if (currentWindow == nullptr)
        return;
    currentWindow->onDraw();
}

void EGLUTWindow::_eglutReshapeFunc(int w, int h) {
    if (currentWindow == nullptr || (currentWindow->width == w && currentWindow->height == h))
        return;
    currentWindow->onWindowSizeChanged(w, h);
    currentWindow->width = w;
    currentWindow->height = h;
}

void EGLUTWindow::_eglutMouseFunc(int x, int y) {
    if (currentWindow == nullptr)
        return;
    currentWindow->onMousePosition(x, y);
}

void EGLUTWindow::_eglutMouseRawFunc(double x, double y) {
    if (currentWindow == nullptr)
        return;
    currentWindow->onMouseRelativePosition(x, y);
}

void EGLUTWindow::_eglutMouseButtonFunc(int x, int y, int btn, int action) {
    if (currentWindow == nullptr)
        return;
    if (btn == 4 || btn == 5) {
        currentWindow->onMouseScroll(x, y, 0.0, (btn == 5 ? -1.0 : 1.0));
        return;
    }
    btn = (btn == 2 ? 3 : (btn == 3 ? 2 : btn));
    currentWindow->onMouseButton(x, y, btn, action == EGLUT_MOUSE_PRESS ? MouseButtonAction::PRESS :
                                            MouseButtonAction::RELEASE);
}

void EGLUTWindow::_eglutKeyboardFunc(char str[5], int action) {
    if (currentWindow == nullptr ||
        strcmp(str, "\t") == 0 || strcmp(str, "\26") == 0 || strcmp(str, "\33") == 0) // \t, paste, esc
        return;
    if (action == EGLUT_KEY_PRESS || action == EGLUT_KEY_REPEAT) {
        if (str[0] == 13 && str[1] == 0)
            str[0] = 10;
        std::stringstream ss;
        ss << str;
        currentWindow->onKeyboardText(ss.str());
    }
}

int EGLUTWindow::getKeyMinecraft(int keyCode) {
    if (keyCode == 65505)
        return 16;
    if (keyCode >= 97 && keyCode <= 122)
        return (keyCode + 65 - 97);
    if (keyCode >= 65361 && keyCode <= 65364)
        return (keyCode + 37 - 65361);
    if (keyCode >= 65470 && keyCode <= 65481)
        return (keyCode + 112 - 65470);

    return keyCode;
}

void EGLUTWindow::_eglutKeyboardSpecialFunc(int key, int action) {
    if (currentWindow == nullptr)
        return;
    if (key == 65507)
        currentWindow->modCTRL = (action != EGLUT_KEY_RELEASE);
    if (currentWindow->modCTRL && (key == 86 || key == 118) && action == EGLUT_KEY_PRESS) {
        eglutRequestPaste();
    }
    int mKey = getKeyMinecraft(key);
    KeyAction enumAction = (action == EGLUT_KEY_PRESS ? KeyAction::PRESS :
                            (action == EGLUT_KEY_REPEAT ? KeyAction::REPEAT : KeyAction::RELEASE));
    currentWindow->onKeyboard(mKey, enumAction);
}

void EGLUTWindow::_eglutPasteFunc(const char* str, int len) {
    if (currentWindow == nullptr)
        return;
    currentWindow->onPaste(std::string(str, len));
}

void EGLUTWindow::_eglutFocusFunc(int action) {
    if (currentWindow == nullptr)
        return;
    LinuxGamepadJoystickManager::instance.onWindowFocused(currentWindow, (action == EGLUT_FOCUSED));
}

void EGLUTWindow::_eglutCloseWindowFunc() {
    if (currentWindow == nullptr)
        return;
    currentWindow->onClose();
}

void EGLUTWindow::getWindowSize(int& width, int& height) const {
    width = this->width;
    height = this->height;
}
