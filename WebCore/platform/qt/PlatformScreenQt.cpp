/*
 * Copyright (C) 2007 Apple Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "PlatformScreen.h"

#include "FloatRect.h"
#include "Frame.h"
#include "FrameView.h"
#include "Widget.h"
#include <QApplication>
#include <QDesktopWidget>

namespace WebCore {
    
int screenDepth(Widget* w)
{
    QDesktopWidget* d = QApplication::desktop();
    return d->screen(d->screenNumber(w->qwidget()))->depth();
}

int screenDepthPerComponent(Widget* w)
{
    return w->qwidget()->depth();
}

bool screenIsMonochrome(Widget* w)
{
    QDesktopWidget* d = QApplication::desktop();
    return d->screen(d->screenNumber(w->qwidget()))->numColors() < 2;
}

FloatRect screenRect(Widget* w)
{
    return static_cast<QRectF>(QApplication::desktop()->screenGeometry(w->qwidget()));
}

FloatRect screenAvailableRect(Widget* w)
{
    return static_cast<QRectF>(QApplication::desktop()->availableGeometry(w->qwidget()));
}

} // namespace WebCore
