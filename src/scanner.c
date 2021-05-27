#ifdef __linux__

#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "scanner.h"

static Display* display;
static Window root_window;
static Window current_window;
static int revert;

void initializeScanner() {
    display = XOpenDisplay(NULL);
    root_window = DefaultRootWindow(display);
    XGetInputFocus (display, &current_window, &revert);
    XSelectInput(display, current_window, KeyPressMask|KeyReleaseMask|FocusChangeMask);
}

bool getNextScancode(int* code) {
    XEvent event;
    XNextEvent(display, &event);
    switch (event.type) {
        case FocusOut:
            if (current_window != root_window) {
                XSelectInput(display, current_window, 0);
            }
            XGetInputFocus(display, &current_window, &revert);
            if (current_window == PointerRoot) {
                current_window = root_window;
            }
            XSelectInput(display, current_window, KeyPressMask|KeyReleaseMask|FocusChangeMask);
            *code = -1;
            return false;
        case KeyPress:
            *code = event.xkey.keycode;
            return true;
        case KeyRelease:
            *code = event.xkey.keycode;
            return false;
        default:
            *code = -1;
            return false;
    }
}

#else

#include <unistd.h>
#include <stdio.h>
#include <windows.h>

#include "scanner.h"

static HHOOK hook;
static volatile int start = 0;
static volatile int end = 0;
static volatile int keys[1024];
static volatile bool pressed[1024];


LRESULT CALLBACK hookCallback(int code, WPARAM wParam, LPARAM lParam) {
    KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT*)lParam;
    if (code >= 0) {
        if (wParam == WM_KEYDOWN) {  
            keys[end] = kbd->scanCode;
            pressed[end] = true;
            end = (end + 1) % 1024;
        } else if (wParam == WM_KEYUP) {
            keys[end] = kbd->scanCode;
            pressed[end] = false;
            end = (end + 1) % 1024;
        }
    }
    return CallNextHookEx(hook, code, wParam, lParam);
}

unsigned int hookThread(void* data) {
    hook = SetWindowsHookEx(WH_KEYBOARD_LL, hookCallback, NULL, 0);
    if (hook == NULL) {
        MessageBox(NULL, "Failed to install hook!", "Error", MB_ICONERROR);
    }
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
        DispatchMessage(&msg);
    }
}

void initializeScanner() {
    ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 0);
    CreateThread(NULL, 0, hookThread, NULL, 0, NULL);
}

bool getNextScancode(int* code) {
    if (start == end) {
        usleep(100000);
        *code = -1;
        return false;
    }
    *code = keys[start] + 8;
    if (*code == 80) {
        *code = 111;
    } else if (*code == 85) {
        *code = 114;
    } else if (*code == 88) {
        *code = 116;
    } else if (*code == 83) {
        *code = 113;
    }
    bool ret = pressed[start];
    start = (start + 1) % 1024;
    return ret;
}

#endif
