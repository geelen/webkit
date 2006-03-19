/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Peter Kelly (pmk@post.com)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
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
 */

#include "config.h"
#include "dom_elementimpl.h"

#include "ExceptionCode.h"
#include "Frame.h"
#include "TextImpl.h"
#include "css_stylesheetimpl.h"
#include "css_valueimpl.h"
#include "cssstyleselector.h"
#include "cssvalues.h"
#include "dom2_eventsimpl.h"
#include "dom_xmlimpl.h"
#include "htmlnames.h"
#include "htmlparser.h"
#include "render_canvas.h"
#include <qtextstream.h>

namespace WebCore {

using namespace HTMLNames;

struct MappedAttributeKey {
    uint16_t type;
    DOMStringImpl* name;
    DOMStringImpl* value;
    MappedAttributeKey(MappedAttributeEntry t, DOMStringImpl* n, DOMStringImpl* v)
        : type(t), name(n), value(v) { }
};

static inline bool operator==(const MappedAttributeKey& a, const MappedAttributeKey& b)
    { return a.type == b.type && a.name == b.name && a.value == b.value; } 

struct MappedAttributeKeyTraits {
    typedef MappedAttributeKey TraitType;
    static const bool emptyValueIsZero = true;
    static const bool needsDestruction = false;
    static MappedAttributeKey emptyValue() { return MappedAttributeKey(eNone, 0, 0); }
    static MappedAttributeKey deletedValue() { return MappedAttributeKey(eLastEntry, 0, 0); }
};

struct MappedAttributeHash {
    static unsigned hash(const MappedAttributeKey&);
    static bool equal(const MappedAttributeKey& a, const MappedAttributeKey& b) { return a == b; }
};

typedef HashMap<MappedAttributeKey, CSSMappedAttributeDeclarationImpl*, MappedAttributeHash, MappedAttributeKeyTraits> MappedAttributeDecls;

static MappedAttributeDecls* mappedAttributeDecls = 0;

AttributeImpl* AttributeImpl::clone(bool) const
{
    return new AttributeImpl(m_name, m_value);
}

PassRefPtr<AttrImpl> AttributeImpl::createAttrImplIfNeeded(ElementImpl* e)
{
    RefPtr<AttrImpl> r(m_impl);
    if (!r) {
        r = new AttrImpl(e, e->getDocument(), this);
        r->createTextChild();
    }
    return r.release();
}

AttrImpl::AttrImpl(ElementImpl* element, DocumentImpl* docPtr, AttributeImpl* a)
    : ContainerNodeImpl(docPtr),
      m_element(element),
      m_attribute(a),
      m_ignoreChildrenChanged(0)
{
    assert(!m_attribute->m_impl);
    m_attribute->m_impl = this;
    m_specified = true;
}

AttrImpl::~AttrImpl()
{
    assert(m_attribute->m_impl == this);
    m_attribute->m_impl = 0;
}

void AttrImpl::createTextChild()
{
    assert(refCount());
    if (!m_attribute->value().isEmpty()) {
        ExceptionCode ec = 0;
        m_ignoreChildrenChanged++;
        appendChild(getDocument()->createTextNode(m_attribute->value().impl()), ec);
        m_ignoreChildrenChanged--;
    }
}

DOMString AttrImpl::nodeName() const
{
    return name();
}

NodeImpl::NodeType AttrImpl::nodeType() const
{
    return ATTRIBUTE_NODE;
}

const AtomicString& AttrImpl::localName() const
{
    return m_attribute->localName();
}

const AtomicString& AttrImpl::namespaceURI() const
{
    return m_attribute->namespaceURI();
}

const AtomicString& AttrImpl::prefix() const
{
    return m_attribute->prefix();
}

void AttrImpl::setPrefix(const AtomicString &_prefix, ExceptionCode& ec)
{
    checkSetPrefix(_prefix, ec);
    if (ec)
        return;

    m_attribute->setPrefix(_prefix);
}

DOMString AttrImpl::nodeValue() const
{
    return value();
}

void AttrImpl::setValue( const String& v, ExceptionCode& ec)
{
    ec = 0;

    // do not interprete entities in the string, its literal!

    // NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly
    if (isReadOnly()) {
        ec = NO_MODIFICATION_ALLOWED_ERR;
        return;
    }

    // ### what to do on 0 ?
    if (v.isNull()) {
        ec = DOMSTRING_SIZE_ERR;
        return;
    }

    int e = 0;
    m_ignoreChildrenChanged++;
    removeChildren();
    appendChild(getDocument()->createTextNode(v.impl()), e);
    m_ignoreChildrenChanged--;
    
    m_attribute->setValue(v.impl());
    if (m_element)
        m_element->attributeChanged(m_attribute.get());
}

void AttrImpl::setNodeValue(const String& v, ExceptionCode& ec)
{
    // NO_MODIFICATION_ALLOWED_ERR: taken care of by setValue()
    setValue(v, ec);
}

PassRefPtr<NodeImpl> AttrImpl::cloneNode(bool /*deep*/)
{
    RefPtr<AttrImpl> clone = new AttrImpl(0, getDocument(), m_attribute->clone());
    cloneChildNodes(clone.get());
    return clone.release();
}

// DOM Section 1.1.1
bool AttrImpl::childTypeAllowed(NodeType type)
{
    switch (type) {
        case TEXT_NODE:
        case ENTITY_REFERENCE_NODE:
            return true;
        default:
            return false;
    }
}

void AttrImpl::childrenChanged()
{
    NodeImpl::childrenChanged();
    
    if (m_ignoreChildrenChanged > 0)
        return;
    
    // FIXME: We should include entity references in the value
    
    DOMString val = "";
    for (NodeImpl *n = firstChild(); n; n = n->nextSibling()) {
        if (n->isTextNode())
            val += static_cast<TextImpl *>(n)->data();
    }
    
    m_attribute->setValue(val.impl());
    if (m_element)
        m_element->attributeChanged(m_attribute.get());
}

DOMString AttrImpl::toString() const
{
    DOMString result;

    result += nodeName();

    // FIXME: substitute entities for any instances of " or ' --
    // maybe easier to just use text value and ignore existing
    // entity refs?

    if (firstChild() != NULL) {
        result += "=\"";

        for (NodeImpl *child = firstChild(); child != NULL; child = child->nextSibling()) {
            result += child->toString();
        }
        
        result += "\"";
    }

    return result;
}

// -------------------------------------------------------------------------

#ifndef NDEBUG
struct ElementImplCounter 
{ 
    static int count; 
    ~ElementImplCounter() { /* if (count != 0) fprintf(stderr, "LEAK: %d ElementImpl\n", count); */ } 
};
int ElementImplCounter::count;
static ElementImplCounter elementImplCounter;
#endif NDEBUG

ElementImpl::ElementImpl(const QualifiedName& qName, DocumentImpl *doc)
    : ContainerNodeImpl(doc), m_tagName(qName)
{
#ifndef NDEBUG
    ++ElementImplCounter::count;
#endif
}

ElementImpl::~ElementImpl()
{
#ifndef NDEBUG
    --ElementImplCounter::count;
#endif
    if (namedAttrMap)
        namedAttrMap->detachFromElement();
}

PassRefPtr<NodeImpl> ElementImpl::cloneNode(bool deep)
{
    ExceptionCode ec = 0;
    RefPtr<ElementImpl> clone = getDocument()->createElementNS(namespaceURI(), nodeName(), ec);
    assert(!ec);
    
    // clone attributes
    if (namedAttrMap)
        *clone->attributes() = *namedAttrMap;

    clone->copyNonAttributeProperties(this);
    
    if (deep)
        cloneChildNodes(clone.get());

    return clone.release();
}

void ElementImpl::removeAttribute(const QualifiedName& name, ExceptionCode& ec)
{
    if (namedAttrMap) {
        namedAttrMap->removeNamedItem(name, ec);
        if (ec == NOT_FOUND_ERR)
            ec = 0;
    }
}

void ElementImpl::setAttribute(const QualifiedName& name, const DOMString &value)
{
    ExceptionCode ec = 0;
    setAttribute(name, value.impl(), ec);
}

// Virtual function, defined in base class.
NamedAttrMapImpl *ElementImpl::attributes() const
{
    return attributes(false);
}

NamedAttrMapImpl* ElementImpl::attributes(bool readonly) const
{
    updateStyleAttributeIfNeeded();
    if (!readonly && !namedAttrMap)
        createAttributeMap();
    return namedAttrMap.get();
}

NodeImpl::NodeType ElementImpl::nodeType() const
{
    return ELEMENT_NODE;
}

const AtomicStringList* ElementImpl::getClassList() const
{
    return 0;
}

const AtomicString& ElementImpl::getIDAttribute() const
{
    return namedAttrMap ? namedAttrMap->id() : nullAtom;
}

bool ElementImpl::hasAttribute(const QualifiedName& name) const
{
    return hasAttributeNS(name.namespaceURI(), name.localName());
}

const AtomicString& ElementImpl::getAttribute(const QualifiedName& name) const
{
    if (name == styleAttr)
        updateStyleAttributeIfNeeded();

    if (namedAttrMap)
        if (AttributeImpl* a = namedAttrMap->getAttributeItem(name))
            return a->value();

    return nullAtom;
}

void ElementImpl::scrollIntoView(bool alignToTop) 
{
    IntRect bounds = this->getRect();    
    if (renderer()) {
        // Align to the top / bottom and to the closest edge.
        if (alignToTop)
            renderer()->enclosingLayer()->scrollRectToVisible(bounds, RenderLayer::gAlignToEdgeIfNeeded, RenderLayer::gAlignTopAlways);
        else
            renderer()->enclosingLayer()->scrollRectToVisible(bounds, RenderLayer::gAlignToEdgeIfNeeded, RenderLayer::gAlignBottomAlways);
    }
}

void ElementImpl::scrollIntoViewIfNeeded(bool centerIfNeeded)
{
    IntRect bounds = this->getRect();    
    if (renderer()) {
        if (centerIfNeeded)
            renderer()->enclosingLayer()->scrollRectToVisible(bounds, RenderLayer::gAlignCenterIfNeeded, RenderLayer::gAlignCenterIfNeeded);
        else
            renderer()->enclosingLayer()->scrollRectToVisible(bounds, RenderLayer::gAlignToEdgeIfNeeded, RenderLayer::gAlignToEdgeIfNeeded);
    }
}

static inline bool inHTMLDocument(const ElementImpl* e)
{
    return e && e->getDocument()->isHTMLDocument();
}

const AtomicString& ElementImpl::getAttribute(const String& name) const
{
    String localName = inHTMLDocument(this) ? name.lower() : name;
    return getAttribute(QualifiedName(nullAtom, localName.impl(), nullAtom));
}

const AtomicString& ElementImpl::getAttributeNS(const String& namespaceURI, const String& localName) const
{
    return getAttribute(QualifiedName(nullAtom, localName.impl(), namespaceURI.impl()));
}

void ElementImpl::setAttribute(const String& name, const String& value, ExceptionCode& ec)
{
    if (!DocumentImpl::isValidName(name)) {
        ec = INVALID_CHARACTER_ERR;
        return;
    }
    String localName = inHTMLDocument(this) ? name.lower() : name;
    setAttribute(QualifiedName(nullAtom, localName.impl(), nullAtom), value.impl(), ec);
}

void ElementImpl::setAttribute(const QualifiedName& name, DOMStringImpl* value, ExceptionCode& ec)
{
    if (inDocument())
        getDocument()->incDOMTreeVersion();

    // allocate attributemap if necessary
    AttributeImpl* old = attributes(false)->getAttributeItem(name);

    // NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly
    if (namedAttrMap->isReadOnly()) {
        ec = NO_MODIFICATION_ALLOWED_ERR;
        return;
    }

    if (name == idAttr)
        updateId(old ? old->value() : nullAtom, value);
    
    if (old && !value)
        namedAttrMap->removeAttribute(name);
    else if (!old && value)
        namedAttrMap->addAttribute(createAttribute(name, value));
    else if (old && value) {
        old->setValue(value);
        attributeChanged(old);
    }
}

AttributeImpl* ElementImpl::createAttribute(const QualifiedName& name, DOMStringImpl* value)
{
    return new AttributeImpl(name, value);
}

void ElementImpl::setAttributeMap(NamedAttrMapImpl* list)
{
    if (inDocument())
        getDocument()->incDOMTreeVersion();

    // If setting the whole map changes the id attribute, we need to call updateId.

    AttributeImpl *oldId = namedAttrMap ? namedAttrMap->getAttributeItem(idAttr) : 0;
    AttributeImpl *newId = list ? list->getAttributeItem(idAttr) : 0;

    if (oldId || newId)
        updateId(oldId ? oldId->value() : nullAtom, newId ? newId->value() : nullAtom);

    if (namedAttrMap)
        namedAttrMap->element = 0;

    namedAttrMap = list;

    if (namedAttrMap) {
        namedAttrMap->element = this;
        unsigned len = namedAttrMap->length();
        for (unsigned i = 0; i < len; i++)
            attributeChanged(namedAttrMap->attrs[i]);
        // FIXME: What about attributes that were in the old map that are not in the new map?
    }
}

bool ElementImpl::hasAttributes() const
{
    updateStyleAttributeIfNeeded();
    return namedAttrMap && namedAttrMap->length() > 0;
}

DOMString ElementImpl::nodeName() const
{
    return m_tagName.toString();
}

void ElementImpl::setPrefix(const AtomicString &_prefix, ExceptionCode& ec)
{
    checkSetPrefix(_prefix, ec);
    if (ec)
        return;

    m_tagName.setPrefix(_prefix);
}

void ElementImpl::createAttributeMap() const
{
    namedAttrMap = new NamedAttrMapImpl(const_cast<ElementImpl*>(this));
}

bool ElementImpl::isURLAttribute(AttributeImpl *attr) const
{
    return false;
}

RenderStyle *ElementImpl::styleForRenderer(RenderObject *parentRenderer)
{
    return getDocument()->styleSelector()->styleForElement(this);
}

RenderObject *ElementImpl::createRenderer(RenderArena *arena, RenderStyle *style)
{
    if (getDocument()->documentElement() == this && style->display() == NONE) {
        // Ignore display: none on root elements.  Force a display of block in that case.
        RenderBlock* result = new (arena) RenderBlock(this);
        if (result) result->setStyle(style);
        return result;
    }
    return RenderObject::createObject(this, style);
}


void ElementImpl::insertedIntoDocument()
{
    // need to do superclass processing first so inDocument() is true
    // by the time we reach updateId
    ContainerNodeImpl::insertedIntoDocument();

    if (hasID()) {
        NamedAttrMapImpl* attrs = attributes(true);
        if (attrs) {
            AttributeImpl* idItem = attrs->getAttributeItem(idAttr);
            if (idItem && !idItem->isNull())
                updateId(nullAtom, idItem->value());
        }
    }
}

void ElementImpl::removedFromDocument()
{
    if (hasID()) {
        NamedAttrMapImpl* attrs = attributes(true);
        if (attrs) {
            AttributeImpl* idItem = attrs->getAttributeItem(idAttr);
            if (idItem && !idItem->isNull())
                updateId(idItem->value(), nullAtom);
        }
    }

    ContainerNodeImpl::removedFromDocument();
}

void ElementImpl::attach()
{
#if SPEED_DEBUG < 1
    createRendererIfNeeded();
#endif
    ContainerNodeImpl::attach();
}

void ElementImpl::recalcStyle( StyleChange change )
{
    // ### should go away and be done in renderobject
    RenderStyle* _style = renderer() ? renderer()->style() : 0;
    bool hasParentRenderer = parent() ? parent()->renderer() : false;
    
    if ( hasParentRenderer && (change >= Inherit || changed()) ) {
        RenderStyle *newStyle = getDocument()->styleSelector()->styleForElement(this);
        newStyle->ref();
        StyleChange ch = diff( _style, newStyle );
        if (ch == Detach) {
            if (attached()) detach();
            // ### Suboptimal. Style gets calculated again.
            attach();
            // attach recalulates the style for all children. No need to do it twice.
            setChanged( false );
            setHasChangedChild( false );
            newStyle->deref(getDocument()->renderArena());
            return;
        }
        else if (ch != NoChange) {
            if (renderer() && newStyle)
                renderer()->setStyle(newStyle);
        }
        else if (changed() && renderer() && newStyle && (getDocument()->usesSiblingRules() || getDocument()->usesDescendantRules())) {
            // Although no change occurred, we use the new style so that the cousin style sharing code won't get
            // fooled into believing this style is the same.  This is only necessary if the document actually uses
            // sibling/descendant rules, since otherwise it isn't possible for ancestor styles to affect sharing of
            // descendants.
            renderer()->setStyleInternal(newStyle);
        }

        newStyle->deref(getDocument()->renderArena());

        if ( change != Force) {
            if (getDocument()->usesDescendantRules())
                change = Force;
            else
                change = ch;
        }
    }

    for (NodeImpl *n = fastFirstChild(); n; n = n->nextSibling()) {
        if (change >= Inherit || n->isTextNode() || n->hasChangedChild() || n->changed())
            n->recalcStyle(change);
    }

    setChanged( false );
    setHasChangedChild( false );
}

bool ElementImpl::childTypeAllowed(NodeType type)
{
    switch (type) {
        case ELEMENT_NODE:
        case TEXT_NODE:
        case COMMENT_NODE:
        case PROCESSING_INSTRUCTION_NODE:
        case CDATA_SECTION_NODE:
        case ENTITY_REFERENCE_NODE:
            return true;
            break;
        default:
            return false;
    }
}

void ElementImpl::dispatchAttrRemovalEvent(AttributeImpl*)
{
#if 0
    if (!getDocument()->hasListenerType(DocumentImpl::DOMATTRMODIFIED_LISTENER))
        return;
    ExceptionCode ec = 0;
    dispatchEvent(new MutationEventImpl(DOMAttrModifiedEvent, true, false, attr, attr->value(),
        attr->value(), getDocument()->attrName(attr->id()), MutationEvent::REMOVAL), ec);
#endif
}

void ElementImpl::dispatchAttrAdditionEvent(AttributeImpl *attr)
{
#if 0
    if (!getDocument()->hasListenerType(DocumentImpl::DOMATTRMODIFIED_LISTENER))
        return;
    ExceptionCode ec = 0;
    dispatchEvent(new MutationEventImpl(DOMAttrModifiedEvent, true, false, attr, attr->value(),
        attr->value(),getDocument()->attrName(attr->id()), MutationEvent::ADDITION), ec);
#endif
}

DOMString ElementImpl::openTagStartToString() const
{
    DOMString result = DOMString("<") + nodeName();

    NamedAttrMapImpl *attrMap = attributes(true);

    if (attrMap) {
        unsigned numAttrs = attrMap->length();
        for (unsigned i = 0; i < numAttrs; i++) {
            result += " ";

            AttributeImpl *attribute = attrMap->attributeItem(i);
            result += attribute->name().toString();
            if (!attribute->value().isNull()) {
                result += "=\"";
                // FIXME: substitute entities for any instances of " or '
                result += attribute->value();
                result += "\"";
            }
        }
    }

    return result;
}

DOMString ElementImpl::toString() const
{
    DOMString result = openTagStartToString();

    if (hasChildNodes()) {
        result += ">";

        for (NodeImpl *child = firstChild(); child != NULL; child = child->nextSibling()) {
            result += child->toString();
        }

        result += "</";
        result += nodeName();
        result += ">";
    } else {
        result += " />";
    }

    return result;
}

void ElementImpl::updateId(const AtomicString& oldId, const AtomicString& newId)
{
    if (!inDocument())
        return;

    if (oldId == newId)
        return;

    DocumentImpl* doc = getDocument();
    if (!oldId.isEmpty())
        doc->removeElementById(oldId, this);
    if (!newId.isEmpty())
        doc->addElementById(newId, this);
}

#ifndef NDEBUG
void ElementImpl::dump(QTextStream *stream, QString ind) const
{
    updateStyleAttributeIfNeeded();
    if (namedAttrMap) {
        for (uint i = 0; i < namedAttrMap->length(); i++) {
            AttributeImpl *attr = namedAttrMap->attributeItem(i);
            *stream << " " << attr->name().localName().qstring().ascii()
                    << "=\"" << attr->value().qstring().ascii() << "\"";
        }
    }

    ContainerNodeImpl::dump(stream,ind);
}
#endif

#ifndef NDEBUG
void ElementImpl::formatForDebugger(char *buffer, unsigned length) const
{
    DOMString result;
    DOMString s;
    
    s = nodeName();
    if (s.length() > 0) {
        result += s;
    }
          
    s = getAttribute(idAttr);
    if (s.length() > 0) {
        if (result.length() > 0)
            result += "; ";
        result += "id=";
        result += s;
    }
          
    s = getAttribute(classAttr);
    if (s.length() > 0) {
        if (result.length() > 0)
            result += "; ";
        result += "class=";
        result += s;
    }
          
    strncpy(buffer, result.qstring().latin1(), length - 1);
}
#endif

PassRefPtr<AttrImpl> ElementImpl::setAttributeNode(AttrImpl *attr, ExceptionCode& ec)
{
    return static_pointer_cast<AttrImpl>(attributes(false)->setNamedItem(attr, ec));
}

PassRefPtr<AttrImpl> ElementImpl::removeAttributeNode(AttrImpl *attr, ExceptionCode& ec)
{
    if (!attr || attr->ownerElement() != this) {
        ec = NOT_FOUND_ERR;
        return 0;
    }
    if (getDocument() != attr->getDocument()) {
        ec = WRONG_DOCUMENT_ERR;
        return 0;
    }

    NamedAttrMapImpl *attrs = attributes(true);
    if (!attrs)
        return 0;

    return static_pointer_cast<AttrImpl>(attrs->removeNamedItem(attr->qualifiedName(), ec));
}

void ElementImpl::setAttributeNS(const String& namespaceURI, const String& qualifiedName, const String& value, ExceptionCode& ec)
{
    DOMString prefix, localName;
    if (!DocumentImpl::parseQualifiedName(qualifiedName, prefix, localName)) {
        ec = INVALID_CHARACTER_ERR;
        return;
    }
    setAttribute(QualifiedName(prefix.impl(), localName.impl(), namespaceURI.impl()), value.impl(), ec);
}

void ElementImpl::removeAttribute(const String& name, ExceptionCode& ec)
{
    String localName = inHTMLDocument(this) ? name.lower() : name;
    removeAttribute(QualifiedName(nullAtom, localName.impl(), nullAtom), ec);
}

void ElementImpl::removeAttributeNS(const String& namespaceURI, const String& localName, ExceptionCode& ec)
{
    removeAttribute(QualifiedName(nullAtom, localName.impl(), namespaceURI.impl()), ec);
}

PassRefPtr<AttrImpl> ElementImpl::getAttributeNode(const String& name)
{
    NamedAttrMapImpl *attrs = attributes(true);
    if (!attrs)
        return 0;
    String localName = inHTMLDocument(this) ? name.lower() : name;
    return static_pointer_cast<AttrImpl>(attrs->getNamedItem(QualifiedName(nullAtom, localName.impl(), nullAtom)));
}

PassRefPtr<AttrImpl> ElementImpl::getAttributeNodeNS(const String& namespaceURI, const String& localName)
{
    NamedAttrMapImpl *attrs = attributes(true);
    if (!attrs)
        return 0;
    return static_pointer_cast<AttrImpl>(attrs->getNamedItem(QualifiedName(nullAtom, localName.impl(), namespaceURI.impl())));
}

bool ElementImpl::hasAttribute(const String& name) const
{
    NamedAttrMapImpl *attrs = attributes(true);
    if (!attrs)
        return false;
    String localName = inHTMLDocument(this) ? name.lower() : name;
    return attrs->getAttributeItem(QualifiedName(nullAtom, localName.impl(), nullAtom));
}

bool ElementImpl::hasAttributeNS(const String& namespaceURI, const String& localName) const
{
    NamedAttrMapImpl *attrs = attributes(true);
    if (!attrs)
        return false;
    return attrs->getAttributeItem(QualifiedName(nullAtom, localName.impl(), namespaceURI.impl()));
}

CSSStyleDeclarationImpl *ElementImpl::style()
{
    return 0;
}

void ElementImpl::focus()
{
    DocumentImpl* doc = getDocument();
    doc->updateLayout();
    if (isFocusable()) {
        doc->setFocusNode(this);
        if (rootEditableElement() == this) {
            // FIXME: we should restore the previous selection if there is one, instead of always selecting all.
            if (doc->frame()->selectContentsOfNode(this))
                doc->frame()->revealSelection();
        } else if (renderer() && !renderer()->isWidget())
            renderer()->enclosingLayer()->scrollRectToVisible(getRect());
    }
}

void ElementImpl::blur()
{
    DocumentImpl* doc = getDocument();
    if (doc->focusNode() == this)
        doc->setFocusNode(0);
}

// -------------------------------------------------------------------------

NamedAttrMapImpl::NamedAttrMapImpl(ElementImpl *e)
    : element(e)
    , attrs(0)
    , len(0)
{
}

NamedAttrMapImpl::~NamedAttrMapImpl()
{
    NamedAttrMapImpl::clearAttributes(); // virtual method, so qualify just to be explicit
}

bool NamedAttrMapImpl::isMappedAttributeMap() const
{
    return false;
}

PassRefPtr<NodeImpl> NamedAttrMapImpl::getNamedItem(const String& name) const
{
    String localName = inHTMLDocument(element) ? name.lower() : name;
    return getNamedItem(QualifiedName(nullAtom, localName.impl(), nullAtom));
}

PassRefPtr<NodeImpl> NamedAttrMapImpl::getNamedItemNS(const String& namespaceURI, const String& localName) const
{
    return getNamedItem(QualifiedName(nullAtom, localName.impl(), namespaceURI.impl()));
}

PassRefPtr<NodeImpl> NamedAttrMapImpl::removeNamedItem(const String& name, ExceptionCode& ec)
{
    String localName = inHTMLDocument(element) ? name.lower() : name;
    return removeNamedItem(QualifiedName(nullAtom, localName.impl(), nullAtom), ec);
}

PassRefPtr<NodeImpl> NamedAttrMapImpl::removeNamedItemNS(const String& namespaceURI, const String& localName, ExceptionCode& ec)
{
    return removeNamedItem(QualifiedName(nullAtom, localName.impl(), namespaceURI.impl()), ec);
}

PassRefPtr<NodeImpl> NamedAttrMapImpl::getNamedItem(const QualifiedName& name) const
{
    AttributeImpl* a = getAttributeItem(name);
    if (!a) return 0;

    return a->createAttrImplIfNeeded(element);
}

PassRefPtr<NodeImpl> NamedAttrMapImpl::setNamedItem(NodeImpl* arg, ExceptionCode& ec)
{
    if (!element) {
        ec = NOT_FOUND_ERR;
        return 0;
    }

    // NO_MODIFICATION_ALLOWED_ERR: Raised if this map is readonly.
    if (isReadOnly()) {
        ec = NO_MODIFICATION_ALLOWED_ERR;
        return 0;
    }

    // WRONG_DOCUMENT_ERR: Raised if arg was created from a different document than the one that created this map.
    if (arg->getDocument() != element->getDocument()) {
        ec = WRONG_DOCUMENT_ERR;
        return 0;
    }

    // Not mentioned in spec: throw a HIERARCHY_REQUEST_ERROR if the user passes in a non-attribute node
    if (!arg->isAttributeNode()) {
        ec = HIERARCHY_REQUEST_ERR;
        return 0;
    }
    AttrImpl *attr = static_cast<AttrImpl*>(arg);

    AttributeImpl* a = attr->attrImpl();
    AttributeImpl* old = getAttributeItem(a->name());
    if (old == a)
        return RefPtr<NodeImpl>(arg); // we know about it already

    // INUSE_ATTRIBUTE_ERR: Raised if arg is an Attr that is already an attribute of another Element object.
    // The DOM user must explicitly clone Attr nodes to re-use them in other elements.
    if (attr->ownerElement()) {
        ec = INUSE_ATTRIBUTE_ERR;
        return 0;
    }

    if (a->name() == idAttr)
        element->updateId(old ? old->value() : nullAtom, a->value());

    // ### slightly inefficient - resizes attribute array twice.
    RefPtr<NodeImpl> r;
    if (old) {
        r = old->createAttrImplIfNeeded(element);
        removeAttribute(a->name());
    }

    addAttribute(a);
    return r.release();
}

// The DOM2 spec doesn't say that removeAttribute[NS] throws NOT_FOUND_ERR
// if the attribute is not found, but at this level we have to throw NOT_FOUND_ERR
// because of removeNamedItem, removeNamedItemNS, and removeAttributeNode.
PassRefPtr<NodeImpl> NamedAttrMapImpl::removeNamedItem(const QualifiedName& name, ExceptionCode& ec)
{
    // ### should this really be raised when the attribute to remove isn't there at all?
    // NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly
    if (isReadOnly()) {
        ec = NO_MODIFICATION_ALLOWED_ERR;
        return 0;
    }

    AttributeImpl* a = getAttributeItem(name);
    if (!a) {
        ec = NOT_FOUND_ERR;
        return 0;
    }

    RefPtr<NodeImpl> r = a->createAttrImplIfNeeded(element);

    if (name == idAttr)
        element->updateId(a->value(), nullAtom);

    removeAttribute(name);
    return r.release();
}

PassRefPtr<NodeImpl> NamedAttrMapImpl::item ( unsigned index ) const
{
    if (index >= len)
        return 0;

    return attrs[index]->createAttrImplIfNeeded(element);
}

AttributeImpl* NamedAttrMapImpl::getAttributeItem(const QualifiedName& name) const
{
    for (unsigned i = 0; i < len; ++i) {
        if (attrs[i]->name().matches(name))
            return attrs[i];
    }
    return 0;
}

void NamedAttrMapImpl::clearAttributes()
{
    if (attrs) {
        for (unsigned i = 0; i < len; i++) {
            if (attrs[i]->m_impl)
                attrs[i]->m_impl->m_element = 0;
            attrs[i]->deref();
        }
        fastFree(attrs);
        attrs = 0;
    }
    len = 0;
}

void NamedAttrMapImpl::detachFromElement()
{
    // we allow a NamedAttrMapImpl w/o an element in case someone still has a reference
    // to if after the element gets deleted - but the map is now invalid
    element = 0;
    clearAttributes();
}

NamedAttrMapImpl& NamedAttrMapImpl::operator=(const NamedAttrMapImpl& other)
{
    // clone all attributes in the other map, but attach to our element
    if (!element) return *this;

    // If assigning the map changes the id attribute, we need to call
    // updateId.

    AttributeImpl *oldId = getAttributeItem(idAttr);
    AttributeImpl *newId = other.getAttributeItem(idAttr);

    if (oldId || newId) {
        element->updateId(oldId ? oldId->value() : nullAtom, newId ? newId->value() : nullAtom);
    }

    clearAttributes();
    len = other.len;
    attrs = static_cast<AttributeImpl **>(fastMalloc(len * sizeof(AttributeImpl *)));

    // first initialize attrs vector, then call attributeChanged on it
    // this allows attributeChanged to use getAttribute
    for (uint i = 0; i < len; i++) {
        attrs[i] = other.attrs[i]->clone();
        attrs[i]->ref();
    }

    // FIXME: This is wasteful.  The class list could be preserved on a copy, and we
    // wouldn't have to waste time reparsing the attribute.
    // The derived class, HTMLNamedAttrMapImpl, which manages a parsed class list for the CLASS attribute,
    // will update its member variable when parse attribute is called.
    for(uint i = 0; i < len; i++)
        element->attributeChanged(attrs[i], true);

    return *this;
}

void NamedAttrMapImpl::addAttribute(AttributeImpl *attribute)
{
    // Add the attribute to the list
    AttributeImpl **newAttrs = static_cast<AttributeImpl **>(fastMalloc((len + 1) * sizeof(AttributeImpl *)));
    if (attrs) {
      for (uint i = 0; i < len; i++)
        newAttrs[i] = attrs[i];
      fastFree(attrs);
    }
    attrs = newAttrs;
    attrs[len++] = attribute;
    attribute->ref();

    AttrImpl * const attrImpl = attribute->m_impl;
    if (attrImpl)
        attrImpl->m_element = element;

    // Notify the element that the attribute has been added, and dispatch appropriate mutation events
    // Note that element may be null here if we are called from insertAttr() during parsing
    if (element) {
        RefPtr<AttributeImpl> a = attribute;
        element->attributeChanged(a.get());
        element->dispatchAttrAdditionEvent(a.get());
        element->dispatchSubtreeModifiedEvent(false);
    }
}

void NamedAttrMapImpl::removeAttribute(const QualifiedName& name)
{
    unsigned index = len+1;
    for (unsigned i = 0; i < len; ++i)
        if (attrs[i]->name().matches(name)) {
            index = i;
            break;
        }

    if (index >= len) return;

    // Remove the attribute from the list
    AttributeImpl* attr = attrs[index];
    if (attrs[index]->m_impl)
        attrs[index]->m_impl->m_element = 0;
    if (len == 1) {
        fastFree(attrs);
        attrs = 0;
        len = 0;
    }
    else {
        AttributeImpl **newAttrs = static_cast<AttributeImpl **>(fastMalloc((len - 1) * sizeof(AttributeImpl *)));
        uint i;
        for (i = 0; i < uint(index); i++)
            newAttrs[i] = attrs[i];
        len--;
        for (; i < len; i++)
            newAttrs[i] = attrs[i+1];
        fastFree(attrs);
        attrs = newAttrs;
    }

    // Notify the element that the attribute has been removed
    // dispatch appropriate mutation events
    if (element && !attr->m_value.isNull()) {
        AtomicString value = attr->m_value;
        attr->m_value = nullAtom;
        element->attributeChanged(attr);
        attr->m_value = value;
    }
    if (element) {
        element->dispatchAttrRemovalEvent(attr);
        element->dispatchSubtreeModifiedEvent(false);
    }
    attr->deref();
}

bool NamedAttrMapImpl::mapsEquivalent(const NamedAttrMapImpl* otherMap) const
{
    if (!otherMap)
        return false;
    
    if (length() != otherMap->length())
        return false;
    
    for (unsigned i = 0; i < length(); i++) {
        AttributeImpl *attr = attributeItem(i);
        AttributeImpl *otherAttr = otherMap->getAttributeItem(attr->name());
            
        if (!otherAttr || attr->value() != otherAttr->value())
            return false;
    }
    
    return true;
}

// ------------------------------- Styled Element and Mapped Attribute Implementation

CSSMappedAttributeDeclarationImpl::~CSSMappedAttributeDeclarationImpl()
{
    if (m_entryType != ePersistent)
        StyledElementImpl::removeMappedAttributeDecl(m_entryType, m_attrName, m_attrValue);
}

CSSMappedAttributeDeclarationImpl* StyledElementImpl::getMappedAttributeDecl(MappedAttributeEntry entryType, AttributeImpl* attr)
{
    if (!mappedAttributeDecls)
        return 0;
    return mappedAttributeDecls->get(MappedAttributeKey(entryType, attr->name().localName().impl(), attr->value().impl()));
}

void StyledElementImpl::setMappedAttributeDecl(MappedAttributeEntry entryType, AttributeImpl* attr, CSSMappedAttributeDeclarationImpl* decl)
{
    if (!mappedAttributeDecls)
        mappedAttributeDecls = new MappedAttributeDecls;
    mappedAttributeDecls->set(MappedAttributeKey(entryType, attr->name().localName().impl(), attr->value().impl()), decl);
}

void StyledElementImpl::removeMappedAttributeDecl(MappedAttributeEntry entryType,
                                                  const QualifiedName& attrName, const AtomicString& attrValue)
{
    if (!mappedAttributeDecls)
        return;
    mappedAttributeDecls->remove(MappedAttributeKey(entryType, attrName.localName().impl(), attrValue.impl()));
}

void StyledElementImpl::invalidateStyleAttribute()
{
    m_isStyleAttributeValid = false;
}

void StyledElementImpl::updateStyleAttributeIfNeeded() const
{
    if (!m_isStyleAttributeValid) {
        m_isStyleAttributeValid = true;
        m_synchronizingStyleAttribute = true;
        if (m_inlineStyleDecl)
            const_cast<StyledElementImpl*>(this)->setAttribute(styleAttr, m_inlineStyleDecl->cssText());
        m_synchronizingStyleAttribute = false;
    }
}

AttributeImpl* MappedAttributeImpl::clone(bool preserveDecl) const
{
    return new MappedAttributeImpl(name(), value(), preserveDecl ? m_styleDecl.get() : 0);
}

NamedMappedAttrMapImpl::NamedMappedAttrMapImpl(ElementImpl *e)
:NamedAttrMapImpl(e), m_mappedAttributeCount(0)
{}

void NamedMappedAttrMapImpl::clearAttributes()
{
    m_classList.clear();
    m_mappedAttributeCount = 0;
    NamedAttrMapImpl::clearAttributes();
}

bool NamedMappedAttrMapImpl::isMappedAttributeMap() const
{
    return true;
}

int NamedMappedAttrMapImpl::declCount() const
{
    int result = 0;
    for (uint i = 0; i < length(); i++) {
        MappedAttributeImpl* attr = attributeItem(i);
        if (attr->decl())
            result++;
    }
    return result;
}

bool NamedMappedAttrMapImpl::mapsEquivalent(const NamedMappedAttrMapImpl* otherMap) const
{
    // The # of decls must match.
    if (declCount() != otherMap->declCount())
        return false;
    
    // The values for each decl must match.
    for (uint i = 0; i < length(); i++) {
        MappedAttributeImpl* attr = attributeItem(i);
        if (attr->decl()) {
            AttributeImpl* otherAttr = otherMap->getAttributeItem(attr->name());
            if (!otherAttr || (attr->value() != otherAttr->value()))
                return false;
        }
    }
    return true;
}

inline static bool isClassWhitespace(QChar c)
{
    return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}

void NamedMappedAttrMapImpl::parseClassAttribute(const DOMString& classStr)
{
    m_classList.clear();
    if (!element->hasClass())
        return;
    
    DOMString classAttr = element->getDocument()->inCompatMode() ? 
        (classStr.impl()->isLower() ? classStr : DOMString(classStr.impl()->lower())) :
        classStr;
    
    AtomicStringList* curr = 0;
    
    const QChar* str = classAttr.unicode();
    int length = classAttr.length();
    int sPos = 0;

    while (true) {
        while (sPos < length && isClassWhitespace(str[sPos]))
            ++sPos;
        if (sPos >= length)
            break;
        int ePos = sPos + 1;
        while (ePos < length && !isClassWhitespace(str[ePos]))
            ++ePos;
        if (curr) {
            curr->setNext(new AtomicStringList(AtomicString(str + sPos, ePos - sPos)));
            curr = curr->next();
        } else {
            if (sPos == 0 && ePos == length) {
                m_classList.setString(AtomicString(classAttr));
                break;
            }
            m_classList.setString(AtomicString(str + sPos, ePos - sPos));
            curr = &m_classList;
        }
        sPos = ePos + 1;
    }
}

StyledElementImpl::StyledElementImpl(const QualifiedName& name, DocumentImpl *doc)
    : ElementImpl(name, doc)
{
    m_isStyleAttributeValid = true;
    m_synchronizingStyleAttribute = false;
}

StyledElementImpl::~StyledElementImpl()
{
    destroyInlineStyleDecl();
}

AttributeImpl* StyledElementImpl::createAttribute(const QualifiedName& name, DOMStringImpl* value)
{
    return new MappedAttributeImpl(name, value);
}

void StyledElementImpl::createInlineStyleDecl()
{
    m_inlineStyleDecl = new CSSMutableStyleDeclarationImpl;
    m_inlineStyleDecl->setParent(getDocument()->elementSheet());
    m_inlineStyleDecl->setNode(this);
    m_inlineStyleDecl->setStrictParsing(!getDocument()->inCompatMode());
}

void StyledElementImpl::destroyInlineStyleDecl()
{
    if (m_inlineStyleDecl) {
        m_inlineStyleDecl->setNode(0);
        m_inlineStyleDecl->setParent(0);
        m_inlineStyleDecl = 0;
    }
}

void StyledElementImpl::attributeChanged(AttributeImpl* attr, bool preserveDecls)
{
    MappedAttributeImpl* mappedAttr = static_cast<MappedAttributeImpl*>(attr);
    if (mappedAttr->decl() && !preserveDecls) {
        mappedAttr->setDecl(0);
        setChanged();
        if (namedAttrMap)
            mappedAttributes()->declRemoved();
    }

    bool checkDecl = true;
    MappedAttributeEntry entry;
    bool needToParse = mapToEntry(attr->name(), entry);
    if (preserveDecls) {
        if (mappedAttr->decl()) {
            setChanged();
            if (namedAttrMap)
                mappedAttributes()->declAdded();
            checkDecl = false;
        }
    }
    else if (!attr->isNull() && entry != eNone) {
        CSSMappedAttributeDeclarationImpl* decl = getMappedAttributeDecl(entry, attr);
        if (decl) {
            mappedAttr->setDecl(decl);
            setChanged();
            if (namedAttrMap)
                mappedAttributes()->declAdded();
            checkDecl = false;
        } else
            needToParse = true;
    }

    if (needToParse)
        parseMappedAttribute(mappedAttr);
    
    if (checkDecl && mappedAttr->decl()) {
        // Add the decl to the table in the appropriate spot.
        setMappedAttributeDecl(entry, attr, mappedAttr->decl());
        mappedAttr->decl()->setMappedState(entry, attr->name(), attr->value());
        mappedAttr->decl()->setParent(0);
        mappedAttr->decl()->setNode(0);
        if (namedAttrMap)
            mappedAttributes()->declAdded();
    }
}

bool StyledElementImpl::mapToEntry(const QualifiedName& attrName, MappedAttributeEntry& result) const
{
    result = eNone;
    if (attrName == styleAttr)
        return !m_synchronizingStyleAttribute;
    return true;
}

void StyledElementImpl::parseMappedAttribute(MappedAttributeImpl *attr)
{
    if (attr->name() == idAttr) {
        // unique id
        setHasID(!attr->isNull());
        if (namedAttrMap) {
            if (attr->isNull())
                namedAttrMap->setID(nullAtom);
            else if (getDocument()->inCompatMode() && !attr->value().impl()->isLower())
                namedAttrMap->setID(AtomicString(attr->value().domString().lower()));
            else
                namedAttrMap->setID(attr->value());
        }
        setChanged();
    } else if (attr->name() == classAttr) {
        // class
        setHasClass(!attr->isNull());
        if (namedAttrMap)
            mappedAttributes()->parseClassAttribute(attr->value());
        setChanged();
    } else if (attr->name() == styleAttr) {
        setHasStyle(!attr->isNull());
        if (attr->isNull())
            destroyInlineStyleDecl();
        else
            getInlineStyleDecl()->parseDeclaration(attr->value());
        m_isStyleAttributeValid = true;
        setChanged();
    }
}

void StyledElementImpl::createAttributeMap() const
{
    namedAttrMap = new NamedMappedAttrMapImpl(const_cast<StyledElementImpl*>(this));
}

CSSMutableStyleDeclarationImpl* StyledElementImpl::getInlineStyleDecl()
{
    if (!m_inlineStyleDecl)
        createInlineStyleDecl();
    return m_inlineStyleDecl.get();
}

CSSStyleDeclarationImpl* StyledElementImpl::style()
{
    return getInlineStyleDecl();
}

CSSMutableStyleDeclarationImpl* StyledElementImpl::additionalAttributeStyleDecl()
{
    return 0;
}

const AtomicStringList* StyledElementImpl::getClassList() const
{
    return namedAttrMap ? mappedAttributes()->getClassList() : 0;
}

static inline bool isHexDigit( const QChar &c ) {
    return ( c >= '0' && c <= '9' ) ||
           ( c >= 'a' && c <= 'f' ) ||
           ( c >= 'A' && c <= 'F' );
}

static inline int toHex( const QChar &c ) {
    return ( (c >= '0' && c <= '9')
             ? (c.unicode() - '0')
             : ( ( c >= 'a' && c <= 'f' )
                 ? (c.unicode() - 'a' + 10)
                 : ( ( c >= 'A' && c <= 'F' )
                     ? (c.unicode() - 'A' + 10)
                     : -1 ) ) );
}

void StyledElementImpl::addCSSProperty(MappedAttributeImpl* attr, int id, const DOMString &value)
{
    if (!attr->decl()) createMappedDecl(attr);
    attr->decl()->setProperty(id, value, false);
}

void StyledElementImpl::addCSSProperty(MappedAttributeImpl* attr, int id, int value)
{
    if (!attr->decl()) createMappedDecl(attr);
    attr->decl()->setProperty(id, value, false);
}

void StyledElementImpl::addCSSStringProperty(MappedAttributeImpl* attr, int id, const DOMString &value, CSSPrimitiveValueImpl::UnitTypes type)
{
    if (!attr->decl()) createMappedDecl(attr);
    attr->decl()->setStringProperty(id, value, type, false);
}

void StyledElementImpl::addCSSImageProperty(MappedAttributeImpl* attr, int id, const DOMString &URL)
{
    if (!attr->decl()) createMappedDecl(attr);
    attr->decl()->setImageProperty(id, URL, false);
}

void StyledElementImpl::addCSSLength(MappedAttributeImpl* attr, int id, const DOMString &value)
{
    // FIXME: This function should not spin up the CSS parser, but should instead just figure out the correct
    // length unit and make the appropriate parsed value.
    if (!attr->decl()) createMappedDecl(attr);

    // strip attribute garbage..
    StringImpl* v = value.impl();
    if (v) {
        unsigned int l = 0;
        
        while (l < v->length() && (*v)[l].unicode() <= ' ')
            l++;
        
        for (; l < v->length(); l++) {
            char cc = (*v)[l].latin1();
            if (cc > '9' || (cc < '0' && cc != '*' && cc != '%' && cc != '.'))
                break;
        }

        if (l != v->length()) {
            attr->decl()->setLengthProperty(id, v->substring(0, l), false);
            return;
        }
    }
    
    attr->decl()->setLengthProperty(id, value, false);
}

/* color parsing that tries to match as close as possible IE 6. */
void StyledElementImpl::addCSSColor(MappedAttributeImpl* attr, int id, const DOMString &c)
{
    // this is the only case no color gets applied in IE.
    if ( !c.length() )
        return;

    if (!attr->decl()) createMappedDecl(attr);
    
    if (attr->decl()->setProperty(id, c, false) )
        return;
    
    QString color = c.qstring();
    // not something that fits the specs.
    
    // we're emulating IEs color parser here. It maps transparent to black, otherwise it tries to build a rgb value
    // out of everyhting you put in. The algorithm is experimentally determined, but seems to work for all test cases I have.
    
    // the length of the color value is rounded up to the next
    // multiple of 3. each part of the rgb triple then gets one third
    // of the length.
    //
    // Each triplet is parsed byte by byte, mapping
    // each number to a hex value (0-9a-fA-F to their values
    // everything else to 0).
    //
    // The highest non zero digit in all triplets is remembered, and
    // used as a normalization point to normalize to values between 0
    // and 255.
    
    if ( color.lower() != "transparent" ) {
        if ( color[0] == '#' )
            color.remove( 0,  1 );
        int basicLength = (color.length() + 2) / 3;
        if ( basicLength > 1 ) {
            // IE ignores colors with three digits or less
            int colors[3] = { 0, 0, 0 };
            int component = 0;
            int pos = 0;
            int maxDigit = basicLength-1;
            while ( component < 3 ) {
                // search forward for digits in the string
                int numDigits = 0;
                while ( pos < (int)color.length() && numDigits < basicLength ) {
                    int hex = toHex( color[pos] );
                    colors[component] = (colors[component] << 4);
                    if ( hex > 0 ) {
                        colors[component] += hex;
                        maxDigit = kMin( maxDigit, numDigits );
                    }
                    numDigits++;
                    pos++;
                }
                while ( numDigits++ < basicLength )
                    colors[component] <<= 4;
                component++;
            }
            maxDigit = basicLength - maxDigit;
            
            // normalize to 00-ff. The highest filled digit counts, minimum is 2 digits
            maxDigit -= 2;
            colors[0] >>= 4*maxDigit;
            colors[1] >>= 4*maxDigit;
            colors[2] >>= 4*maxDigit;
            // assert( colors[0] < 0x100 && colors[1] < 0x100 && colors[2] < 0x100 );
            
            color.sprintf("#%02x%02x%02x", colors[0], colors[1], colors[2] );
            if ( attr->decl()->setProperty(id, DOMString(color), false) )
                return;
        }
    }
    attr->decl()->setProperty(id, CSS_VAL_BLACK, false);
}

void StyledElementImpl::createMappedDecl(MappedAttributeImpl* attr)
{
    CSSMappedAttributeDeclarationImpl* decl = new CSSMappedAttributeDeclarationImpl(0);
    attr->setDecl(decl);
    decl->setParent(getDocument()->elementSheet());
    decl->setNode(this);
    decl->setStrictParsing(false); // Mapped attributes are just always quirky.
}

// Golden ratio - arbitrary start value to avoid mapping all 0's to all 0's
// or anything like that.
const unsigned PHI = 0x9e3779b9U;

// Paul Hsieh's SuperFastHash
// http://www.azillionmonkeys.com/qed/hash.html
unsigned MappedAttributeHash::hash(const MappedAttributeKey& key)
{
    uint32_t hash = PHI;
    uint32_t tmp;

    const uint16_t* p;

    p = reinterpret_cast<const uint16_t*>(&key.name);
    hash += p[0];
    tmp = (p[1] << 11) ^ hash;
    hash = (hash << 16) ^ tmp;
    hash += hash >> 11;
    assert(sizeof(key.name) == 4 || sizeof(key.name) == 8);
    if (sizeof(key.name) == 8) {
        p += 2;
        hash += p[0];
        tmp = (p[1] << 11) ^ hash;
        hash = (hash << 16) ^ tmp;
        hash += hash >> 11;
    }

    p = reinterpret_cast<const uint16_t*>(&key.value);
    hash += p[0];
    tmp = (p[1] << 11) ^ hash;
    hash = (hash << 16) ^ tmp;
    hash += hash >> 11;
    assert(sizeof(key.value) == 4 || sizeof(key.value) == 8);
    if (sizeof(key.value) == 8) {
        p += 2;
        hash += p[0];
        tmp = (p[1] << 11) ^ hash;
        hash = (hash << 16) ^ tmp;
        hash += hash >> 11;
    }

    // Handle end case
    hash += key.type;
    hash ^= hash << 11;
    hash += hash >> 17;

    // Force "avalanching" of final 127 bits
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 2;
    hash += hash >> 15;
    hash ^= hash << 10;

    // This avoids ever returning a hash code of 0, since that is used to
    // signal "hash not computed yet", using a value that is likely to be
    // effectively the same as 0 when the low bits are masked
    if (hash == 0)
        hash = 0x80000000;

    return hash;
}

}
