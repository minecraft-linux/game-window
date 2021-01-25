#include "window_sdl2.h"
#include "game_window_manager.h"
#include <string.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <SDL2/SDL.h>

#include <linux/fb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <algorithm>

SDL2GameWindow::SDL2GameWindow(const std::string& title, int width, int height, GraphicsApi api) :
        GameWindow(title, width, height, api) {

    currentGameWindow = this;

    initFrameBuffer();
    initEGL();
    initSDL();
    initCursor();
}

SDL2GameWindow::~SDL2GameWindow() {
    munmap (fb.mmap, fb.mmap_size);
    ::close(fb.fd);
    delete cursor.img;

    eglDestroySurface(egl.display, egl.surface);
    eglDestroyContext(egl.display, egl.context);
    eglMakeCurrent(egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    SDL_Quit();
}

void SDL2GameWindow::abortMsg(const char *msg)
{
    fflush(stdout);
    fprintf(stderr, "Fatal Error: %s\n", msg);
    exit(1);
}

void SDL2GameWindow::initFrameBuffer() {
    // map framebuffer for drawing operations and to get dimensions
    fb.fd = open("/dev/fb0", O_RDWR);
    if (fb.fd < 0)
        abortMsg("Failed to open fb0 for cursor drawing operations!");

    struct fb_var_screeninfo vinfo;

    ioctl (fb.fd, FBIOGET_VSCREENINFO, &vinfo);

    fb.w = vinfo.xres;
    fb.h = vinfo.yres;
    int fb_bpp = vinfo.bits_per_pixel;
    int fb_bytes = fb_bpp / 8;

    fb.mmap_size = fb.w * fb.h * fb_bytes * 2; // double buffered so x2 size

    fb.mmap = (bgra_pixel*)mmap (0, fb.mmap_size,
            PROT_READ | PROT_WRITE, MAP_SHARED, fb.fd, (off_t)0);

    // assume backbuffer is the 2nd half of the memory map - will sync it up after EGL initialised
    fb.frontbuff = fb.mmap;
    fb.backbuff = &fb.mmap[fb.h * fb.w];
}

void SDL2GameWindow::initEGL() {
    // init EGL diplay, window, surface and context
    EGLConfig config;
    EGLint num_config;
    EGLint const config_attribute_list[] = {
        EGL_RED_SIZE, 1,
        EGL_GREEN_SIZE, 1,
        EGL_BLUE_SIZE, 1,
        EGL_ALPHA_SIZE, 1,
        EGL_DEPTH_SIZE, 1,
        EGL_STENCIL_SIZE, 8,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
    EGLint const context_attribute_list[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    egl.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglGetError() != EGL_SUCCESS)
        abortMsg("EGL error when getting display!");

    if (! eglInitialize(egl.display, NULL, NULL))
        abortMsg("Failed to initialize EGL Display!");

    eglChooseConfig(egl.display, config_attribute_list, &config, 1, &num_config);
    if (eglGetError() != EGL_SUCCESS)
        abortMsg("EGL error when choosing config!");

    if (! eglBindAPI(EGL_OPENGL_ES_API))
        abortMsg("Failed to bind EGL API!\n");

    egl.context = eglCreateContext(egl.display, config, EGL_NO_CONTEXT, context_attribute_list);
    if (! egl.context)
        abortMsg("Failed to create EGL Context!");

    egl.surface = eglCreateWindowSurface(egl.display, config, NULL, NULL);
    if (egl.surface == EGL_NO_SURFACE)
        abortMsg("Failed to create EGL Window Surface!");

    if (! eglMakeCurrent(egl.display, egl.surface, egl.surface, egl.context))
        abortMsg("Failed to make EGL Context current!");

    // find the backbuffer address in framebuffer in case it is out of sync
    // first ensure both surfaces are black
    clearColour(0.0);
    clearColour(0.0);
    // then clear to mid-grey and test where it landed after swapping
    clearColour(0.5);
    if(fb.backbuff->r == 128) // back is actually front!
        std::swap(fb.frontbuff, fb.backbuff); // synchronise
}

void SDL2GameWindow::clearColour(float shade) {
    glClearColor(shade, shade, shade, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFinish();
    eglSwapBuffers(egl.display, egl.surface);
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

void SDL2GameWindow::initCursor() {
    const char mcw = 16; // raw mouse cursor image width
    // rgba mouse cursor image data
    const char mci[16][16][4] = {
    {{6,61,51,255}, {6,61,51,255}, {6,61,51,255}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
    {{2,32,26,255}, {165,253,240,255}, {165,253,240,255}, {6,61,51,255}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
    {{2,32,26,255}, {165,253,240,255}, {38,200,174,255}, {165,253,240,255}, {6,61,51,255}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {2,32,26,255}, {165,253,240,255}, {38,200,174,255}, {165,253,240,255}, {6,61,51,255}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {2,32,26,255}, {47,236,204,255}, {38,200,174,255}, {165,253,240,255}, {6,61,51,255}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {2,32,26,255}, {47,236,204,255}, {38,200,174,255}, {165,253,240,255}, {6,61,51,255}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {2,32,26,255}, {47,236,204,255}, {38,200,174,255}, {47,236,204,255}, {6,61,51,255}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {6,61,51,255}, {6,61,51,255}, {0,0,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {2,32,26,255}, {47,236,204,255}, {38,200,174,255}, {47,236,204,255}, {6,61,51,255}, {0,0,0,0}, {6,61,51,255}, {13,99,84,255}, {6,61,51,255}, {0,0,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {2,32,26,255}, {47,236,204,255}, {24,139,120,255}, {47,236,204,255}, {2,32,26,255}, {24,139,120,255}, {6,61,51,255}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {2,32,26,255}, {47,236,204,255}, {13,99,84,255}, {24,139,120,255}, {24,139,120,255}, {6,61,51,255}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {2,32,26,255}, {6,61,51,255}, {13,99,84,255}, {6,61,51,255}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {2,32,26,255}, {6,61,51,255}, {6,61,51,255}, {2,32,26,255}, {104,77,24,255}, {71,51,13,255}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {2,32,26,255}, {6,61,51,255}, {2,32,26,255}, {2,32,26,255}, {0,0,0,0}, {35,24,3,255}, {138,103,34,255}, {71,51,13,255}, {0,0,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {2,32,26,255}, {2,32,26,255}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {35,24,3,255}, {104,77,24,255}, {6,61,51,255}, {6,61,51,255}},
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {2,32,26,255}, {13,99,84,255}, {6,61,51,255}},
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {2,32,26,255}, {2,32,26,255}, {2,32,26,255}}};

    // calculate scale factor to make mouse cursor image 1/8th of screen height
    const char mcs = fb.h / 8 / mcw;
    cursor.size = mcw * mcs; // store width for drawing operations later

    // allocate storage for scaled cursor image
    cursor.img =  new bgra_pixel*[mcw*mcs]; // allocate rows
    // allocate columns
    for(int row=0; row<(mcw*mcs); row++) {
        cursor.img[row] = new bgra_pixel[mcw * mcs];
    }

    // scale image into storage, remapping from rgba to bgra in the process
    for(int u=0; u<mcw; u++) {
        for(int v=0; v<mcw; v++) {
            bgra_pixel pix = { .b = mci[u][v][2], .g = mci[u][v][1], .r = mci[u][v][0], .a = mci[u][v][3] }; // remap raw to pixel
            // draw pixel as scaled square
            for(int x=u*mcs; x<(u*mcs)+mcs; x++) {
                for(int y=v*mcs; y<(v*mcs)+mcs; y++) {
                    cursor.img[x][y] = pix;
                }
            }
        }
    }

    cursor.inuse = true; // assume mouse is the input device until proven otherwise
    setCursorDisabled(false);
}

void SDL2GameWindow::drawCursor() {
    // clip and draw pre-scaled cursor image to back half of framebuffer at mouse position
    int mx = cursor.x, my = cursor.y;

    int clip_w = fb.w - mx;
    if(clip_w > cursor.size)
        clip_w = cursor.size;

    int clip_h = fb.h - my;
    if(clip_h > cursor.size)
        clip_h = cursor.size;

    for(int u=0; u<clip_w; u++) {
        for(int v=0; v<clip_h; v++) {
            if(cursor.img[u][v].a == 255)
                fb.backbuff[(my+v)*fb.w + (mx+u)] = cursor.img[u][v];
        }
    }
}

void SDL2GameWindow::setIcon(std::string const& iconPath) {
    // NOOP - borderless window
}

void SDL2GameWindow::getWindowSize(int& width, int& height) const {
    width = fb.w;
    height = fb.h;
}

void SDL2GameWindow::show() {
    // NOOP - borderless window
}

void SDL2GameWindow::close() {
    // NOOP - borderless window
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
    currentGameWindow->onGamepadState(0, (cdeviceevent->type == SDL_CONTROLLERDEVICEADDED));
}

void SDL2GameWindow::handleControllerAxisEvent(SDL_ControllerAxisEvent *caxisevent) {
    cursor.inuse = false; // gampepad has taken priority over mouse so hide the cursor
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
    currentGameWindow->onGamepadAxis(0, axis, deflection);
}

void SDL2GameWindow::handleControllerButtonEvent(SDL_ControllerButtonEvent *cbuttonevent) {
    cursor.inuse = false; // gampepad has taken priority over mouse so hide the cursor

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

    currentGameWindow->onGamepadButton(0, btn, (cbuttonevent->state == SDL_PRESSED));
}

void SDL2GameWindow::handleMouseWheelEvent(SDL_MouseWheelEvent *wheelevent) {
    if (wheelevent->x < 0)
        currentGameWindow->onMouseScroll(cursor.x, cursor.y, -1.0, 0.0);
    else if (wheelevent->x > 0)
        currentGameWindow->onMouseScroll(cursor.x, cursor.y, 1.0, 0.0);
    else if (wheelevent->y < 0)
        currentGameWindow->onMouseScroll(0.0, 0.0, 0.0, -1);
    else if (wheelevent->y > 0)
        currentGameWindow->onMouseScroll(0.0, 0.0, 0.0, 1);
}

void SDL2GameWindow::handleMouseMotionEvent(SDL_MouseMotionEvent *motionevent) {
    if (cursor.hidden) {
        currentGameWindow->onMouseRelativePosition(motionevent->xrel, motionevent->yrel);
    }
    else {
        cursor.inuse = true; // mouse has taken priority as input, so draw the cursor as required
        // provide own AbsolutePosition as SDL won't do this without a window
        int tx = cursor.x + motionevent->xrel;
        cursor.x = tx < 0 ? 0 : (tx >= fb.w ? fb.w-1 : tx);
        int ty = cursor.y + motionevent->yrel;
        cursor.y = ty < 0 ? 0 : (ty >= fb.h ? fb.h-1 : ty);
        currentGameWindow->onMousePosition(cursor.x, cursor.y);
    }
}

void SDL2GameWindow::handleMouseClickEvent(SDL_MouseButtonEvent *clickevent) {
    currentGameWindow->onMouseButton(cursor.x, cursor.y, clickevent->button, (clickevent->state == SDL_PRESSED ? MouseButtonAction::PRESS : MouseButtonAction::RELEASE));
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

    currentGameWindow->onKeyboard(key, action);
}

void SDL2GameWindow::setCursorDisabled(bool disabled) {
    if (!disabled) {
        // warp mouse to center for initial display
        cursor.x = fb.w / 2;
        cursor.y = fb.h / 2;
    }

    cursor.hidden = disabled;
}

void SDL2GameWindow::setFullscreen(bool fullscreen) {
    // NOOP - always fullscreen
}

void SDL2GameWindow::setClipboardText(std::string const &text) {
    // NOOP - nowhere to cut/paste to/from without a desktop and other applications
}

void SDL2GameWindow::swapBuffers() {
    if (!cursor.hidden && cursor.inuse) {
        glFinish();
        drawCursor();
    }
    std::swap(fb.frontbuff, fb.backbuff);
    eglSwapBuffers(egl.display, egl.surface);
}

void SDL2GameWindow::setSwapInterval(int interval) {
    eglSwapInterval(egl.display, interval);
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
