/*
 * Copyright (C) 2004, 2005, 2006 Apple Computer, Inc.  All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef PlatformMouseEvent_h
#define PlatformMouseEvent_h

#include "IntPoint.h"
#include <wtf/Platform.h>

#if PLATFORM(MAC)
#ifdef __OBJC__
@class NSEvent;
@class NSScreen;
@class NSWindow;
#else
class NSEvent;
class NSScreen;
class NSWindow;
#endif
#endif

#if PLATFORM(WIN)
typedef struct HWND__* HWND;
typedef unsigned UINT;
typedef unsigned WPARAM;
typedef long LPARAM;
#endif

#if PLATFORM(GDK)
typedef union _GdkEvent GdkEvent;
#endif

#if PLATFORM(QT)
class QMouseEvent;
#endif

namespace WebCore {

    // These button numbers match the ones used in the DOM API, 0 through 2, except for NoButton which isn't specified.
    enum MouseButton { NoButton = -1, LeftButton, MiddleButton, RightButton };

    class PlatformMouseEvent {
    public:
        static const struct CurrentEventTag {} currentEvent;
    
        PlatformMouseEvent()
            : m_button(LeftButton)
            , m_clickCount(0)
            , m_shiftKey(false)
            , m_ctrlKey(false)
            , m_altKey(false)
            , m_metaKey(false)
        {
        }

        PlatformMouseEvent(const CurrentEventTag&);

        PlatformMouseEvent(const IntPoint& pos, const IntPoint& globalPos, MouseButton button,
                           int clickCount, bool shift, bool ctrl, bool alt, bool meta)
            : m_position(pos), m_globalPosition(globalPos), m_button(button)
            , m_clickCount(clickCount)
            , m_shiftKey(shift)
            , m_ctrlKey(ctrl)
            , m_altKey(alt)
            , m_metaKey(meta)
        {
        }

        const IntPoint& pos() const { return m_position; }
        int x() const { return m_position.x(); }
        int y() const { return m_position.y(); }
        int globalX() const { return m_globalPosition.x(); }
        int globalY() const { return m_globalPosition.y(); }
        MouseButton button() const { return m_button; }
        int clickCount() const { return m_clickCount; }
        bool shiftKey() const { return m_shiftKey; }
        bool ctrlKey() const { return m_ctrlKey; }
        bool altKey() const { return m_altKey; }
        bool metaKey() const { return m_metaKey; }

#if PLATFORM(MAC)
        PlatformMouseEvent(NSEvent*);
#endif
#if PLATFORM(WIN)
        PlatformMouseEvent(HWND, UINT, WPARAM, LPARAM, bool activatedWebView = false);
        void setClickCount(int count) { m_clickCount = count; }
        double timestamp() const { return m_timestamp; }
        bool activatedWebView() const { return m_activatedWebView; }
#endif
#if PLATFORM(GDK) 
        PlatformMouseEvent(GdkEvent*);
#endif
#if PLATFORM(QT)
        PlatformMouseEvent(QMouseEvent*, int clickCount);
#endif

    private:
        IntPoint m_position;
        IntPoint m_globalPosition;
        MouseButton m_button;
        int m_clickCount;
        bool m_shiftKey;
        bool m_ctrlKey;
        bool m_altKey;
        bool m_metaKey;

#if PLATFORM(WIN)
        double m_timestamp; // unit: seconds
        bool m_activatedWebView;
#endif
    };

#if PLATFORM(MAC)
    IntPoint globalPoint(const NSPoint& windowPoint, NSWindow *window);
    IntPoint pointForEvent(NSEvent *event);
    IntPoint globalPointForEvent(NSEvent *event);
#endif

} // namespace WebCore

#endif // PlatformMouseEvent_h
