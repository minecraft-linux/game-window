#include "window_sdl3.h"
#include "game_window_manager.h"

#include <codecvt>
#include <iomanip>
#include <thread>

#include <math.h>
#include <SDL3/SDL.h>

SDL3GameWindow::SDL3GameWindow(const std::string& title, int width, int height, GraphicsApi api) :
        GameWindow(title, width, height, api), width(width), height(height), windowedWidth(width), windowedHeight(height) {
    if(api == GraphicsApi::OPENGL_ES2) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    }
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    window = SDL_CreateWindow(title.data(), width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if(window == nullptr) {
        // Throw an exception, otherwise it would crash due to a nullptr without any information
        const char* error = SDL_GetError();
        throw std::runtime_error(error == nullptr ? "SDL3 failed to create a window without any error message" : error);
    }
    context = SDL_GL_CreateContext(window);
    if(context == nullptr) {
        SDL_DestroyWindow(window);
        const char* error = SDL_GetError();
        throw std::runtime_error(error == nullptr ? "SDL3 failed to create a window context without any error message" : error);
    }
    SDL_GL_MakeCurrent(window, context);

    setRelativeScale();
}

void SDL3GameWindow::makeCurrent(bool c) {
    SDL_GL_MakeCurrent(window, c ? context : nullptr);
}

SDL3GameWindow::~SDL3GameWindow() {
    if(window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

void SDL3GameWindow::setIcon(std::string const& iconPath) {
    
}

void SDL3GameWindow::setRelativeScale() {
    int fx, fy;
    SDL_GetWindowSizeInPixels(window, &fx, &fy);

    int wx, wy;
    SDL_GetWindowSize(window, &wx, &wy);

    relativeScale = (int) floor(((fx / wx) + (fy / wy)) / 2);
    // Update window size to match content size mismatch
    width = fx;
    height = fy;
    resized = true;
}

int SDL3GameWindow::getRelativeScale() const {
    return relativeScale;
}

void SDL3GameWindow::getWindowSize(int& width, int& height) const {
    width = this->width;
    height = this->height;
}

void SDL3GameWindow::show() {
    SDL_ShowWindow(window);
}

void SDL3GameWindow::close() {
    if(window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

static GamepadButtonId getKeyGamePad(int btn) {
    switch (btn)
    {
    case SDL_GAMEPAD_BUTTON_SOUTH:
        return GamepadButtonId::A;
    case SDL_GAMEPAD_BUTTON_EAST:
        return GamepadButtonId::B;
    case SDL_GAMEPAD_BUTTON_WEST:
        return GamepadButtonId::X;
    case SDL_GAMEPAD_BUTTON_NORTH:
        return GamepadButtonId::Y;
    case SDL_GAMEPAD_BUTTON_BACK:
        return GamepadButtonId::BACK;
    case SDL_GAMEPAD_BUTTON_GUIDE:
        return GamepadButtonId::GUIDE;
    case SDL_GAMEPAD_BUTTON_START:
        return GamepadButtonId::START;
    case SDL_GAMEPAD_BUTTON_LEFT_STICK:
        return GamepadButtonId::LEFT_STICK;
    case SDL_GAMEPAD_BUTTON_RIGHT_STICK:
        return GamepadButtonId::RIGHT_STICK;
    case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
        return GamepadButtonId::LB;
    case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
        return GamepadButtonId::RB;
    case SDL_GAMEPAD_BUTTON_DPAD_UP:
        return GamepadButtonId::DPAD_UP;
    case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
        return GamepadButtonId::DPAD_DOWN;
    case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
        return GamepadButtonId::DPAD_LEFT;
    case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
        return GamepadButtonId::DPAD_RIGHT;
    default:
        return GamepadButtonId::UNKNOWN;
    }
}

static GamepadAxisId getAxisGamepad(int btn) {
    switch (btn)
    {
    case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
        return GamepadAxisId::LEFT_TRIGGER;
    case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
        return GamepadAxisId::RIGHT_TRIGGER;
    case SDL_GAMEPAD_AXIS_LEFTX:
        return GamepadAxisId::LEFT_X;
    case SDL_GAMEPAD_AXIS_LEFTY:
        return GamepadAxisId::LEFT_Y;
    case SDL_GAMEPAD_AXIS_RIGHTX:
        return GamepadAxisId::RIGHT_X;
    case SDL_GAMEPAD_AXIS_RIGHTY:
        return GamepadAxisId::RIGHT_Y;
    default:
        return GamepadAxisId::UNKNOWN;
    }
}

static int getMouseButton(int btn) {
    switch (btn)
    {
    case SDL_BUTTON_LEFT:
        return 1;
    case SDL_BUTTON_RIGHT:
        return 2;
    case SDL_BUTTON_MIDDLE:
        return 3;
    case SDL_BUTTON_X1:
        return 4;
    case SDL_BUTTON_X2:
        return 5;
    default:
        return 0;
    }
}

void SDL3GameWindow::pollEvents() {
    SDL_Event ev;
    while(SDL_PollEvent(&ev)) {
        switch (ev.type)
        {
        case SDL_EVENT_MOUSE_MOTION:
            if(!SDL_GetRelativeMouseMode()) {
                onMousePosition(ev.motion.x, ev.motion.y);
            } else {
                onMouseRelativePosition(ev.motion.xrel, ev.motion.yrel);
            }
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            onMouseScroll(ev.wheel.mouseX, ev.wheel.mouseY, ev.wheel.x, ev.wheel.y);
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            onMouseButton(ev.button.x, ev.button.y, getMouseButton(ev.button.button), ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? MouseButtonAction::PRESS : MouseButtonAction::RELEASE);
            break;
        case SDL_EVENT_FINGER_DOWN:
            onTouchStart(ev.tfinger.fingerId, ev.tfinger.x, ev.tfinger.y);
            break;
        case SDL_EVENT_FINGER_UP:
            onTouchEnd(ev.tfinger.fingerId, ev.tfinger.x, ev.tfinger.y);
            break;
        case SDL_EVENT_FINGER_MOTION:
            onTouchUpdate(ev.tfinger.fingerId, ev.tfinger.x, ev.tfinger.y);
            break;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            if(SDL_TextInputActive() && ev.type == SDL_EVENT_KEY_DOWN) {
                if(ev.key.keysym.sym == SDLK_BACKSPACE) {
                    onKeyboardText("\b");
                } else if(ev.key.keysym.sym == SDLK_DELETE) {
                    onKeyboardText("\x7F");
                } else if(ev.key.keysym.sym == SDLK_RETURN) {
                    onKeyboardText("\n");
                }
            }
            onKeyboard(getKeyMinecraft(ev.key.keysym.sym), ev.type == SDL_EVENT_KEY_DOWN ? ev.key.repeat ? KeyAction::REPEAT : KeyAction::PRESS : KeyAction::RELEASE );
            break;
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            onGamepadButton(ev.gbutton.which, getKeyGamePad(ev.gbutton.button), ev.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
            break;
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            onGamepadAxis(ev.gbutton.which, getAxisGamepad(ev.gbutton.button), ev.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
            break;
        case SDL_EVENT_GAMEPAD_ADDED:
        case SDL_EVENT_GAMEPAD_REMOVED:
            onGamepadState(ev.gdevice.which, ev.type == SDL_EVENT_GAMEPAD_ADDED);
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            width = ev.window.data1;
            height = ev.window.data2;
            onWindowSizeChanged(ev.window.data1, ev.window.data2);
            break;
        case SDL_EVENT_TEXT_INPUT:
            onKeyboardText(ev.text.text ? ev.text.text : "");
            break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            onClose();
        default:
            break;
        }
    }
}

void SDL3GameWindow::setCursorDisabled(bool disabled) {
    SDL_SetRelativeMouseMode(disabled);
}

void SDL3GameWindow::setFullscreen(bool fullscreen) {
    SDL_SetWindowFullscreen(window, fullscreen);
}

void SDL3GameWindow::setClipboardText(std::string const &text) {
    SDL_SetClipboardText(text.data());
}

void SDL3GameWindow::swapBuffers() {
    SDL_GL_SwapWindow(window);
}

void SDL3GameWindow::setSwapInterval(int interval) {
    SDL_GL_SetSwapInterval(interval);
}

void SDL3GameWindow::startTextInput() {
    SDL_StartTextInput();
}

void SDL3GameWindow::stopTextInput() {
    SDL_StopTextInput();
}

KeyCode SDL3GameWindow::getKeyMinecraft(int keyCode) {
    if (keyCode >= SDLK_F1 && keyCode <= SDLK_F12)
        return (KeyCode) (keyCode - SDLK_F1 + (int) KeyCode::FN1);
    if (keyCode >= SDLK_KP_0 && keyCode <= SDLK_KP_9)
        return (KeyCode) (keyCode - SDLK_KP_0 + (int) KeyCode::NUMPAD_0);
    if (keyCode >= SDLK_a && keyCode <= SDLK_z)
        return (KeyCode) (keyCode - SDLK_a + (int) KeyCode::A);
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
        case SDLK_LGUI:
            return KeyCode::LEFT_SUPER;
        case SDLK_RGUI:
            return KeyCode::RIGHT_SUPER;
        case SDLK_LALT:
            return KeyCode::LEFT_ALT;
        case SDLK_RALT:
            return KeyCode::RIGHT_ALT;
        case SDLK_KP_ENTER:
            return KeyCode::ENTER;
        case SDLK_KP_MINUS:
            return KeyCode::NUMPAD_SUBTRACT;
        case SDLK_KP_MULTIPLY:
            return KeyCode::NUMPAD_MULTIPLY;
        case SDLK_KP_PLUS:
            return KeyCode::NUMPAD_ADD;
        case SDLK_KP_DIVIDE:
            return KeyCode::NUMPAD_DIVIDE;
        case SDLK_KP_DECIMAL:
            return KeyCode::NUMPAD_DECIMAL;
    }
    if (keyCode < 256)
        return (KeyCode) keyCode;
    return KeyCode::UNKNOWN;
}