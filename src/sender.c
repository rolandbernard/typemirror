#ifdef __linux__

#include <X11/extensions/XTest.h>

#include "sender.h"

static Display* display;

void initializeSender() {
    display = XOpenDisplay(NULL);
}

void sendScancode(int code, bool pressed) {
    XTestFakeKeyEvent(display, code, pressed, 0);
    XFlush(display);
}

#else

#include <windows.h>

#include "sender.h"

void initializeSender() {
}

void sendScancode(int code, bool pressed) {
}

#endif
