// -*- mode: c++; c-basic-offset: 4 -*-
/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
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

#ifndef JSObjectRef_h
#define JSObjectRef_h

#include <JavaScriptCore/JSBase.h>
#include <JavaScriptCore/JSValueRef.h>

#include <stdbool.h>
#include <stddef.h> // for size_t

#ifdef __cplusplus
extern "C" {
#endif

/*!
@enum JSPropertyAttribute
@constant kJSPropertyAttributeNone         Specifies that a property has no special attributes.
@constant kJSPropertyAttributeReadOnly     Specifies that a property is read-only.
@constant kJSPropertyAttributeDontEnum     Specifies that a property should not be enumerated by JSPropertyEnumerators and JavaScript for...in loops.
@constant kJSPropertyAttributeDontDelete   Specifies that the delete operation should fail on a property.
*/
enum { 
    kJSPropertyAttributeNone         = 0,
    kJSPropertyAttributeReadOnly     = 1 << 1,
    kJSPropertyAttributeDontEnum     = 1 << 2,
    kJSPropertyAttributeDontDelete   = 1 << 3
};

/*! 
@typedef JSPropertyAttributes
@abstract A set of JSPropertyAttributes. Combine multiple attributes by logically ORing them together.
*/
typedef unsigned JSPropertyAttributes;

/*! 
@typedef JSObjectInitializeCallback
@abstract The callback invoked when an object is first created.
@param context The execution context to use.
@param object The JSObject being created.
@param exception A pointer to a JSValueRef in which to return an exception, if any.
@discussion If you named your function Initialize, you would declare it like this:

void Initialize(JSContextRef context, JSObjectRef object, JSValueRef* exception);
*/
typedef void
(*JSObjectInitializeCallback) (JSContextRef context, JSObjectRef object, JSValueRef* exception);

/*! 
@typedef JSObjectFinalizeCallback
@abstract The callback invoked when an object is finalized (prepared for garbage collection).
@param object The JSObject being finalized.
@discussion If you named your function Finalize, you would declare it like this:

void Finalize(JSObjectRef object);
*/
typedef void            
(*JSObjectFinalizeCallback) (JSObjectRef object);

/*! 
@typedef JSObjectHasPropertyCallback
@abstract The callback invoked when determining whether an object has a property.
@param context The current execution context.
@param object The JSObject to search for the property.
@param propertyName A JSString containing the name of the property look up.
@result true if object has the property, otherwise false.
@discussion If you named your function HasProperty, you would declare it like this:

bool HasProperty(JSContextRef context, JSObjectRef object, JSStringRef propertyName);

If this function returns false, the hasProperty request forwards to object's statically declared properties, then its parent class chain (which includes the default object class), then its prototype chain.

This callback enables optimization in cases where only a property's existence needs to be known, not its value, and computing its value would be expensive.

If this callback is NULL, the getProperty callback will be used to service hasProperty requests.
*/
typedef bool
(*JSObjectHasPropertyCallback) (JSContextRef context, JSObjectRef object, JSStringRef propertyName);

/*! 
@typedef JSObjectGetPropertyCallback
@abstract The callback invoked when getting a property's value.
@param context The current execution context.
@param object The JSObject to search for the property.
@param propertyName A JSString containing the name of the property to get.
@param exception A pointer to a JSValueRef in which to return an exception, if any.
@result The property's value if object has the property, otherwise NULL.
@discussion If you named your function GetProperty, you would declare it like this:

JSValueRef GetProperty(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception);

If this function returns NULL, the get request forwards to object's statically declared properties, then its parent class chain (which includes the default object class), then its prototype chain.
*/
typedef JSValueRef
(*JSObjectGetPropertyCallback) (JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception);

/*! 
@typedef JSObjectSetPropertyCallback
@abstract The callback invoked when setting a property's value.
@param context The current execution context.
@param object The JSObject on which to set the property's value.
@param propertyName A JSString containing the name of the property to set.
@param value A JSValue to use as the property's value.
@param exception A pointer to a JSValueRef in which to return an exception, if any.
@result true if the property was set, otherwise false.
@discussion If you named your function SetProperty, you would declare it like this:

bool SetProperty(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSValueRef* exception);

If this function returns false, the set request forwards to object's statically declared properties, then its parent class chain (which includes the default object class).
*/
typedef bool
(*JSObjectSetPropertyCallback) (JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSValueRef* exception);

/*! 
@typedef JSObjectDeletePropertyCallback
@abstract The callback invoked when deleting a property.
@param context The current execution context.
@param object The JSObject in which to delete the property.
@param propertyName A JSString containing the name of the property to delete.
@param exception A pointer to a JSValueRef in which to return an exception, if any.
@result true if propertyName was successfully deleted, otherwise false.
@discussion If you named your function DeleteProperty, you would declare it like this:

bool DeleteProperty(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception);

If this function returns false, the delete request forwards to object's statically declared properties, then its parent class chain (which includes the default object class).
*/
typedef bool
(*JSObjectDeletePropertyCallback) (JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception);

/*! 
@typedef JSObjectAddPropertiesToListCallback
@abstract The callback invoked when adding an object's properties to a property list.
@param object The JSObject whose properties need to be added to propertyList.
@param propertyList A JavaScript property list that will be used to enumerate object's properties.
@discussion If you named your function GetPropertyList, you would declare it like this:

void AddPropertiesToList(JSObjectRef object, JSPropertyListRef propertyList);

Use JSPropertyListAdd to add properties to propertyList.

Property lists are used by JSPropertyEnumerators and JavaScript for...in loops.
*/
typedef void
(*JSObjectAddPropertiesToListCallback) (JSObjectRef object, JSPropertyListRef propertyList);

/*! 
@typedef JSObjectCallAsFunctionCallback
@abstract The callback invoked when an object is called as a function.
@param context The current execution context.
@param function A JSObject that is the function being called.
@param thisObject A JSObject that is the 'this' variable in the function's scope.
@param argumentCount An integer count of the number of arguments in arguments.
@param arguments A JSValue array of the  arguments passed to the function.
@param exception A pointer to a JSValueRef in which to return an exception, if any.
@result A JSValue that is the function's return value.
@discussion If you named your function CallAsFunction, you would declare it like this:

JSValueRef CallAsFunction(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);

If your callback were invoked by the JavaScript expression 'myObject.myMemberFunction()', function would be set to myMemberFunction, and thisObject would be set to myObject.

If this callback is NULL, calling your object as a function will throw an exception.
*/
typedef JSValueRef 
(*JSObjectCallAsFunctionCallback) (JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);

/*! 
@typedef JSObjectCallAsConstructorCallback
@abstract The callback invoked when an object is used as a constructor in a 'new' expression.
@param context The current execution context.
@param constructor A JSObject that is the constructor being called.
@param argumentCount An integer count of the number of arguments in arguments.
@param arguments A JSValue array of the  arguments passed to the function.
@param exception A pointer to a JSValueRef in which to return an exception, if any.
@result A JSObject that is the constructor's return value.
@discussion If you named your function CallAsConstructor, you would declare it like this:

JSObjectRef CallAsConstructor(JSContextRef context, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);

If your callback were invoked by the JavaScript expression 'new myConstructorFunction()', constructor would be set to myConstructorFunction.

If this callback is NULL, using your object as a constructor in a 'new' expression will throw an exception.
*/
typedef JSObjectRef 
(*JSObjectCallAsConstructorCallback) (JSContextRef context, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);

/*! 
@typedef JSObjectHasInstanceCallback
@abstract hasInstance The callback invoked when an object is used as the target of an 'instanceof' expression.
@param context The current execution context.
@param constructor The JSObject that is the target of the 'instanceof' expression.
@param possibleInstance The JSValue being tested to determine if it is an instance of constructor.
@param exception A pointer to a JSValueRef in which to return an exception, if any.
@result true if possibleInstance is an instance of constructor, otherwise false.

@discussion If you named your function HasInstance, you would declare it like this:

bool HasInstance(JSContextRef context, JSObjectRef constructor, JSValueRef possibleInstance, JSValueRef* exception);

If your callback were invoked by the JavaScript expression 'someValue instanceof myObject', constructor would be set to myObject and possibleInstance would be set to someValue.

If this callback is NULL, 'instanceof' expressions that target your object will return false.

Standard JavaScript practice calls for objects that implement the callAsConstructor callback to implement the hasInstance callback as well.
*/
typedef bool 
(*JSObjectHasInstanceCallback)  (JSContextRef context, JSObjectRef constructor, JSValueRef possibleInstance, JSValueRef* exception);

/*! 
@typedef JSObjectConvertToTypeCallback
@abstract The callback invoked when converting an object to a particular JavaScript type.
@param context The current execution context.
@param object The JSObject to convert.
@param type A JSType specifying the JavaScript type to convert to.
@param exception A pointer to a JSValueRef in which to return an exception, if any.
@result The objects's converted value, or NULL if the object was not converted.
@discussion If you named your function ConvertToType, you would declare it like this:

JSValueRef ConvertToType(JSContextRef context, JSObjectRef object, JSType type, JSValueRef* exception);

If this function returns false, the conversion request forwards to object's parent class chain (which includes the default object class).

This function is only invoked when converting an object to number or string. An object converted to boolean is 'true.' An object converted to object is itself.
*/
typedef JSValueRef
(*JSObjectConvertToTypeCallback) (JSContextRef context, JSObjectRef object, JSType type, JSValueRef* exception);

/*! 
@struct JSStaticValue
@abstract This structure describes a statically declared value property.
@field name A null-terminated UTF8 string containing the property's name.
@field getProperty A JSObjectGetPropertyCallback to invoke when getting the property's value.
@field setProperty A JSObjectSetPropertyCallback to invoke when setting the property's value.
@field attributes A logically ORed set of JSPropertyAttributes to give to the property.
*/
typedef struct {
    const char* const name; // FIXME: convert UTF8
    JSObjectGetPropertyCallback getProperty;
    JSObjectSetPropertyCallback setProperty;
    JSPropertyAttributes attributes;
} JSStaticValue;

/*! 
@struct JSStaticFunction
@abstract This structure describes a statically declared function property.
@field name A null-terminated UTF8 string containing the property's name.
@field callAsFunction A JSObjectCallAsFunctionCallback to invoke when the property is called as a function.
@field attributes A logically ORed set of JSPropertyAttributes to give to the property.
*/
typedef struct {
    const char* const name; // FIXME: convert UTF8
    JSObjectCallAsFunctionCallback callAsFunction;
    JSPropertyAttributes attributes;
} JSStaticFunction;

/*!
@struct JSClassDefinition
@abstract This structure contains properties and callbacks that define a type of object. All fields are optional. Any field may be NULL.
@field version The version number of this structure. The current version is 0.
@field className A null-terminated UTF8 string containing the class's name.
@field parentClass A JSClass to set as the class's parent class. Pass NULL use the default object class.
@field staticValues A JSStaticValue array containing the class's statically declared value properties. Pass NULL to specify no statically declared value properties. The array must be terminated by a JSStaticValue whose name field is NULL.
@field staticFunctions A JSStaticFunction array containing the class's statically declared function properties. Pass NULL to specify no statically declared function properties. The array must be terminated by a JSStaticFunction whose name field is NULL.
@field initialize The callback invoked when an object is first created. Use this callback to initialize the object.
@field finalize The callback invoked when an object is finalized (prepared for garbage collection). Use this callback to release resources allocated for the object, and perform other cleanup.
@field hasProperty The callback invoked when determining whether an object has a property. If this field is NULL, getProperty is called instead. The hasProperty callback enables optimization in cases where only a property's existence needs to be known, not its value, and computing its value is expensive. 
@field getProperty The callback invoked when getting a property's value.
@field setProperty The callback invoked when setting a property's value.
@field deleteProperty The callback invoked when deleting a property.
@field addPropertiesToList The callback invoked when adding an object's properties to a property list.
@field callAsFunction The callback invoked when an object is called as a function.
@field hasInstance The callback invoked when an object is used as the target of an 'instanceof' expression.
@field callAsConstructor The callback invoked when an object is used as a constructor in a 'new' expression.
@field convertToType The callback invoked when converting an object to a particular JavaScript type.
@discussion The staticValues and staticFunctions arrays are the simplest and most efficient means for vending custom properties. Statically declared properties autmatically service requests like get, set, and enumerate. Property access callbacks are required only to implement unusual properties, like array indexes, whose names are not known at compile-time.

If you named your getter function "GetX" and your setter function "SetX", you would declare a JSStaticValue array containing "X" like this:

JSStaticValue StaticValueArray[] = {
    { "X", GetX, SetX, kJSPropertyAttributeNone },
    { 0, 0, 0, 0 }
};

Standard JavaScript practice calls for storing functions in prototype objects, so derived objects can share them. Therefore, it is common for prototype classes to have function properties but no value properties, and for object classes to have value properties but no function properties.

A NULL callback specifies that the default object callback should substitute, except in the case of hasProperty, where it specifies that getProperty should substitute.
*/
typedef struct {
    int                                 version; // current (and only) version is 0

    const char*                         className;
    JSClassRef                          parentClass;
        
    JSStaticValue*                      staticValues;
    JSStaticFunction*                   staticFunctions;
    
    JSObjectInitializeCallback          initialize;
    JSObjectFinalizeCallback            finalize;
    JSObjectHasPropertyCallback         hasProperty;
    JSObjectGetPropertyCallback         getProperty;
    JSObjectSetPropertyCallback         setProperty;
    JSObjectDeletePropertyCallback      deleteProperty;
    JSObjectAddPropertiesToListCallback addPropertiesToList;
    JSObjectCallAsFunctionCallback      callAsFunction;
    JSObjectCallAsConstructorCallback   callAsConstructor;
    JSObjectHasInstanceCallback         hasInstance;
    JSObjectConvertToTypeCallback       convertToType;
} JSClassDefinition;

/*! 
@const kJSClassDefinitionNull 
@abstract A JSClassDefinition structure of the current version, filled with NULL pointers.
@discussion Use this constant as a convenience when creating class definitions. For example, to create a class definition with only a finalize method:

JSClassDefinition definition = kJSClassDefinitionNull;

definition.finalize = Finalize;
*/
extern const JSClassDefinition kJSClassDefinitionNull;

/*!
@function
@abstract Creates a JavaScript class suitable for use with JSObjectMake.
@param definition A JSClassDefinition that defines the class.
@result A JSClass with the given definition's name, properties, callbacks, and parent class. Ownership follows the Create Rule.
*/
JSClassRef JSClassCreate(JSClassDefinition* definition);

/*!
@function
@abstract Retains a JavaScript class.
@param jsClass The JSClass to retain.
@result A JSClass that is the same as jsClass.
*/
JSClassRef JSClassRetain(JSClassRef jsClass);
/*!
@function
@abstract Releases a JavaScript class.
@param jsClass The JSClass to release.
*/
void JSClassRelease(JSClassRef jsClass);

/*!
@function
@abstract Creates a JavaScript object with a given class and prototype.
@param context The execution context to use.
@param jsClass The JSClass to assign to the object. Pass NULL to use the default object class.
@param prototype The prototype to assign to the object. Pass NULL to use the default object prototype.
@result A JSObject with the given class and prototype.
*/
JSObjectRef JSObjectMake(JSContextRef context, JSClassRef jsClass, JSValueRef prototype);

/*!
@function
@abstract Convenience method for creating a JavaScript function with a given callback as its implementation.
@param context The execution context to use.
@param callAsFunction The JSObjectCallAsFunctionCallback to invoke when the function is called.
@result A JSObject that is an anonymous function. The object's prototype will be the default function prototype.
*/
JSObjectRef JSObjectMakeFunction(JSContextRef context, JSObjectCallAsFunctionCallback callAsFunction);
/*!
@function
@abstract Convenience method for creating a JavaScript constructor with a given callback as its implementation.
@param context The execution context to use.
@param callAsConstructor The JSObjectCallAsConstructorCallback to invoke when the constructor is used in a 'new' expression.
@result A JSObject that is a constructor. The object's prototype will be the default object prototype.
*/
JSObjectRef JSObjectMakeConstructor(JSContextRef context, JSObjectCallAsConstructorCallback callAsConstructor);

/*!
@function
@abstract Creates a function with a given script as its body.
@param context The execution context to use.
@param name A JSString containing the function's name. Pass NULL to create an anonymous function.
@param parameterCount An integer count of the number of parameter names in parameterNames.
@param parameterNames A JSString array containing the names of the function's parameters. Pass NULL if parameterCount is 0.
@param body A JSString containing the script to use as the function's body.
@param sourceURL A JSString containing a URL for the script's source file. This is only used when reporting exceptions. Pass NULL if you do not care to include source file information in exceptions.
@param startingLineNumber An integer value specifying the script's starting line number in the file located at sourceURL. This is only used when reporting exceptions.
@param exception A pointer to a JSValueRef in which to store a syntax error exception, if any. Pass NULL if you do not care to store a syntax error exception.
@result A JSObject that is a function, or NULL if either body or parameterNames contains a syntax error. The object's prototype will be the default function prototype.
@discussion Use this method when you want to execute a script repeatedly, to avoid the cost of re-parsing the script before each execution.
*/
JSObjectRef JSObjectMakeFunctionWithBody(JSContextRef context, JSStringRef name, unsigned parameterCount, const JSStringRef parameterNames[], JSStringRef body, JSStringRef sourceURL, int startingLineNumber, JSValueRef* exception);

/*!
@function
@abstract Gets an object's prototype.
@param object A JSObject whose prototype you want to get.
@result A JSValue containing the object's prototype.
*/
JSValueRef JSObjectGetPrototype(JSObjectRef object);
/*!
@function
@abstract Sets an object's prototype.
@param object The JSObject whose prototype you want to set.
@param value A JSValue to set as the object's prototype.
*/
void JSObjectSetPrototype(JSObjectRef object, JSValueRef value);

/*!
@function
@abstract Tests whether an object has a given property.
@param object The JSObject to test.
@param propertyName A JSString containing the property's name.
@result true if the object has a property whose name matches propertyName, otherwise false.
*/
bool JSObjectHasProperty(JSContextRef context, JSObjectRef object, JSStringRef propertyName);

/*!
@function
@abstract Gets a property from an object.
@param context The execution context to use.
@param object The JSObject whose property you want to get.
@param propertyName A JSString containing the property's name.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result The property's value if object has the property, otherwise NULL.
*/
JSValueRef JSObjectGetProperty(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception);

/*!
@function
@abstract Sets a property on an object.
@param context The execution context to use.
@param object The JSObject whose property you want to set.
@param propertyName A JSString containing the property's name.
@param value A JSValue to use as the property's value.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@param attributes A logically ORed set of JSPropertyAttributes to give to the property.
*/
void JSObjectSetProperty(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSPropertyAttributes attributes, JSValueRef* exception);

/*!
@function
@abstract Deletes a property from an object.
@param context The execution context to use.
@param object The JSObject whose property you want to delete.
@param propertyName A JSString containing the property's name.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result true if the delete operation succeeds, otherwise false (for example, if the property has the kJSPropertyAttributeDontDelete attribute set).
*/
bool JSObjectDeleteProperty(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception);

/*!
@function
@abstract Gets a property from an object by numeric index.
@param context The execution context to use.
@param object The JSObject whose property you want to get.
@param propertyIndex The property's name as a number
@result The property's value if object has the property, otherwise NULL.
@discussion This is equivalent to getting a property by a string name containing the number, but allows faster access to JS arrays.
*/
JSValueRef JSObjectGetPropertyAtIndex(JSContextRef context, JSObjectRef object, unsigned propertyIndex);

/*!
@function
@abstract Sets a property on an object by numeric index.
@param context The execution context to use.
@param object The JSObject whose property you want to set.
@param propertyIndex The property's name as a number
@param value A JSValue to use as the property's value.
@param attributes A logically ORed set of JSPropertyAttributes to give to the property.
@discussion This is equivalent to setting a property by a string name containing the number, but allows faster access to JS arrays.
*/
void JSObjectSetPropertyAtIndex(JSContextRef context, JSObjectRef object, unsigned propertyIndex, JSValueRef value, JSPropertyAttributes attributes);

/*!
@function
@abstract Gets a pointer to private data from an object.
@param object A JSObject whose private data you want to get.
@result A void* that points to the object's private data, if the object has private data, otherwise NULL.
@discussion JSObjectGetPrivate and JSObjectSetPrivate only work on objects created by JSObjectMake, JSObjectMakeFunction, and JSObjectMakeConstructor.
*/
void* JSObjectGetPrivate(JSObjectRef object);

/*!
@function
@abstract Sets a pointer to private data on an object.
@param object A JSObject whose private data you want to set.
@param data A void* that points to the object's private data.
@result true if the set operation succeeds, otherwise false.
@discussion JSObjectGetPrivate and JSObjectSetPrivate only work on objects created by JSObjectMake, JSObjectMakeFunction, and JSObjectMakeConstructor.
*/
bool JSObjectSetPrivate(JSObjectRef object, void* data);

/*!
@function
@abstract Tests whether an object can be called as a function.
@param object The JSObject to test.
@result true if the object can be called as a function, otherwise false.
*/
bool JSObjectIsFunction(JSObjectRef object);
/*!
@function
@abstract Calls an object as a function.
@param context The execution context to use.
@param object The JSObject to call as a function.
@param thisObject The object to use as "this," or NULL to use the global object as "this."
@param argumentCount An integer count of the number of arguments in arguments.
@param arguments A JSValue array of the  arguments to pass to the function. Pass NULL if argumentCount is 0.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result The JSValue that results from calling object as a function, or NULL if an exception is thrown or object is not a function.
*/
JSValueRef JSObjectCallAsFunction(JSContextRef context, JSObjectRef object, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
/*!
@function
@abstract Tests whether an object can be called as a constructor.
@param object The JSObject to test.
@result true if the object can be called as a constructor, otherwise false.
*/
bool JSObjectIsConstructor(JSObjectRef object);
/*!
@function
@abstract Calls an object as a constructor.
@param context The execution context to use.
@param object The JSObject to call as a constructor.
@param argumentCount An integer count of the number of arguments in arguments.
@param arguments A JSValue array of the  arguments to pass to the function. Pass NULL if argumentCount is 0.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result The JSObject that results from calling object as a constructor, or NULL if an exception is thrown or object is not a constructor.
*/
JSObjectRef JSObjectCallAsConstructor(JSContextRef context, JSObjectRef object, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);

/*!
@function
@abstract Creates an enumerator for an object's properties.
@param object The object whose properties you want to enumerate.
@result A JSPropertyEnumerator with a list of object's properties. Ownership follows the Create Rule.
*/
JSPropertyEnumeratorRef JSObjectCreatePropertyEnumerator(JSObjectRef object);
/*!
@function
@abstract Retains a property enumerator.
@param enumerator The JSPropertyEnumerator to retain.
@result A JSPropertyEnumerator that is the same as enumerator.
*/
JSPropertyEnumeratorRef JSPropertyEnumeratorRetain(JSPropertyEnumeratorRef enumerator);
/*!
@function
@abstract Releases a property enumerator.
@param enumerator The JSPropertyEnumerator to release.
*/
void JSPropertyEnumeratorRelease(JSPropertyEnumeratorRef enumerator);
/*!
@function
@abstract Gets a property enumerator's next property.
@param enumerator The JSPropertyEnumerator whose next property you want to get.
@result A JSString containing the property's name, or NULL if all properties have been enumerated.
*/
JSStringRef JSPropertyEnumeratorGetNextName(JSPropertyEnumeratorRef enumerator);

/*!
@function
@abstract Adds a property to a property list.
@discussion Use this method inside a JSObjectAddPropertiesToListCallback to add a property to an object's property list.
@param propertyList The JSPropertyList to which you want to add a property.
@param thisObject The JSObject to which the property belongs.
@param propertyName A JSString specifying the property's name.
*/
void JSPropertyListAdd(JSPropertyListRef propertyList, JSObjectRef thisObject, JSStringRef propertyName);

#ifdef __cplusplus
}
#endif

#endif // JSObjectRef_h
