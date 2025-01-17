/*
 * Copyright (C) 2014 Igalia S.L.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WPEInputEvents_h
#define WPEInputEvents_h

#include "WebEvent.h"
#include <wtf/Vector.h>

namespace WPE {

struct KeyboardEvent {
    struct Raw {
        uint32_t time;
        uint32_t key;
        uint32_t state;
    };

    enum Modifiers {
        Control = 1 << 0,
        Shift   = 1 << 1,
        Alt     = 1 << 2,
        Meta    = 1 << 3
    };

    uint32_t time;
    uint32_t keyCode;
    uint32_t unicode;
    bool pressed;
    uint8_t modifiers;
};

struct PointerEvent {
    enum Type : uint32_t {
        Null,
        Motion,
        Button
    };

    struct Raw {
        Type type;
        uint32_t time;
        int x;
        int y;
        uint32_t button;
        uint32_t state;
    };

    Type type;
    uint32_t time;
    double x;
    double y;
    uint32_t button;
    uint32_t state;
};

struct TouchEvent {
    enum Type : uint32_t {
        Null,
        Down,
        Motion,
        Up
    };

    struct Raw {
        Type type;
        uint32_t time;
        int id;
        int32_t x;
        int32_t y;
    };

    Vector<WebKit::WebPlatformTouchPoint> touchPoints;
    Type type;
    uint32_t time;
};

} // namespace WPE;

#endif // WPEInputEvents_h
