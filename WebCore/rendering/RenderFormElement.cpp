/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2005, 2006 Apple Computer, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include "config.h"
#include "RenderFormElement.h"

#include "EventNames.h"
#include "HTMLGenericFormElement.h"
#include "PlatformMouseEvent.h"
#include "Widget.h"

namespace WebCore {

using namespace EventNames;

RenderFormElement::RenderFormElement(HTMLGenericFormElement* element)
    : RenderWidget(element)
{
    setInline(true);
}

RenderFormElement::~RenderFormElement()
{
}

short RenderFormElement::baselinePosition(bool f, bool isRootLineBox) const
{
    return marginTop() + widget()->baselinePosition(m_height);
}

void RenderFormElement::setStyle(RenderStyle* newStyle)
{
    RenderWidget::setStyle(newStyle);

    // Do not paint a background or border for Aqua form elements
    setShouldPaintBackgroundOrBorder(false);

    m_widget->setFont(style()->font());
}

void RenderFormElement::updateFromElement()
{
    m_widget->setEnabled(!static_cast<HTMLGenericFormElement*>(node())->disabled());
}

void RenderFormElement::layout()
{
    ASSERT(needsLayout());
    ASSERT(minMaxKnown());

    // minimum height
    m_height = 0;

    calcWidth();
    calcHeight();

    setNeedsLayout(false);
}

void RenderFormElement::clicked(Widget*)
{
    RenderArena* arena = ref();
    PlatformMouseEvent event(PlatformMouseEvent::currentEvent);
    if (node())
        static_cast<EventTargetNode*>(node())->dispatchMouseEvent(event, clickEvent, event.clickCount());
    deref(arena);
}

HorizontalAlignment RenderFormElement::textAlignment() const
{
    switch (style()->textAlign()) {
        case LEFT:
        case KHTML_LEFT:
            return AlignLeft;
        case RIGHT:
        case KHTML_RIGHT:
            return AlignRight;
        case CENTER:
        case KHTML_CENTER:
            return AlignHCenter;
        case JUSTIFY:
            // Just fall into the auto code for justify.
        case TAAUTO:
            return style()->direction() == RTL ? AlignRight : AlignLeft;
    }
    ASSERT_NOT_REACHED();
    return AlignLeft;
}

} // namespace WebCore
