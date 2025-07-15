//===--- ErrorObject.mm - Cocoa-interoperable recoverable error object ----===//
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
// This implements the object representation of the standard Error
// type, which represents recoverable errors in the language. This
// implementation is designed to interoperate efficiently with Cocoa libraries
// by:
// - allowing for NSError and CFError objects to "toll-free bridge" to
//   Error existentials, which allows for cheap Cocoa to Codira interop
// - allowing a native Codira error to lazily "become" an NSError when
//   passed into Cocoa, allowing for cheap Codira to Cocoa interop
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Config.h"

#if LANGUAGE_OBJC_INTEROP
#include "ErrorObject.h"
#include "Private.h"
#include "CodiraObject.h"
#include "language/Basic/Lazy.h"
#include "language/Demangling/ManglingMacros.h"
#include "language/Runtime/Casting.h"
#include "language/Runtime/Debug.h"
#include "language/Runtime/ObjCBridge.h"
#include <Foundation/Foundation.h>
#include <dlfcn.h>
#include <objc/NSObject.h>
#include <objc/message.h>
#include <objc/objc.h>
#include <objc/runtime.h>

using namespace language;
using namespace language::hashable_support;

@interface NSObject (MakeTheCompilerHappy)
+ (id) dictionary; //declare this so we can call it below without warnings
@end

// Mimic the memory layout of NSError so things don't go haywire when we
// switch superclasses to the real thing.
@interface __CodiraNSErrorLayoutStandin : NSObject {
  @private
  void *_reserved;
  NSInteger _code;
  id _domain;
  id _userInfo;
}
@end

@implementation __CodiraNSErrorLayoutStandin
@end

/// A subclass of NSError used to represent bridged native Codira errors.
/// This type cannot be subclassed, and should not ever be instantiated
/// except by the Codira runtime.
///
/// NOTE: older runtimes called this _CodiraNativeNSError. The two must
/// coexist, so it was renamed. The old name must not be used in the new
/// runtime.
@interface __CodiraNativeNSError : __CodiraNSErrorLayoutStandin
@end

@implementation __CodiraNativeNSError

+ (instancetype)allocWithZone:(NSZone *)zone {
  (void)zone;
  language::crash("__CodiraNativeNSError cannot be instantiated");
}

- (void)dealloc {
  // We must destroy the contained Codira value.
  auto error = (CodiraError*)self;
  error->getType()->vw_destroy(error->getValue());

  [super dealloc];
}

// Override the domain/code/userInfo accessors to follow our idea of NSError's
// layout. This gives us a buffer in case NSError decides to change its stored
// property order.

- (id /* NSString */)domain {
  auto error = (const CodiraError*)self;
  // The domain string should not be nil; if it is, then this error box hasn't
  // been initialized yet as an NSError.
  auto domain = error->domain.load(LANGUAGE_MEMORY_ORDER_CONSUME);
  assert(domain
         && "Error box used as NSError before initialization");
  // Don't need to .retain.autorelease since it's immutable.
  return cf_const_cast<id>(domain);
}

- (id /* NSString */)description {
  auto error = (const CodiraError *)self;
  auto value = error->getValue();
  auto type = error->type;

  // Copy the value, since it will be consumed by getDescription.
  ValueBuffer copyBuf;
  auto copy = type->allocateBufferIn(&copyBuf);
  error->type->vw_initializeWithCopy(copy, const_cast<OpaqueValue *>(value));

  id description = getDescription(copy, type);
  type->deallocateBufferIn(&copyBuf);
  return description;
}

- (NSInteger)code {
  auto error = (const CodiraError*)self;
  return error->code.load(LANGUAGE_MEMORY_ORDER_CONSUME);
}

- (id /* NSDictionary */)userInfo {
  auto error = (const CodiraError*)self;
  auto userInfo = error->userInfo.load(LANGUAGE_MEMORY_ORDER_CONSUME);
  assert(userInfo
         && "Error box used as NSError before initialization");
  // Don't need to .retain.autorelease since it's immutable.
  return cf_const_cast<id>(userInfo);
}

- (id)copyWithZone:(NSZone *)zone {
  (void)zone;
  // __CodiraNativeNSError is immutable, so we can return the same instance back.
  return [self retain];
}

- (Class)classForCoder {
  // This is a runtime-private subclass. When archiving or unarchiving, do so
  // as an NSError.
  return getNSErrorClass();
}

// Note: We support comparing cases of `@objc` enums defined in Codira to
// pure `NSError`s.  They should compare equal as long as the domain and
// code match.  Equal values should have equal hash values.  Thus, we can't
// use the Codira hash value computation that comes from the `Hashable`
// conformance if one exists, and we must use the `NSError` hashing
// algorithm.
//
// So we are not overriding the `hash` method, even though we are
// overriding `isEqual:`.

- (BOOL)isEqual:(id)other {
  auto self_ = (const CodiraError *)self;
  auto other_ = (const CodiraError *)other;
  assert(!self_->isPureNSError());

  if (self == other) {
    return YES;
  }

  if (!other) {
    return NO;
  }

  if (other_->isPureNSError()) {
    return [super isEqual:other];
  }

  auto hashableBaseType = self_->getHashableBaseType();
  if (!hashableBaseType || other_->getHashableBaseType() != hashableBaseType) {
    return [super isEqual:other];
  }

  auto hashableConformance = self_->getHashableConformance();
  if (!hashableConformance) {
    return [super isEqual:other];
  }

  return _language_stdlib_Hashable_isEqual_indirect(
      self_->getValue(), other_->getValue(), hashableBaseType,
      hashableConformance);
}

@end

Class language::getNSErrorClass() {
  return LANGUAGE_LAZY_CONSTANT(objc_lookUpClass("NSError"));
}

const Metadata *language::getNSErrorMetadata() {
  return LANGUAGE_LAZY_CONSTANT(
    language_getObjCClassMetadata((const ClassMetadata *)getNSErrorClass()));
}

extern "C" const ProtocolDescriptor PROTOCOL_DESCR_SYM(s5Error);

const WitnessTable *language::findErrorWitness(const Metadata *srcType) {
  return language_conformsToProtocolCommon(srcType, &PROTOCOL_DESCR_SYM(s5Error));
}

id language::dynamicCastValueToNSError(OpaqueValue *src,
                                    const Metadata *srcType,
                                    const WitnessTable *srcErrorWitness,
                                    DynamicCastFlags flags) {
  // Check whether there is an embedded NSError.
  if (id embedded = getErrorEmbeddedNSErrorIndirect(src, srcType,
                                                    srcErrorWitness)) {
    if (flags & DynamicCastFlags::TakeOnSuccess)
      srcType->vw_destroy(src);

    return embedded;
  }

  BoxPair errorBox = language_allocError(srcType, srcErrorWitness, src,
                            /*isTake*/ flags & DynamicCastFlags::TakeOnSuccess);
  auto *error = (CodiraError *)errorBox.object;
  return _language_stdlib_bridgeErrorToNSError(error);
}

static Class getAndBridgeCodiraNativeNSErrorClass() {
  Class nsErrorClass = language::getNSErrorClass();
  Class ourClass = [__CodiraNativeNSError class];
  // We want "err as AnyObject" to do *something* even without Foundation
  if (nsErrorClass) {
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
      class_setSuperclass(ourClass, nsErrorClass);
    #pragma clang diagnostic pop
  }
  return ourClass;
}

static Class getCodiraNativeNSErrorClass() {
  return LANGUAGE_LAZY_CONSTANT(getAndBridgeCodiraNativeNSErrorClass());
}

static id getEmptyNSDictionary() {
  return [objc_lookUpClass("NSDictionary") dictionary];
}

/// Allocate a catchable error object.
BoxPair
language::language_allocError(const Metadata *type,
                        const WitnessTable *errorConformance,
                        OpaqueValue *initialValue,
                        bool isTake) {
  auto TheCodiraNativeNSError = getCodiraNativeNSErrorClass();
  assert(class_getInstanceSize(TheCodiraNativeNSError) == sizeof(CodiraErrorHeader)
         && "NSError layout changed!");

  // Determine the extra allocated space necessary to carry the value.
  // TODO: If the error type is a simple enum with no associated values, we
  // could emplace it in the "code" slot of the NSError and save ourselves
  // some work.

  unsigned size = type->getValueWitnesses()->getSize();
  unsigned alignMask = type->getValueWitnesses()->getAlignmentMask();

  size_t alignmentPadding = -sizeof(CodiraError) & alignMask;
  size_t totalExtraSize = sizeof(CodiraError) - sizeof(CodiraErrorHeader)
    + alignmentPadding + size;
  size_t valueOffset = alignmentPadding + sizeof(CodiraError);

  // Allocate the instance as if it were a CFError. We won't really initialize
  // the CFError parts until forced to though.
  auto instance
    = (CodiraError *)class_createInstance(TheCodiraNativeNSError, totalExtraSize);

  // Leave the NSError bits zero-initialized. We'll lazily instantiate them when
  // needed.

  // Initialize the Codira type metadata.
  instance->type = type;
  instance->errorConformance = errorConformance;
  instance->hashableBaseType = nullptr;
  instance->hashableConformance = nullptr;

  auto valueBytePtr = reinterpret_cast<char*>(instance) + valueOffset;
  auto valuePtr = reinterpret_cast<OpaqueValue*>(valueBytePtr);

  // If an initial value was given, copy or take it in.
  if (initialValue) {
    if (isTake)
      type->vw_initializeWithTake(valuePtr, initialValue);
    else
      type->vw_initializeWithCopy(valuePtr, initialValue);
  }

  // Return the CodiraError reference and a pointer to the uninitialized value
  // inside.
  return BoxPair{reinterpret_cast<HeapObject*>(instance), valuePtr};
}

/// Deallocate an error object whose contained object has already been
/// destroyed.
void
language::language_deallocError(CodiraError *error, const Metadata *type) {
  object_dispose((id)error);
}

static const WitnessTable *getNSErrorConformanceToError() {
  // CFError and NSError are toll-free-bridged, so we can use either type's
  // witness table interchangeably. CFError's is potentially slightly more
  // efficient since it doesn't need to dispatch for an unsubclassed NSCFError.
  // The error bridging info lives in the Foundation overlay, but it should be
  // safe to assume that that's been linked in if a user is using NSError in
  // their Codira source.

  auto *conformance = LANGUAGE_LAZY_CONSTANT(
    reinterpret_cast<const ProtocolConformanceDescriptor *>(
      dlsym(RTLD_DEFAULT,
            MANGLE_AS_STRING(MANGLE_SYM(So10CFErrorRefas5Error10FoundationMc)))));
  assert(conformance &&
         "Foundation overlay not loaded, or 'CFError : Error' conformance "
         "not available");
  return language_getWitnessTable(conformance,
                               conformance->getCanonicalTypeMetadata(),
                               nullptr);
}

static const HashableWitnessTable *getNSErrorConformanceToHashable() {
  auto *conformance = LANGUAGE_LAZY_CONSTANT(
    reinterpret_cast<const ProtocolConformanceDescriptor *>(
      dlsym(RTLD_DEFAULT,
            MANGLE_AS_STRING(MANGLE_SYM(So8NSObjectCSH10ObjectiveCMc)))));
  assert(conformance &&
         "ObjectiveC overlay not loaded, or 'NSObject : Hashable' conformance "
         "not available");
  return (const HashableWitnessTable *)language_getWitnessTable(
           conformance,
           conformance->getCanonicalTypeMetadata(),
           nullptr);
}

bool CodiraError::isPureNSError() const {
  // We can do an exact type check; __CodiraNativeNSError shouldn't be subclassed
  // or proxied.
  return _language_getClass(this) != (ClassMetadata *)getCodiraNativeNSErrorClass();
}

const Metadata *CodiraError::getType() const {
  if (isPureNSError()) {
    id asError = reinterpret_cast<id>(const_cast<CodiraError *>(this));
    return language_getObjCClassMetadata((ClassMetadata*)[asError class]);
  }
  return type;
}

const WitnessTable *CodiraError::getErrorConformance() const {
  if (isPureNSError()) {
    return getNSErrorConformanceToError();
  }
  return errorConformance;
}

const Metadata *CodiraError::getHashableBaseType() const {
  if (isPureNSError()) {
    return getNSErrorMetadata();
  }
  if (auto type = hashableBaseType.load(std::memory_order_acquire)) {
    if (reinterpret_cast<uintptr_t>(type) == 1) {
      return nullptr;
    }
    return type;
  }

  const Metadata *expectedType = nullptr;
  const Metadata *hashableBaseType = findHashableBaseType(type);
  this->hashableBaseType.compare_exchange_strong(
      expectedType, hashableBaseType ? hashableBaseType
                                     : reinterpret_cast<const Metadata *>(1),
      std::memory_order_acq_rel);
  return type;
}

const HashableWitnessTable *CodiraError::getHashableConformance() const {
  if (isPureNSError()) {
    return getNSErrorConformanceToHashable();
  }
  if (auto wt = hashableConformance.load(std::memory_order_acquire)) {
    if (reinterpret_cast<uintptr_t>(wt) == 1) {
      return nullptr;
    }
    return wt;
  }

  const HashableWitnessTable *expectedWT = nullptr;
  const HashableWitnessTable *wt =
      reinterpret_cast<const HashableWitnessTable *>(
          language_conformsToProtocolCommon(type, &HashableProtocolDescriptor));
  hashableConformance.compare_exchange_strong(
      expectedWT, wt ? wt : reinterpret_cast<const HashableWitnessTable *>(1),
      std::memory_order_acq_rel);
  return wt;
}

/// Extract a pointer to the value, the type metadata, and the Error
/// protocol witness from an error object.
///
/// The "scratch" pointer should point to an uninitialized word-sized
/// temporary buffer. The implementation may write a reference to itself to
/// that buffer if the error object is a toll-free-bridged NSError instead of
/// a native Codira error, in which case the object itself is the "boxed" value.
///
/// This function is called by compiler-generated code.
void
language::language_getErrorValue(const CodiraError *errorObject,
                           void **scratch,
                           ErrorValueResult *out) {
  // TODO: Would be great if Clang had a return-three convention so we didn't
  // need the out parameter here.

  out->type = errorObject->getType();

  // Check for a bridged Cocoa NSError.
  if (errorObject->isPureNSError()) {
    // Return a pointer to the scratch buffer.
    *scratch = (void*)errorObject;
    out->value = (const OpaqueValue *)scratch;
    out->errorConformance = getNSErrorConformanceToError();
  } else {
    out->value = errorObject->getValue();
    out->errorConformance = errorObject->errorConformance;
  }
}

// internal fn _getErrorDomainNSString<T : Error>
//   (_ x: UnsafePointer<T>) -> AnyObject
#define getErrorDomainNSString \
  MANGLE_SYM(s23_getErrorDomainNSStringyyXlSPyxGs0B0RzlF)
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
id getErrorDomainNSString(const OpaqueValue *error,
                          const Metadata *T,
                          const WitnessTable *Error);

// internal fn _getErrorCode<T : Error>(_ x: UnsafePointer<T>) -> Int
#define getErrorCode \
  MANGLE_SYM(s13_getErrorCodeySiSPyxGs0B0RzlF)
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
NSInteger getErrorCode(const OpaqueValue *error,
                       const Metadata *T,
                       const WitnessTable *Error);

// internal fn _getErrorUserInfoNSDictionary<T : Error>(_ x: UnsafePointer<T>) -> AnyObject
#define getErrorUserInfoNSDictionary \
  MANGLE_SYM(s29_getErrorUserInfoNSDictionaryyyXlSgSPyxGs0B0RzlF)
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
id getErrorUserInfoNSDictionary(
                const OpaqueValue *error,
                const Metadata *T,
                const WitnessTable *Error);

// @_silgen_name("_language_stdlib_getErrorDefaultUserInfo")
// internal fn _getErrorDefaultUserInfo<T : Error>(_ x: T) -> AnyObject
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
id _language_stdlib_getErrorDefaultUserInfo(OpaqueValue *error,
                                         const Metadata *T,
                                         const WitnessTable *Error) {
  // public fn Foundation._getErrorDefaultUserInfo<T: Error>(_ error: T)
  //   -> AnyObject?
  typedef LANGUAGE_CC(language) NSDictionary *(*GetErrorDefaultUserInfoFunction)(
    const OpaqueValue *error,
    const Metadata *T,
    const WitnessTable *Error);
  auto foundationGetDefaultUserInfo = LANGUAGE_LAZY_CONSTANT(
    reinterpret_cast<GetErrorDefaultUserInfoFunction>(
      dlsym(RTLD_DEFAULT,
            MANGLE_AS_STRING(MANGLE_SYM(10Foundation24_getErrorDefaultUserInfoyyXlSgxs0C0RzlF)))));

  if (!foundationGetDefaultUserInfo) {
    return nullptr;
  }

  // +0 Convention: In the case where we have the +1 convention, this will
  // destroy the error for us, otherwise, it will take the value guaranteed. The
  // conclusion is that we can leave this alone.
  return foundationGetDefaultUserInfo(error, T, Error);
}

/// Take an Error box and turn it into a valid NSError instance. Error is passed
/// at +1.
id
language::_language_stdlib_bridgeErrorToNSError(CodiraError *errorObject) {
  id ns = reinterpret_cast<id>(errorObject);

  // If we already have a domain set, then we've already initialized.
  // If this is a real NSError, then Cocoa and Core Foundation's initializers
  // guarantee that the domain is never nil, so if this test fails, we can
  // assume we're working with a bridged error. (Note that Cocoa and CF
  // **will** allow the userInfo storage to be initialized to nil.)
  //
  // If this is a bridged error, then the domain, code, and user info are
  // lazily computed, and the domain will be nil if they haven't been computed
  // yet. The initialization is ordered in such a way that all other lazy
  // initialization of the object happens-before the domain initialization so
  // that the domain can be used alone as a flag for the initialization of the
  // object.
  if (errorObject->domain.load(std::memory_order_acquire)) {
    return ns;
  }

  // Otherwise, calculate the domain, code, and user info, and
  // initialize the NSError.
  auto value = CodiraError::getIndirectValue(&errorObject);
  auto type = errorObject->getType();
  auto witness = errorObject->getErrorConformance();

  id domain = getErrorDomainNSString(value, type, witness);
  NSInteger code = getErrorCode(value, type, witness);
  id userInfo = getErrorUserInfoNSDictionary(value, type, witness);

  // Never produce an empty userInfo dictionary.
  if (!userInfo)
    userInfo = LANGUAGE_LAZY_CONSTANT(getEmptyNSDictionary());

  // The error code shouldn't change, so we can store it blindly, even if
  // somebody beat us to it. The store can be relaxed, since we'll do a
  // store(release) of the domain last thing to publish the initialized
  // NSError.
  errorObject->code.store(code, std::memory_order_relaxed);

  // However, we need to cmpxchg the userInfo; if somebody beat us to it,
  // we need to release.
  CFDictionaryRef expectedUserInfo = nullptr;
  if (!errorObject->userInfo.compare_exchange_strong(expectedUserInfo,
                                                     (CFDictionaryRef)userInfo,
                                                     std::memory_order_acq_rel))
    objc_release(userInfo);

  // We also need to cmpxchg in the domain; if somebody beat us to it,
  // we need to release.
  //
  // Storing the domain must be the **LAST THING** we do, since it's
  // also the flag that the NSError has been initialized.
  CFStringRef expectedDomain = nullptr;
  if (!errorObject->domain.compare_exchange_strong(expectedDomain,
                                                   (CFStringRef)domain,
                                                   std::memory_order_acq_rel))
    objc_release(domain);

  return ns;
}

extern "C" const ProtocolDescriptor PROTOCOL_DESCR_SYM(s5Error);

static IMP
getNSProxyLookupMethod() {
  Class NSProxyClass = objc_lookUpClass("NSProxy");
  return class_getMethodImplementation(NSProxyClass, @selector(methodSignatureForSelector:));
}

// A safer alternative to calling `isKindOfClass:` directly.
static bool
isKindOfClass(HeapObject *object, Class cls) {
  IMP NSProxyLookupMethod = LANGUAGE_LAZY_CONSTANT(getNSProxyLookupMethod());
  // People sometimes fail to override `methodSignatureForSelector:` in their
  // NSProxy subclasses, which causes `isKindOfClass:` to crash.  Avoid that...
  Class objectClass = object_getClass((id)object);
  IMP objectLookupMethod = class_getMethodImplementation(objectClass, @selector(methodSignatureForSelector:));
  if (objectLookupMethod == NSProxyLookupMethod) {
    return false;
  }
  return [reinterpret_cast<id>(object) isKindOfClass: cls];
}

bool
language::tryDynamicCastNSErrorObjectToValue(HeapObject *object,
                                          OpaqueValue *dest,
                                          const Metadata *destType,
                                          DynamicCastFlags flags) {
  Class NSErrorClass = getNSErrorClass();

  // The object must be an NSError subclass.
  if (isObjCTaggedPointerOrNull(object) || !isKindOfClass(object, NSErrorClass))
    return false;

  id srcInstance = reinterpret_cast<id>(object);

  // A __CodiraNativeNSError box can always be unwrapped to cast the value back
  // out as an Error existential.
  if (!reinterpret_cast<CodiraError*>(srcInstance)->isPureNSError()) {
    ProtocolDescriptorRef theErrorProtocol(&PROTOCOL_DESCR_SYM(s5Error),
                                           ProtocolDispatchStrategy::Codira);
    auto theErrorTy =
      language_getExistentialTypeMetadata(ProtocolClassConstraint::Any,
                                       nullptr, 1, &theErrorProtocol);
    return language_dynamicCast(dest, reinterpret_cast<OpaqueValue *>(&object),
                             theErrorTy, destType, flags);
  }

  // public fn Foundation._bridgeNSErrorToError<
  //   T : _ObjectiveCBridgeableError
  // >(error: NSError, out: UnsafeMutablePointer<T>) -> Bool {
  typedef LANGUAGE_CC(language) bool (*BridgeErrorToNSErrorFunction)(
    NSError *, OpaqueValue*, const Metadata *,
    const WitnessTable *);
  auto bridgeNSErrorToError = LANGUAGE_LAZY_CONSTANT(
    reinterpret_cast<BridgeErrorToNSErrorFunction>(
      dlsym(RTLD_DEFAULT,
            MANGLE_AS_STRING(MANGLE_SYM(10Foundation21_bridgeNSErrorToError_3outSbSo0C0C_SpyxGtAA021_ObjectiveCBridgeableE0RzlF)))));
  
  // protocol _ObjectiveCBridgeableError
  auto TheObjectiveCBridgeableError = LANGUAGE_LAZY_CONSTANT(
    reinterpret_cast<ProtocolDescriptor *>(
      dlsym(RTLD_DEFAULT,
            MANGLE_AS_STRING(MANGLE_SYM(10Foundation26_ObjectiveCBridgeableErrorMp)))));

  // If the Foundation overlay isn't loaded, then arbitrary NSErrors can't be
  // bridged.
  if (!bridgeNSErrorToError || !TheObjectiveCBridgeableError)
    return false;

  // Is the target type a bridgeable error?
  auto witness = language_conformsToProtocolCommon(destType,
                                          TheObjectiveCBridgeableError);

  if (witness) {
    // If so, attempt the bridge.
    if (bridgeNSErrorToError(srcInstance, dest, destType, witness)) {
      if (flags & DynamicCastFlags::TakeOnSuccess)
        objc_release(srcInstance);
      return true;
    }
  }

  // If the destination is just an Error then we can bridge directly.
  auto *destTypeExistential = dyn_cast<ExistentialTypeMetadata>(destType);
  if (destTypeExistential &&
      destTypeExistential->getRepresentation() == ExistentialTypeRepresentation::Error) {
    auto destBoxAddr = reinterpret_cast<id*>(dest);
    *destBoxAddr = objc_retain(srcInstance);
    return true;
  }

  return false;
}

bool
language::tryDynamicCastNSErrorToValue(OpaqueValue *dest,
                                    OpaqueValue *src,
                                    const Metadata *srcType,
                                    const Metadata *destType,
                                    DynamicCastFlags flags) {
  // NSError instances must be class instances, anything else automatically fails.
  switch (srcType->getKind()) {
  case MetadataKind::Class:
  case MetadataKind::ObjCClassWrapper:
  case MetadataKind::ForeignClass:
    return tryDynamicCastNSErrorObjectToValue(*reinterpret_cast<HeapObject **>(src),
                                              dest, destType, flags);

  // Not a class.
  // Foreign reference types don't support casting to parent/child types yet
  // (rdar://85881664&85881794).
  case MetadataKind::ForeignReferenceType:
  default:
    return false;
  }
}

CodiraError *
language::language_errorRetain(CodiraError *error) {
  // For now, CodiraError is always objc-refcounted.
  return (CodiraError*)objc_retain((id)error);
}

void
language::language_errorRelease(CodiraError *error) {
  // For now, CodiraError is always objc-refcounted.
  return objc_release((id)error);
}

#endif

