#include "window_sdl2.h"
#include "game_window_manager.h"
#include <string.h>

#include <SDL2/SDL.h>

#include <linux/fb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <algorithm>

SDL2GameWindow::SDL2GameWindow(const std::string& title, int width, int height, GraphicsApi api) :
        GameWindow(title, width, height, api) {
    captured = false;
    initSDL();
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    
    window = SDL_CreateWindow(title.data(), 0, 0, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    context = SDL_GL_CreateContext(window);
}

SDL2GameWindow::~SDL2GameWindow() {
    SDL_Quit();
}

void SDL2GameWindow::abortMsg(const char *msg)
{
    fflush(stdout);
    fprintf(stderr, "Fatal Error: %s\n", msg);
    exit(1);
}

void SDL2GameWindow::initSDL() {
    // video is mandatory to get events, even though we aren't using it, so we wont be creating a window
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_GAMECONTROLLER) != 0) {
        abortMsg("Unable to initialize SDL for video|events|gamecontroller");
    }

    if(!SDL_GameControllerEventState(SDL_QUERY))
        SDL_GameControllerEventState(SDL_ENABLE);

    gamepad.count = 0;
}

void SDL2GameWindow::setIcon(std::string const& iconPath) {
    // NOOP - borderless window
}

void SDL2GameWindow::getWindowSize(int& width, int& height) const {
    SDL_GetWindowSize(window, &width, &height);
}

void SDL2GameWindow::show() {
    SDL_ShowWindow(window);
}

void SDL2GameWindow::close() {
    SDL_DestroyWindow(window);
}

void SDL2GameWindow::pollEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
                handleControllerDeviceEvent(&event.cdevice);
                break;
            case SDL_CONTROLLERAXISMOTION:
                handleControllerAxisEvent(&event.caxis);
                break;
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                handleControllerButtonEvent(&event.cbutton);
                break;
            case SDL_MOUSEMOTION:
                handleMouseMotionEvent(&event.motion);
                break;
            case SDL_MOUSEWHEEL:
                handleMouseWheelEvent(&event.wheel);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                handleMouseClickEvent(&event.button);
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                handleKeyboardEvent(&event.key);
                break;
            case SDL_QUIT:
                SDL_Quit();
                break;
            default:
                break;
        }
    }
}

void SDL2GameWindow::handleControllerDeviceEvent(SDL_ControllerDeviceEvent *cdeviceevent) {
    if (cdeviceevent->type == SDL_CONTROLLERDEVICEADDED) {
        // The game will only be informed of the first connection and last disconnection.
        // All inputs seen as controller 0 so the behaviour of multiple connected gamepads will be undefined.
        gamepad.count++;
        SDL_GameController *controller = NULL;
        controller = SDL_GameControllerOpen(cdeviceevent->which);
        if(!controller)
            printf("SDL2GameWindow: Couldn't open controller! - %s\n", SDL_GetError());
        else
            printf("SDL2GameWindow: Controller %d opened: %s!\n", cdeviceevent->which, SDL_GameControllerName(controller));
        if (gamepad.count > 1)
            return;
    }
    else if (cdeviceevent->type == SDL_CONTROLLERDEVICEREMOVED) {
        if (gamepad.count < 1) {
            printf("SDL2GameWindow: Error - controller removed when none were known to be connected");
            return;
        }
        else
            gamepad.count--;
            printf("SDL2GameWindow: Controller %d removed!\n", cdeviceevent->which);
            if (gamepad.count > 0)
                return;
    }
    else
        return;

    printf("SDL2GameWindow: There are now %d connected joysticks\n", SDL_NumJoysticks());
    onGamepadState(0, (cdeviceevent->type == SDL_CONTROLLERDEVICEADDED));
}

void SDL2GameWindow::handleControllerAxisEvent(SDL_ControllerAxisEvent *caxisevent) {
    GamepadAxisId axis;
    switch (caxisevent->axis) {
        case SDL_CONTROLLER_AXIS_LEFTX:
            axis = GamepadAxisId::LEFT_X;
            break;
        case SDL_CONTROLLER_AXIS_LEFTY:
            axis = GamepadAxisId::LEFT_Y;
            break;
        case SDL_CONTROLLER_AXIS_RIGHTX:
            axis = GamepadAxisId::RIGHT_X;
            break;
        case SDL_CONTROLLER_AXIS_RIGHTY:
            axis = GamepadAxisId::RIGHT_Y;
            break;
        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
            axis = GamepadAxisId::LEFT_TRIGGER;
            break;
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
            axis = GamepadAxisId::RIGHT_TRIGGER;
            break;
        default :
            return;
    }

    double deflection = (double)caxisevent->value / 32768; // normalised -1 to 1 range
    onGamepadAxis(0, axis, deflection);
}

void SDL2GameWindow::handleControllerButtonEvent(SDL_ControllerButtonEvent *cbuttonevent) {
    GamepadButtonId btn;

    switch (cbuttonevent->button) {
        case SDL_CONTROLLER_BUTTON_A:
            btn = GamepadButtonId::A;
            break;
        case SDL_CONTROLLER_BUTTON_B:
            btn = GamepadButtonId::B;
            break;
        case SDL_CONTROLLER_BUTTON_X:
            btn = GamepadButtonId::X;
            break;
        case SDL_CONTROLLER_BUTTON_Y:
            btn = GamepadButtonId::Y;
            break;
        case SDL_CONTROLLER_BUTTON_BACK:
            btn = GamepadButtonId::BACK;
            break;
        case SDL_CONTROLLER_BUTTON_START:
            btn = GamepadButtonId::START;
            break;
        case SDL_CONTROLLER_BUTTON_GUIDE:
            btn = GamepadButtonId::GUIDE;
            break;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            btn = GamepadButtonId::LEFT_STICK;
            break;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            btn = GamepadButtonId::RIGHT_STICK;
            break;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            btn = GamepadButtonId::LB;
            break;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            btn = GamepadButtonId::RB;
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            btn = GamepadButtonId::DPAD_UP;
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            btn = GamepadButtonId::DPAD_DOWN;
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            btn = GamepadButtonId::DPAD_LEFT;
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            btn = GamepadButtonId::DPAD_RIGHT;
            break;
        default :
            return;
    }

    onGamepadButton(0, btn, (cbuttonevent->state == SDL_PRESSED));
}

void SDL2GameWindow::handleMouseWheelEvent(SDL_MouseWheelEvent *wheelevent) {
    if (wheelevent->x < 0)
        onMouseScroll(wheelevent->x, wheelevent->y, -1.0, 0.0);
    else if (wheelevent->x > 0)
        onMouseScroll(wheelevent->x, wheelevent->y, 1.0, 0.0);
    else if (wheelevent->y < 0)
        onMouseScroll(0.0, 0.0, 0.0, -1);
    else if (wheelevent->y > 0)
        onMouseScroll(0.0, 0.0, 0.0, 1);
}

void SDL2GameWindow::handleMouseMotionEvent(SDL_MouseMotionEvent *motionevent) {
    if (captured) {
        onMouseRelativePosition(motionevent->xrel, motionevent->yrel);
    }
    else {
        onMousePosition(motionevent->x, motionevent->y);
    }
}

void SDL2GameWindow::handleMouseClickEvent(SDL_MouseButtonEvent *clickevent) {
    onMouseButton(clickevent->x, clickevent->y, clickevent->button, (clickevent->state == SDL_PRESSED ? MouseButtonAction::PRESS : MouseButtonAction::RELEASE));
}

void SDL2GameWindow::handleKeyboardEvent(SDL_KeyboardEvent *keyevent) {
    KeyCode key = getKeyMinecraft(keyevent->keysym.scancode);

    KeyAction action;
    if (keyevent->repeat) {
        action = KeyAction::REPEAT;
    }
    else {
        switch (keyevent->state) {
            case SDL_PRESSED:
                action = KeyAction::PRESS;
                break;
            case SDL_RELEASED:
                action = KeyAction::RELEASE;
                break;
            default:
              return;
        }
    }

    onKeyboard(key, action);
}

void SDL2GameWindow::setCursorDisabled(bool disabled) {
    captured = disabled;
    SDL_CaptureMouse((SDL_bool) disabled);
}

void SDL2GameWindow::setFullscreen(bool fullscreen) {
    SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
}

void SDL2GameWindow::setClipboardText(std::string const &text) {
    // NOOP - nowhere to cut/paste to/from without a desktop and other applications
}

void SDL2GameWindow::swapBuffers() {
    SDL_GL_SwapWindow(window);
}

void SDL2GameWindow::setSwapInterval(int interval) {
    SDL_GL_SetSwapInterval(interval);
}

// TODO fix QWERTY and numpad mapping.
KeyCode SDL2GameWindow::getKeyMinecraft(int keyCode) {
    if (keyCode >= SDLK_F1 && keyCode <= SDLK_F12)
        return (KeyCode) (keyCode - SDLK_F1 + (int) KeyCode::FN1);

    switch (keyCode) {
        case SDLK_BACKSPACE:
            return KeyCode::BACKSPACE;
        case SDLK_TAB:
            return KeyCode::TAB;
        case SDLK_RETURN:
            return KeyCode::ENTER;
        case SDLK_LSHIFT:
            return KeyCode::LEFT_SHIFT;
        case SDLK_RSHIFT:
            return KeyCode::RIGHT_SHIFT;
        case SDLK_LCTRL:
            return KeyCode::LEFT_CTRL;
        case SDLK_RCTRL:
            return KeyCode::RIGHT_CTRL;
        case SDLK_PAUSE:
            return KeyCode::PAUSE;
        case SDLK_CAPSLOCK:
            return KeyCode::CAPS_LOCK;
        case SDLK_ESCAPE:
            return KeyCode::ESCAPE;
        case SDLK_PAGEUP:
            return KeyCode::PAGE_UP;
        case SDLK_PAGEDOWN:
            return KeyCode::PAGE_DOWN;
        case SDLK_END:
            return KeyCode::END;
        case SDLK_HOME:
            return KeyCode::HOME;
        case SDLK_LEFT:
            return KeyCode::LEFT;
        case SDLK_UP:
            return KeyCode::UP;
        case SDLK_RIGHT:
            return KeyCode::RIGHT;
        case SDLK_DOWN:
            return KeyCode::DOWN;
        case SDLK_INSERT:
            return KeyCode::INSERT;
        case SDLK_DELETE:
            return KeyCode::DELETE;
        case SDLK_NUMLOCKCLEAR:
            return KeyCode::NUM_LOCK;
        case SDLK_SCROLLLOCK:
            return KeyCode::SCROLL_LOCK;
        case SDLK_SEMICOLON:
            return KeyCode::SEMICOLON;
        case SDLK_EQUALS:
            return KeyCode::EQUAL;
        case SDLK_COMMA:
            return KeyCode::COMMA;
        case SDLK_MINUS:
            return KeyCode::MINUS;
        case SDLK_PERIOD:
            return KeyCode::PERIOD;
        case SDLK_SLASH:
            return KeyCode::SLASH;
        case SDLK_BACKQUOTE:
            return KeyCode::GRAVE;
        case SDLK_LEFTBRACKET:
            return KeyCode::LEFT_BRACKET;
        case SDLK_BACKSLASH:
            return KeyCode::BACKSLASH;
        case SDLK_RIGHTBRACKET:
            return KeyCode::RIGHT_BRACKET;
        case SDLK_QUOTE:
            return KeyCode::APOSTROPHE;
        case SDLK_APPLICATION:
            return KeyCode::LEFT_SUPER;
        case SDLK_LALT:
            return KeyCode::LEFT_ALT;
        case SDLK_RALT:
            return KeyCode::RIGHT_ALT;
    }

    if (keyCode < 256)
        return (KeyCode) keyCode;
    return KeyCode::UNKNOWN;
}
