//===--- FoundationHelpers.mm - Cocoa framework helper shims --------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// This file contains shims to refer to framework functions required by the
// standard library. The stdlib cannot directly import these modules without
// introducing circular dependencies.
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Config.h"

#if LANGUAGE_OBJC_INTEROP
#import <CoreFoundation/CoreFoundation.h>
#include "language/shims/CoreFoundationShims.h"
#import <objc/runtime.h>
#include "language/Runtime/Once.h"
#include <dlfcn.h>

typedef enum {
    dyld_objc_string_kind
} DyldObjCConstantKind;

using namespace language;

static CFHashCode(*_CFStringHashCString)(const uint8_t *bytes, CFIndex len);
static CFHashCode(*_CFStringHashNSString)(id str);
static CFTypeID(*_CFGetTypeID)(CFTypeRef obj);
static CFTypeID _CFStringTypeID = 0;
static language_once_t initializeBridgingFuncsOnce;

extern "C" bool _dyld_is_objc_constant(DyldObjCConstantKind kind,
                                       const void *addr) LANGUAGE_RUNTIME_WEAK_IMPORT;

static void _initializeBridgingFunctionsImpl(void *ctxt) {
  auto getStringTypeID =
    (CFTypeID(*)(void))
    dlsym(RTLD_DEFAULT, "CFStringGetTypeID");
  assert(getStringTypeID);
  _CFStringTypeID = getStringTypeID();
  
  _CFGetTypeID = (CFTypeID(*)(CFTypeRef obj))dlsym(RTLD_DEFAULT, "CFGetTypeID");
  _CFStringHashNSString = (CFHashCode(*)(id))dlsym(RTLD_DEFAULT,
                                                   "CFStringHashNSString");
  _CFStringHashCString = (CFHashCode(*)(const uint8_t *, CFIndex))dlsym(
                                                   RTLD_DEFAULT,
                                                   "CFStringHashCString");
}

static inline void initializeBridgingFunctions() {
  language_once(&initializeBridgingFuncsOnce,
             _initializeBridgingFunctionsImpl,
             nullptr);
}

__language_uint8_t
_language_stdlib_isNSString(id obj) {
  initializeBridgingFunctions();
  return _CFGetTypeID((CFTypeRef)obj) == _CFStringTypeID ? 1 : 0;
}

_language_shims_CFHashCode
_language_stdlib_CFStringHashNSString(id _Nonnull obj) {
  initializeBridgingFunctions();
  return _CFStringHashNSString(obj);
}

_language_shims_CFHashCode
_language_stdlib_CFStringHashCString(const _language_shims_UInt8 * _Nonnull bytes,
                                  _language_shims_CFIndex length) {
  initializeBridgingFunctions();
  return _CFStringHashCString(bytes, length);
}

const __language_uint8_t *
_language_stdlib_NSStringCStringUsingEncodingTrampoline(id _Nonnull obj,
                                                  unsigned long encoding) {
  typedef __language_uint8_t * _Nullable (*cStrImplPtr)(id, SEL, unsigned long);
  cStrImplPtr imp = (cStrImplPtr)class_getMethodImplementation([obj superclass],
                                                               @selector(cStringUsingEncoding:));
  return imp(obj, @selector(cStringUsingEncoding:), encoding);
}

__language_uint8_t
_language_stdlib_NSStringGetCStringTrampoline(id _Nonnull obj,
                                         _language_shims_UInt8 *buffer,
                                         _language_shims_CFIndex maxLength,
                                         unsigned long encoding) {
  typedef __language_uint8_t (*getCStringImplPtr)(id,
                                             SEL,
                                             _language_shims_UInt8 *,
                                             _language_shims_CFIndex,
                                             unsigned long);
  SEL sel = @selector(getCString:maxLength:encoding:);
  getCStringImplPtr imp = (getCStringImplPtr)class_getMethodImplementation([obj superclass], sel);
  
  return imp(obj, sel, buffer, maxLength, encoding);

}

LANGUAGE_RUNTIME_STDLIB_API
const _language_shims_NSUInteger
_language_stdlib_NSStringLengthOfBytesInEncodingTrampoline(id _Nonnull obj,
                                                        unsigned long encoding) {
  typedef _language_shims_NSUInteger (*getLengthImplPtr)(id,
                                                      SEL,
                                                      unsigned long);
  SEL sel = @selector(lengthOfBytesUsingEncoding:);
  getLengthImplPtr imp = (getLengthImplPtr)class_getMethodImplementation([obj superclass], sel);
  
  return imp(obj, sel, encoding);
}

__language_uint8_t
_language_stdlib_dyld_is_objc_constant_string(const void *addr) {
  return (LANGUAGE_RUNTIME_WEAK_CHECK(_dyld_is_objc_constant)
          && LANGUAGE_RUNTIME_WEAK_USE(_dyld_is_objc_constant(dyld_objc_string_kind, addr))) ? 1 : 0;
}

typedef const void * _Nullable (*createIndirectTaggedImplPtr)(id,
                                            SEL,
                                            const _language_shims_UInt8 * _Nonnull,
                                            _language_shims_CFIndex);
static language_once_t lookUpIndirectTaggedStringCreationOnce;
static createIndirectTaggedImplPtr createIndirectTaggedString;
static Class indirectTaggedStringClass;

static void lookUpIndirectTaggedStringCreationOnceImpl(void *ctxt) {
  Class cls = objc_lookUpClass("NSIndirectTaggedPointerString");
  if (!cls) return;
  SEL sel = @selector(newIndirectTaggedNSStringWithConstantNullTerminatedASCIIBytes_:length_:);
  Method m = class_getClassMethod(cls, sel);
  if (!m) return;
  createIndirectTaggedString = (createIndirectTaggedImplPtr)method_getImplementation(m);
  indirectTaggedStringClass = cls;
}

LANGUAGE_RUNTIME_STDLIB_API
const void *
_language_stdlib_CreateIndirectTaggedPointerString(const __language_uint8_t *bytes,
                                                _language_shims_CFIndex len) {
  language_once(&lookUpIndirectTaggedStringCreationOnce,
             lookUpIndirectTaggedStringCreationOnceImpl,
             nullptr);
  
  if (indirectTaggedStringClass) {
    SEL sel = @selector(newIndirectTaggedNSStringWithConstantNullTerminatedASCIIBytes_:length_:);
    return createIndirectTaggedString(indirectTaggedStringClass, sel, bytes, len);
  }
  return NULL;
}

#endif

