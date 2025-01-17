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

#ifndef WPEInputHandler_h
#define WPEInputHandler_h

#include "WPEInputEvents.h"
#include <array>
#include <wtf/Vector.h>
#include <xkbcommon/xkbcommon.h>

namespace WPE {

class View;

class InputHandler {
public:
    static InputHandler* create(View& view)
    {
        return new InputHandler(view);
    }
    ~InputHandler();

    void handleKeyboardKey(KeyboardEvent::Raw);

    void handlePointerEvent(PointerEvent::Raw);

    void handleTouchDown(TouchEvent::Raw);
    void handleTouchUp(TouchEvent::Raw);
    void handleTouchMotion(TouchEvent::Raw);

private:
    InputHandler(View&);

    View& m_view;

    struct xkb_keymap* m_xkbKeymap;
    struct xkb_state* m_xkbState;

    struct Modifiers {
        xkb_mod_index_t ctrl;
        xkb_mod_index_t shift;

        uint32_t effective;
    } m_modifiers;

    struct Pointer {
        double x;
        double y;
    } m_pointer;

    void dispatchTouchEvent(int id);
    std::array<TouchEvent::Raw, 10> m_touchEvents;
};

} // namespace WPE

#endif // WPEInputHandler_h
