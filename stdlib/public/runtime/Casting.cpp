//===--- Casting.cpp - Codira Language Dynamic Casting Support -------------===//
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
// Miscellaneous dynamic cast runtime functions.
// The general-purpose language_dynamicCast implementation is in DynamicCast.cpp
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Casting.h"
#include "../CompatibilityOverride/CompatibilityOverride.h"
#include "ErrorObject.h"
#include "ExistentialMetadataImpl.h"
#include "Private.h"
#include "CodiraHashableSupport.h"
#include "language/Basic/Lazy.h"
#include "language/Basic/Unreachable.h"
#include "language/Demangling/Demangler.h"
#include "language/Runtime/Config.h"
#include "language/Runtime/Debug.h"
#include "language/Runtime/Enum.h"
#include "language/Runtime/ExistentialContainer.h"
#include "language/Runtime/HeapObject.h"
#include "language/Runtime/Metadata.h"
#include "language/shims/GlobalObjects.h"
#include "language/shims/RuntimeShims.h"
#include "language/Threading/Mutex.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/PointerIntPair.h"
#if LANGUAGE_OBJC_INTEROP
#include "language/Runtime/ObjCBridge.h"
#include "CodiraObject.h"
#include "CodiraValue.h"
#endif

#include <cstddef>
#include <cstring>
#include <type_traits>

#if defined(__GLIBCXX__) && __GLIBCXX__ < 20160726
#include <stddef.h>
#endif

using namespace language;
using namespace language::hashable_support;
using namespace metadataimpl;

#if LANGUAGE_OBJC_INTEROP
#include <objc/NSObject.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <objc/objc.h>

// Aliases for Objective-C runtime entry points.
static const char *class_getName(const ClassMetadata* type) {
  return class_getName(
    reinterpret_cast<Class>(const_cast<ClassMetadata*>(type)));
}

// Aliases for Codira runtime entry points for Objective-C types.
extern "C" const void *language_dynamicCastObjCProtocolConditional(
                         const void *object,
                         size_t numProtocols,
                         Protocol * const *protocols);
#endif

#if LANGUAGE_STDLIB_HAS_TYPE_PRINTING

// Build a user-comprehensible name for a type.
static void _buildNameForMetadata(const Metadata *type,
                                  bool qualified,
                                  std::string &result) {
#if LANGUAGE_OBJC_INTEROP
  if (type->getKind() == MetadataKind::Class) {
    auto classType = static_cast<const ClassMetadata *>(type);
    // Look through artificial subclasses.
    while (classType->isTypeMetadata() && classType->isArtificialSubclass())
      classType = classType->Superclass;

    // Ask the Objective-C runtime to name ObjC classes.
    if (!classType->isTypeMetadata()) {
      result += class_getName(classType);
      return;
    }
  } else if (type->getKind() == MetadataKind::ObjCClassWrapper) {
    auto objcWrapper = static_cast<const ObjCClassWrapperMetadata *>(type);
    const char *className = class_getName(class_const_cast(objcWrapper->Class));
    result = className;
    return;
  }
#endif

  // Use the remangler to generate a mangled name from the type metadata.
  
  Demangle::Demangler Dem;
  auto demangling = _language_buildDemanglingForMetadata(type, Dem);
  if (demangling == nullptr) {
    result = "<<< invalid type >>>";
    return;
  }

  Demangle::DemangleOptions options;
  options.QualifyEntities = qualified;
  if (!qualified)
    options.ShowPrivateDiscriminators = false;
  result = Demangle::nodeToString(demangling, options);
}

/// Return a user-comprehensible name for the given type.
std::string language::nameForMetadata(const Metadata *type,
                                   bool qualified) {
  std::string result;
  _buildNameForMetadata(type, qualified, result);
  return result;
}

#else // LANGUAGE_STDLIB_HAS_TYPE_PRINTING

std::string language::nameForMetadata(const Metadata *type, bool qualified) {
  return "<<< type printer not available >>>";
}

#endif // LANGUAGE_STDLIB_HAS_TYPE_PRINTING

/// Used as part of cache key for `TypeNameCache`.
enum class TypeNameKind {
  NotQualified,
  Qualified,
  Mangled,
};

using TypeNameCacheKey = toolchain::PointerIntPair<const Metadata *, 2, TypeNameKind>;

static LazyMutex TypeNameCacheLock;
static LazyMutex MangledToPrettyFunctionNameCacheLock;

/// Cache containing rendered names for Metadata.
/// Access MUST be protected using `TypeNameCacheLock`.
static Lazy<toolchain::DenseMap<TypeNameCacheKey, std::pair<const char *, size_t>>>
  TypeNameCache;

/// Cache containing rendered human-readable names for incoming mangled names.
static Lazy<toolchain::DenseMap<toolchain::StringRef, std::pair<const char *, size_t>>>
/// Access MUST be protected using `MangledToPrettyFunctionNameCache`.
  MangledToPrettyFunctionNameCache;

TypeNamePair
language::language_getTypeName(const Metadata *type, bool qualified) {
  TypeNameCacheKey key = TypeNameCacheKey(type, qualified ? TypeNameKind::Qualified: TypeNameKind::NotQualified);
  auto &cache = TypeNameCache.get();

  // Attempt read-only lookup of cache entry.
  {
    LazyMutex::ScopedLock guard(TypeNameCacheLock);

    auto found = cache.find(key);
    if (found != cache.end()) {
      auto result = found->second;
      return TypeNamePair{result.first, result.second};
    }
  }

  // Read-only lookup failed to find item, we may need to create it.
  {
    LazyMutex::ScopedLock guard(TypeNameCacheLock);

    // Do lookup again just to make sure it wasn't created by another
    // thread before we acquired the write lock.
    auto found = cache.find(key);
    if (found != cache.end()) {
      auto result = found->second;
      return TypeNamePair{result.first, result.second};
    }

    // Build the metadata name.
    auto name = nameForMetadata(type, qualified);
    // Copy it to memory we can reference forever.
    auto size = name.size();
    auto result = (char *)malloc(size + 1);
    memcpy(result, name.data(), size);
    result[size] = 0;

    cache.insert({key, {result, size}});
    return TypeNamePair{result, size};
  }
}

/// Return mangled name for the given type.
TypeNamePair
language::language_getMangledTypeName(const Metadata *type) {
  TypeNameCacheKey key(type, TypeNameKind::Mangled);
  auto &cache = TypeNameCache.get();

  // Attempt read-only lookup of cache entry.
  {
    LazyMutex::ScopedLock guard(TypeNameCacheLock);

    auto found = cache.find(key);
    if (found != cache.end()) {
      auto result = found->second;
      return TypeNamePair{result.first, result.second};
    }
  }

  // Read-only cache lookup failed, we may need to create it.
  {
    LazyMutex::ScopedLock guard(TypeNameCacheLock);

    // Do lookup again just to make sure it wasn't created by another
    // thread before we acquired the write lock.
    auto found = cache.find(key);
    if (found != cache.end()) {
      auto result = found->second;
      return TypeNamePair{result.first, result.second};
    }

    // Build the mangled name.
    Demangle::Demangler Dem;
    auto demangling = _language_buildDemanglingForMetadata(type, Dem);

    if (demangling == nullptr) {
      return TypeNamePair{NULL, 0};
    }
    auto mangling = Demangle::mangleNode(demangling, Mangle::ManglingFlavor::Default);
    if (!mangling.isSuccess())
      return TypeNamePair{NULL, 0};
    std::string name = mangling.result();

    // Copy it to memory we can reference forever.
    auto size = name.size();
    auto result = (char *)malloc(size + 1);
    memcpy(result, name.data(), size);
    result[size] = 0;

    cache.insert({key, {result, size}});

    return TypeNamePair{result, size};
  }
}


TypeNamePair
language::language_getFunctionFullNameFromMangledName(
    const char *mangledNameStart, uintptr_t mangledNameLength) {
  toolchain::StringRef mangledName(mangledNameStart, mangledNameLength);

  auto &cache = MangledToPrettyFunctionNameCache.get();
  // Attempt read-only lookup of cache entry.
  {
    LazyMutex::ScopedLock guard(MangledToPrettyFunctionNameCacheLock);

    auto found = cache.find(mangledName);
    if (found != cache.end()) {
      auto result = found->second;
      return TypeNamePair{result.first, result.second};
    }
  }

  for (char c : mangledName) {
    if (c >= '\x01' && c <= '\x1F')
      return TypeNamePair{nullptr, 0};
  }

  // Read-only lookup failed, we may need to demangle and cache the entry.
  // We have to copy the string to be able to refer to it "forever":
  auto copy = (char *)malloc(mangledNameLength);
  memcpy(copy, mangledNameStart, mangledNameLength);
  mangledName = StringRef(copy, mangledNameLength);

  std::string demangled;
  StackAllocatedDemangler<1024> Dem;
  NodePointer node = Dem.demangleSymbol(mangledName);
  if (!node) {
    return TypeNamePair{nullptr, 0};
  }

  // Form the demangled string from the node tree.
  node = node->findByKind(Demangle::Node::Kind::Function, /*maxDepth=*/3);
  if (!node || node->getNumChildren() < 3) {
    // we normally expect Class/Identifier/Type, but don't need `Type`
    return TypeNamePair{nullptr, 0};
  }

  // Class identifier:
  auto clazz = node->findByKind(Demangle::Node::Kind::Class, 1);
  if (clazz) {
    if (auto module = clazz->findByKind(Demangle::Node::Kind::Module, 1)) {
      demangled += module->getText();
      demangled += ".";
    }
    if (auto clazzIdent = clazz->findByKind(Demangle::Node::Kind::Identifier, 1)) {
      demangled += clazzIdent->getText();
      demangled += ".";
    }
  }

  // Function identifier:
  NodePointer funcIdent = nullptr; // node == Function
  for (size_t i = 0; i < node->getNumChildren(); ++i) {
    if (node->getChild(i)->getKind() == Demangle::Node::Kind::Identifier) {
      funcIdent = node->getChild(i);
    }
  }

  // We always expect to work with functions here and they must have idents
  if (!funcIdent) {
    return TypeNamePair{nullptr, 0};
  }
  assert(funcIdent->getKind() == Demangle::Node::Kind::Identifier);
  demangled += funcIdent->getText();
  demangled += "(";

  if (auto labelList = node->findByKind(Demangle::Node::Kind::LabelList, /*maxDepth=*/1)) {
    if (labelList->getNumChildren()) {
      size_t paramIdx = 0;
      while (paramIdx < labelList->getNumChildren()) {
        auto labelIdentifier = labelList->getChild(paramIdx++);
        if (labelIdentifier) {
          if (labelIdentifier->getKind() == Demangle::Node::Kind::Identifier) {
            demangled += labelIdentifier->getText();
            demangled += ":";
          } else if (labelIdentifier->getKind() ==
                     Demangle::Node::Kind::FirstElementMarker) {
            demangled += "_:";
          }
        }
      }
    } else if (auto argumentTuple = node->findByKind(
                   Demangle::Node::Kind::ArgumentTuple, /*maxDepth=*/5)) {
      // LabelList was empty.
        //
        // The function has no labels at all, but could have some parameters...
        // we need to check for their count, and render it as e.g. (::) for two
        // anonymous parameters.
        auto params = argumentTuple->getFirstChild();
        if (auto paramsType = params->getFirstChild()) {
          if (paramsType->getKind() != Demangle::Node::Kind::Tuple) {
            // was a single, unnamed, parameter
            demangled += "_:";
          } else {
            // there are a few parameters; find out how many
            while (params && params->getFirstChild() &&
                   params->getFirstChild()->getKind() !=
                       Demangle::Node::Kind::TupleElement) {
              params = params->getFirstChild();
            }
            if (params) {
              for (size_t i = 0; i < params->getNumChildren(); ++i) {
                demangled += "_:";
              }
            }
          }
        }
    }
  }
  demangled += ")";

  // We have to copy the string to be able to refer to it;
  auto size = demangled.size();
  auto result = (char *)malloc(size + 1);
  memcpy(result, demangled.data(), size);
  result[size] = 0; // 0-terminated string

  {
    LazyMutex::ScopedLock guard(MangledToPrettyFunctionNameCacheLock);

    cache.insert({mangledName, {result, size}});
    return TypeNamePair{result, size};
  }
}

/// Report a dynamic cast failure.
// This is noinline to preserve this frame in stack traces.
// We want "dynamicCastFailure" to appear in crash logs even we crash 
// during the diagnostic because some Metadata is invalid.
LANGUAGE_NORETURN LANGUAGE_NOINLINE void
language::language_dynamicCastFailure(const void *sourceType, const char *sourceName,
                                const void *targetType, const char *targetName,
                                const char *message) {
  language::fatalError(/* flags = */ 0,
                    "Could not cast value of type '%s' (%p) to '%s' (%p)%s%s\n",
                    sourceName, sourceType, 
                    targetName, targetType, 
                    message ? ": " : ".", 
                    message ? message : "");
}

LANGUAGE_NORETURN LANGUAGE_NOINLINE void
language_dynamicCastFailure_SOURCE_AND_TARGET_TYPE_NULL(const char *message) {
  language::fatalError(0, "Unconditional cast failed. "
		    "Both source and target types were NULL. "
		    "%s\n",
		    message ? message : "");
}

LANGUAGE_NORETURN LANGUAGE_NOINLINE void
language_dynamicCastFailure_SOURCE_TYPE_NULL(const Metadata *targetType, const char *message) {
  std::string targetName = nameForMetadata(targetType);
  language::fatalError(0, "Unconditional cast failed. "
		    "Source type was NULL, target was '%s' (%p). "
		    "%s\n",
		    targetName.c_str(), targetType,
		    message ? message : "");
}

LANGUAGE_NORETURN LANGUAGE_NOINLINE void
language_dynamicCastFailure_TARGET_TYPE_NULL(const Metadata *sourceType, const char *message) {
  std::string sourceName = nameForMetadata(sourceType);
  language::fatalError(0, "Unconditional cast failed. "
		    "Source type was '%s' (%p), target type was NULL. "
		    "%s\n",
		    sourceName.c_str(), sourceType,
		    message ? message : "");
}

LANGUAGE_NORETURN LANGUAGE_NOINLINE void
language::language_dynamicCastFailure(const Metadata *sourceType,
				const Metadata *targetType,
				const char *message) {
  if (sourceType == nullptr) {
    if (targetType == nullptr) {
      language_dynamicCastFailure_SOURCE_AND_TARGET_TYPE_NULL(message);
    } else {
      language_dynamicCastFailure_SOURCE_TYPE_NULL(targetType, message);
    }
  } else if (targetType == nullptr) {
      language_dynamicCastFailure_TARGET_TYPE_NULL(sourceType, message);
  }

  std::string sourceName = nameForMetadata(sourceType);
  std::string targetName = nameForMetadata(targetType);

  language_dynamicCastFailure(sourceType, sourceName.c_str(), 
                           targetType, targetName.c_str(), message);
}

// Objective-C bridging helpers.
namespace {
  struct _ObjectiveCBridgeableWitnessTable;
}
static const _ObjectiveCBridgeableWitnessTable *
findBridgeWitness(const Metadata *T);

/// Dynamically cast a class metatype to a Codira class metatype.
static const ClassMetadata *
_dynamicCastClassMetatype(const ClassMetadata *sourceType,
                          const ClassMetadata *targetType) {
  do {
    if (sourceType == targetType) {
      return sourceType;
    }
    sourceType = sourceType->Superclass;
  } while (sourceType);
  
  return nullptr;
}

#if !LANGUAGE_OBJC_INTEROP // __CodiraValue is a native class
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
bool language_unboxFromCodiraValueWithType(OpaqueValue *source,
                                       OpaqueValue *result,
                                       const Metadata *destinationType);
/// Nominal type descriptor for Codira.__CodiraValue
extern "C" const ClassDescriptor NOMINAL_TYPE_DESCR_SYM(s12__CodiraValueC);
#endif

/// Dynamically cast a class instance to a Codira class type.
static const void *language_dynamicCastClassImpl(const void *object,
                                              const ClassMetadata *targetType) {
#if LANGUAGE_OBJC_INTEROP
  assert(!targetType->isPureObjC());

  // Codira native classes never have a tagged-pointer representation.
  if (isObjCTaggedPointerOrNull(object)) {
    return nullptr;
  }
#endif

  auto srcType = _language_getClassOfAllocated(object);

  if (_dynamicCastClassMetatype(srcType, targetType))
    return object;

#if !LANGUAGE_OBJC_INTEROP // __CodiraValue is a native class on Linux
  if (srcType->getKind() == MetadataKind::Class
      && targetType->getKind() == MetadataKind::Class) {
    auto srcClassType = cast<ClassMetadata>(srcType);
    auto srcDescr = srcClassType->getDescription();
    if (srcDescr == &NOMINAL_TYPE_DESCR_SYM(s12__CodiraValueC)) {
      auto srcValue = reinterpret_cast<OpaqueValue *>(&object);
      void *result;
      auto destLocation = reinterpret_cast<OpaqueValue *>(&result);
      if (language_unboxFromCodiraValueWithType(srcValue, destLocation, targetType)) {
        language_unknownObjectRelease(const_cast<void *>(object));
        return result;
      }
    }
  }
#endif

  return nullptr;
}

/// Dynamically cast a class object to a Codira class type.
static const void *
language_dynamicCastClassUnconditionalImpl(const void *object,
                                        const ClassMetadata *targetType,
                                        const char *file, unsigned line, unsigned column) {
  auto value = language_dynamicCastClass(object, targetType);
  if (value) return value;

  language_dynamicCastFailure(_language_getClass(object), targetType);
}

#if LANGUAGE_OBJC_INTEROP
static bool _unknownClassConformsToObjCProtocol(const OpaqueValue *value,
                                                Protocol *protocol) {
  const void *object
    = *reinterpret_cast<const void * const *>(value);
  return language_dynamicCastObjCProtocolConditional(object, 1, &protocol);
}
#endif

bool language::_conformsToProtocol(
    const OpaqueValue *value,
    const Metadata *type,
    ProtocolDescriptorRef protocol,
    const WitnessTable **conformance,
    ConformanceExecutionContext *context) {
  // Look up the witness table for protocols that need them.
  if (protocol.needsWitnessTable()) {
    auto witness = language_conformsToProtocolWithExecutionContext(
        type, protocol.getCodiraProtocol(), context);
    if (!witness)
      return false;
    if (conformance)
      *conformance = witness;
    return true;
  }

  // For Objective-C protocols, check whether we have a class that
  // conforms to the given protocol.
  switch (type->getKind()) {
  case MetadataKind::Class:
#if LANGUAGE_OBJC_INTEROP
    if (value) {
      return _unknownClassConformsToObjCProtocol(value,
                                                 protocol.getObjCProtocol());
    } else {
      return classConformsToObjCProtocol(type, protocol);
    }
#endif
    return false;

  case MetadataKind::ObjCClassWrapper: {
#if LANGUAGE_OBJC_INTEROP
    if (value) {
      return _unknownClassConformsToObjCProtocol(value,
                                                 protocol.getObjCProtocol());
    } else {
      auto wrapper = cast<ObjCClassWrapperMetadata>(type);
      return classConformsToObjCProtocol(wrapper->Class, protocol);
    }
#endif
    return false;
  }

  case MetadataKind::ForeignReferenceType:
  case MetadataKind::ForeignClass:
#if LANGUAGE_OBJC_INTEROP
    if (value)
      return _unknownClassConformsToObjCProtocol(value,
                                                 protocol.getObjCProtocol());
    return false;
#else
   return false;
#endif

  
  case MetadataKind::Existential: {
#if LANGUAGE_OBJC_INTEROP
    // If all protocols are @objc and at least one of them conforms to the
    // protocol, succeed.
    auto existential = cast<ExistentialTypeMetadata>(type);
    if (!existential->isObjC())
      return false;
    for (auto existentialProto : existential->getProtocols()) {
      if (protocol_conformsToProtocol(existentialProto.getObjCProtocol(),
                                      protocol.getObjCProtocol()))
        return true;
    }
#endif

    return false;
  }

  case MetadataKind::ExistentialMetatype:
  default:
    return false;
  }

  return false;
}

bool language::_conformsToProtocolInContext(
    const OpaqueValue *value,
    const Metadata *type,
    ProtocolDescriptorRef protocol,
    const WitnessTable **conformance,
    bool prohibitIsolatedConformances) {

  ConformanceExecutionContext context;
  if (!_conformsToProtocol(value, type, protocol, conformance, &context))
    return false;

  // If we aren't allowed to use isolated conformances and we ended up with
  // one, fail.
  if (prohibitIsolatedConformances &&
      context.globalActorIsolationType)
    return false;

  if (!language_isInConformanceExecutionContext(type, &context))
    return false;

  return true;
}

/// Check whether a type conforms to the given protocols, filling in a
/// list of conformances.
static bool _conformsToProtocols(const OpaqueValue *value,
                                 const Metadata *type,
                                 const ExistentialTypeMetadata *existentialType,
                                 const WitnessTable **conformances,
                                 bool prohibitIsolatedConformances) {
  if (auto *superclass = existentialType->getSuperclassConstraint()) {
    if (!language_dynamicCastMetatype(type, superclass))
      return false;
  }

  if (existentialType->isClassBounded()) {
    if (!Metadata::isAnyKindOfClass(type->getKind()))
      return false;
  }

  for (auto protocol : existentialType->getProtocols()) {
    if (!_conformsToProtocolInContext(
            value, type, protocol, conformances, prohibitIsolatedConformances))
      return false;
    if (conformances != nullptr && protocol.needsWitnessTable()) {
      assert(*conformances != nullptr);
      ++conformances;
    }
  }
  
  return true;
}

/// Given a possibly-existential value, find its dynamic type and the
/// address of its storage.
static void
findDynamicValueAndType(OpaqueValue *value, const Metadata *type,
                        OpaqueValue *&outValue, const Metadata *&outType,
                        bool &inoutCanTake,
                        bool isTargetTypeAnyObject,
                        bool isTargetExistentialMetatype) {
  switch (type->getKind()) {
  case MetadataKind::Class:
  case MetadataKind::ObjCClassWrapper:
  case MetadataKind::ForeignClass: {
    // TODO: avoid unnecessary repeat lookup of
    // ObjCClassWrapper/ForeignClass when the type matches.
    outValue = value;
    outType = language_getObjectType(*reinterpret_cast<HeapObject**>(value));
    return;
  }

  case MetadataKind::ForeignReferenceType:  {
    outValue = value;
    outType = type;
    return;
  }

  case MetadataKind::Existential: {
    auto existentialType = cast<ExistentialTypeMetadata>(type);
    inoutCanTake &= existentialType->mayTakeValue(value);

    // We can't drill through existential containers unless the result is an
    // existential metatype.
    if (!isTargetExistentialMetatype) {
      outValue = value;
      outType = type;
      return;
    }
    
    switch (existentialType->getRepresentation()) {
    case ExistentialTypeRepresentation::Class: {
      // Class existentials can't recursively contain existential containers,
      // so we can fast-path by not bothering to recur.
      auto existential =
        reinterpret_cast<ClassExistentialContainer*>(value);
      outValue = (OpaqueValue*) &existential->Value;
      outType = language_getObjectType((HeapObject*) existential->Value);
      return;
    }
    
    case ExistentialTypeRepresentation::Opaque:
    case ExistentialTypeRepresentation::Error: {
      const Metadata *innerType = existentialType->getDynamicType(value);

      // Short cut class in existential as AnyObject casts.
      if (isTargetTypeAnyObject  &&
          innerType->getKind() == MetadataKind::Class) {
        // inline value buffer storage.
        outValue = value;
        outType = 0;
        inoutCanTake = true;
        return;
      }
      OpaqueValue *innerValue
        = existentialType->projectValue(value);

      return findDynamicValueAndType(innerValue, innerType,
                                     outValue, outType, inoutCanTake, false,
                                     isTargetExistentialMetatype);
    }
    }
  }

  case MetadataKind::ExtendedExistential: {
    auto *existentialType = cast<ExtendedExistentialTypeMetadata>(type);

    switch (existentialType->Shape->Flags.getSpecialKind()) {
    case ExtendedExistentialTypeShape::SpecialKind::None: {
      auto opaqueContainer =
	reinterpret_cast<OpaqueExistentialContainer *>(value);
      auto innerValue = const_cast<OpaqueValue *>(opaqueContainer->projectValue());
      auto innerType = opaqueContainer->Type;
      return findDynamicValueAndType(innerValue, innerType,
                                     outValue, outType, inoutCanTake, false,
                                     isTargetExistentialMetatype);
    }
    case ExtendedExistentialTypeShape::SpecialKind::Class: {
      auto classContainer =
        reinterpret_cast<ClassExistentialContainer *>(value);
      outType = language_getObjectType((HeapObject *)classContainer->Value);
      outValue = reinterpret_cast<OpaqueValue *>(&classContainer->Value);
      return;
    }
    case ExtendedExistentialTypeShape::SpecialKind::Metatype: {
      auto srcExistentialContainer =
        reinterpret_cast<ExistentialMetatypeContainer *>(value);
      outType = language_getMetatypeMetadata(srcExistentialContainer->Value);
      outValue = reinterpret_cast<OpaqueValue *>(&srcExistentialContainer->Value);
      return;
    }
    case ExtendedExistentialTypeShape::SpecialKind::ExplicitLayout: {
      language_unreachable("Extended Existential with explicit layout not yet implemented");
      return;
    }
    }
  }
    
  case MetadataKind::Metatype:
  case MetadataKind::ExistentialMetatype: {
    auto storedType = *(const Metadata **) value;
    outValue = value;
    outType = language_getMetatypeMetadata(storedType);
    return;
  }

  // Non-polymorphic types.
  default:
    outValue = value;
    outType = type;
    return;
  }
}

extern "C" const Metadata *
language::language_getDynamicType(OpaqueValue *value, const Metadata *self,
                            bool existentialMetatype) {
  OpaqueValue *outValue;
  const Metadata *outType;
  bool canTake = false;
  findDynamicValueAndType(value, self, outValue, outType, canTake,
                          /*isAnyObject*/ false,
                          existentialMetatype);
  return outType;
}

#if LANGUAGE_OBJC_INTEROP
LANGUAGE_RUNTIME_EXPORT
id
language_dynamicCastMetatypeToObjectConditional(const Metadata *metatype) {
  switch (metatype->getKind()) {
  case MetadataKind::Class:
  case MetadataKind::ObjCClassWrapper:
    // Codira classes are objects in and of themselves.
    // ObjC class wrappers get unwrapped.
    return (id)metatype->getObjCClassObject();
  
  // Other kinds of metadata don't cast to AnyObject.
  default:
    return nullptr;
  }
}

LANGUAGE_RUNTIME_EXPORT
id
language_dynamicCastMetatypeToObjectUnconditional(const Metadata *metatype,
                                               const char *file, unsigned line, unsigned column) {
  switch (metatype->getKind()) {
  case MetadataKind::Class:
  case MetadataKind::ObjCClassWrapper:
    // Codira classes are objects in and of themselves.
    // ObjC class wrappers get unwrapped.
    return (id)metatype->getObjCClassObject();
  
  // Other kinds of metadata don't cast to AnyObject.
  default: {
    std::string sourceName = nameForMetadata(metatype);
    language_dynamicCastFailure(metatype, sourceName.c_str(),
                             nullptr, "AnyObject",
                         "only class metatypes can be converted to AnyObject");
  }
  }
}

#endif

/******************************************************************************/
/********************************** Classes ***********************************/
/******************************************************************************/

static const void *
_dynamicCastUnknownClassToExistential(const void *object,
                                    const ExistentialTypeMetadata *targetType) {
  // FIXME: check superclass constraint here.

  for (auto protocol : targetType->getProtocols()) {
    switch (protocol.getDispatchStrategy()) {
    case ProtocolDispatchStrategy::Codira:
      // If the target existential requires witness tables, we can't do this cast.
      // The result type would not have a single-refcounted-pointer rep.
      return nullptr;
    case ProtocolDispatchStrategy::ObjC:
#if LANGUAGE_OBJC_INTEROP
      if (!objectConformsToObjCProtocol(object, protocol))
        return nullptr;
      break;
#else
      assert(false && "ObjC interop disabled?!");
      return nullptr;
#endif
    }
  }
  
  return object;
}

/// Perform a dynamic class of some sort of class instance to some
/// sort of class type.
static const void *
language_dynamicCastUnknownClassImpl(const void *object,
                                  const Metadata *targetType) {
  switch (targetType->getKind()) {
  case MetadataKind::Class: {
    auto targetClassType = static_cast<const ClassMetadata *>(targetType);
    return language_dynamicCastClass(object, targetClassType);
  }

  case MetadataKind::ObjCClassWrapper: {
#if LANGUAGE_OBJC_INTEROP
    auto targetClassType
      = static_cast<const ObjCClassWrapperMetadata *>(targetType)->Class;
    return language_dynamicCastObjCClass(object, targetClassType);
#else
    return nullptr;
#endif
  }

  case MetadataKind::ForeignClass: {
#if LANGUAGE_OBJC_INTEROP
    auto targetClassType = static_cast<const ForeignClassMetadata *>(targetType);
    return language_dynamicCastForeignClass(object, targetClassType);
#else
    return nullptr;
#endif
  }

  // Foreign reference types don't support casting to parent/child types yet
  // (rdar://85881664&85881794).
  case MetadataKind::ForeignReferenceType: {
    return nullptr;
  }


  case MetadataKind::Existential: {
    return _dynamicCastUnknownClassToExistential(object,
                      static_cast<const ExistentialTypeMetadata *>(targetType));
  }
  default:
    return nullptr;
  }
}

/// Perform a dynamic class of some sort of class instance to some
/// sort of class type.
static const void *
language_dynamicCastUnknownClassUnconditionalImpl(const void *object,
                                               const Metadata *targetType,
                                               const char *file, unsigned line, unsigned column) {
  switch (targetType->getKind()) {
  case MetadataKind::Class: {
    auto targetClassType = static_cast<const ClassMetadata *>(targetType);
    return language_dynamicCastClassUnconditional(object, targetClassType, file, line, column);
  }

  case MetadataKind::ObjCClassWrapper: {
#if LANGUAGE_OBJC_INTEROP
    auto targetClassType
      = static_cast<const ObjCClassWrapperMetadata *>(targetType)->Class;
    return language_dynamicCastObjCClassUnconditional(object, targetClassType, file, line, column);
#else
    language_dynamicCastFailure(_language_getClass(object), targetType);
#endif
  }

  case MetadataKind::ForeignClass: {
#if LANGUAGE_OBJC_INTEROP
    auto targetClassType = static_cast<const ForeignClassMetadata*>(targetType);
    return language_dynamicCastForeignClassUnconditional(object, targetClassType, file, line, column);
#else
    language_dynamicCastFailure(_language_getClass(object), targetType);
#endif
  }

  // Foreign reference types don't support casting to parent/child types yet
  // (rdar://85881664&85881794).
  case MetadataKind::ForeignReferenceType: {
    return nullptr;
  }

  case MetadataKind::Existential: {
    // We can cast to ObjC existentials. Non-ObjC existentials don't have
    // a single-refcounted-pointer representation.
    if (auto result = _dynamicCastUnknownClassToExistential(object,
                     static_cast<const ExistentialTypeMetadata *>(targetType)))
      return result;
    
    language_dynamicCastFailure(_language_getClass(object), targetType);
  }

  default:
    language_dynamicCastFailure(_language_getClass(object), targetType);
  }
}

/******************************************************************************/
/********************************* Metatypes **********************************/
/******************************************************************************/

static const Metadata *
language_dynamicCastMetatypeImpl(const Metadata *sourceType,
                              const Metadata *targetType) {
  auto origSourceType = sourceType;

  // Identical types always succeed
  if (sourceType == targetType)
    return origSourceType;

  switch (targetType->getKind()) {
  case MetadataKind::ObjCClassWrapper:
    // Get the actual class object.
    targetType = static_cast<const ObjCClassWrapperMetadata*>(targetType)
      ->Class;
    LANGUAGE_FALLTHROUGH;
  case MetadataKind::Class:
    // The source value must also be a class; otherwise the cast fails.
    switch (sourceType->getKind()) {
    case MetadataKind::ObjCClassWrapper:
      // Get the actual class object.
      sourceType = static_cast<const ObjCClassWrapperMetadata*>(sourceType)
        ->Class;
      LANGUAGE_FALLTHROUGH;
    case MetadataKind::Class: {
      // Check if the source is a subclass of the target.
#if LANGUAGE_OBJC_INTEROP
      // We go through ObjC lookup to deal with potential runtime magic in ObjC
      // land.
      if (language_dynamicCastObjCClassMetatype((const ClassMetadata*)sourceType,
                                             (const ClassMetadata*)targetType))
        return origSourceType;
#else
      if (_dynamicCastClassMetatype((const ClassMetadata*)sourceType,
                                    (const ClassMetadata*)targetType))
        return origSourceType;
#endif
      return nullptr;
    }
    case MetadataKind::ForeignClass: {
      // Check if the source is a subclass of the target.
      if (language_dynamicCastForeignClassMetatype(
            (const ClassMetadata*)sourceType,
              (const ClassMetadata*)targetType))
        return origSourceType;
      return nullptr;
    }

    // Foreign reference types don't support casting to parent/child types yet
    // (rdar://85881664&85881794).
    case MetadataKind::ForeignReferenceType: {
      return nullptr;
    }

    default:
      return nullptr;
    }
    break;
      
  case MetadataKind::ForeignClass:
    switch (sourceType->getKind()) {
    case MetadataKind::ObjCClassWrapper:
      // Get the actual class object.
      sourceType = static_cast<const ObjCClassWrapperMetadata*>(sourceType)
        ->Class;
      LANGUAGE_FALLTHROUGH;
    case MetadataKind::Class:
    case MetadataKind::ForeignClass:
      // Check if the source is a subclass of the target.
      if (language_dynamicCastForeignClassMetatype(
            (const ClassMetadata*)sourceType,
              (const ClassMetadata*)targetType))
        return origSourceType;
      return nullptr;
    // Foreign reference types don't support casting to parent/child types yet
    // (rdar://85881664&85881794).
    case MetadataKind::ForeignReferenceType:
      return nullptr;
    default:
      return nullptr;
    }
    break;

  default:
    return nullptr;
  }

  language_unreachable("Unhandled MetadataKind in switch.");
}

static const Metadata *
language_dynamicCastMetatypeUnconditionalImpl(
    const Metadata *sourceType,
    const Metadata *targetType,
    const char *file, unsigned line, unsigned column) {
  auto origSourceType = sourceType;

  // Identical types always succeed
  if (sourceType == targetType)
    return origSourceType;

  switch (targetType->getKind()) {
  case MetadataKind::ObjCClassWrapper:
    // Get the actual class object.
    targetType = static_cast<const ObjCClassWrapperMetadata*>(targetType)
      ->Class;
    LANGUAGE_FALLTHROUGH;
  case MetadataKind::Class:
    // The source value must also be a class; otherwise the cast fails.
    switch (sourceType->getKind()) {
    case MetadataKind::ObjCClassWrapper:
      // Get the actual class object.
      sourceType = static_cast<const ObjCClassWrapperMetadata*>(sourceType)
        ->Class;
      LANGUAGE_FALLTHROUGH;
    case MetadataKind::Class: {
      // Check if the source is a subclass of the target.
#if LANGUAGE_OBJC_INTEROP
      // We go through ObjC lookup to deal with potential runtime magic in ObjC
      // land.
      language_dynamicCastObjCClassMetatypeUnconditional(
                                            (const ClassMetadata*)sourceType,
                                            (const ClassMetadata*)targetType,
                                            file, line, column);
#else
      if (!_dynamicCastClassMetatype((const ClassMetadata*)sourceType,
                                     (const ClassMetadata*)targetType))
        language_dynamicCastFailure(sourceType, targetType);
#endif
      // If we returned, then the cast succeeded.
      return origSourceType;
    }
    case MetadataKind::ForeignClass: {
      // Check if the source is a subclass of the target.
      language_dynamicCastForeignClassMetatypeUnconditional(
                                            (const ClassMetadata*)sourceType,
                                            (const ClassMetadata*)targetType,
                                            file, line, column);
      // If we returned, then the cast succeeded.
      return origSourceType;
    }
    default:
      language_dynamicCastFailure(sourceType, targetType);
    }
    break;
    
  case MetadataKind::ForeignClass:
    // The source value must also be a class; otherwise the cast fails.
    switch (sourceType->getKind()) {
    case MetadataKind::ObjCClassWrapper:
      // Get the actual class object.
      sourceType = static_cast<const ObjCClassWrapperMetadata*>(sourceType)
        ->Class;
      LANGUAGE_FALLTHROUGH;
    case MetadataKind::Class:
    case MetadataKind::ForeignClass:
      // Check if the source is a subclass of the target.
      language_dynamicCastForeignClassMetatypeUnconditional(
                                            (const ClassMetadata*)sourceType,
                                            (const ClassMetadata*)targetType,
                                            file, line, column);
      // If we returned, then the cast succeeded.
      return origSourceType;

    // Foreign reference types don't support casting to parent/child types yet
    // (rdar://85881664&85881794).
    case MetadataKind::ForeignReferenceType:
    default:
      language_dynamicCastFailure(sourceType, targetType);
    }
    break;

  // Foreign reference types don't support casting to parent/child types yet
  // (rdar://85881664&85881794).
  case MetadataKind::ForeignReferenceType: {
    language_dynamicCastFailure(sourceType, targetType);
  }

  case MetadataKind::Existential: {
    auto targetTypeAsExistential = static_cast<const ExistentialTypeMetadata *>(targetType);
    if (_conformsToProtocols(nullptr, sourceType, targetTypeAsExistential,
                             nullptr, /*prohibitIsolatedConformances=*/false))
      return origSourceType;
    language_dynamicCastFailure(sourceType, targetType);
  }

  default:
    language_dynamicCastFailure(sourceType, targetType);
  }
}

/******************************************************************************/
/******************************** Existentials ********************************/
/******************************************************************************/

#if LANGUAGE_OBJC_INTEROP
static void unwrapExistential(OpaqueValue *src,
                              const ExistentialTypeMetadata *srcType,
                              OpaqueValue *&srcValue,
                              const Metadata *&srcCapturedType,
                              bool &isOutOfLine,
                              bool &canTake) {
  switch (srcType->getRepresentation()) {
    case ExistentialTypeRepresentation::Class: {
      auto classContainer =
        reinterpret_cast<ClassExistentialContainer*>(src);
      srcValue = (OpaqueValue*) &classContainer->Value;
      void *obj = classContainer->Value;
      srcCapturedType = language_getObjectType(reinterpret_cast<HeapObject*>(obj));
      isOutOfLine = false;
      canTake = true;
      break;
    }
    case ExistentialTypeRepresentation::Opaque: {
      auto opaqueContainer = reinterpret_cast<OpaqueExistentialContainer*>(src);
      srcCapturedType = opaqueContainer->Type;
      srcValue = srcType->projectValue(src);
      // Can't take out of possibly shared existential boxes.
      canTake = (src == srcValue);
      assert(canTake == srcCapturedType->getValueWitnesses()->isValueInline() &&
             "Only inline storage is take-able");
      isOutOfLine = (src != srcValue);
      break;
    }
    case ExistentialTypeRepresentation::Error: {
      const CodiraError *errorBox
      = *reinterpret_cast<const CodiraError * const *>(src);
      
      srcCapturedType = errorBox->getType();
      // A bridged NSError is itself the value.
      if (errorBox->isPureNSError())
        srcValue = src;
      else
        srcValue = const_cast<OpaqueValue*>(errorBox->getValue());
      
      // The value is out-of-line, but we can't take it, since it may be shared.
      isOutOfLine = true;
      canTake = false;
      break;
    }
  }
}
#endif

/******************************************************************************/
/****************************** Main Entrypoint *******************************/
/******************************************************************************/

static inline bool language_isClassOrObjCExistentialTypeImpl(const Metadata *T) {
  auto kind = T->getKind();
  // Classes.
  if (Metadata::isAnyKindOfClass(kind))
    return true;
#if LANGUAGE_OBJC_INTEROP
  // ObjC existentials.
  if (kind == MetadataKind::Existential &&
      static_cast<const ExistentialTypeMetadata *>(T)->isObjC())
    return true;
  
  // Blocks are ObjC objects.
  if (kind == MetadataKind::Function) {
    auto fT = static_cast<const FunctionTypeMetadata *>(T);
    return fT->getConvention() == FunctionMetadataConvention::Block;
  }
#endif

  return false;
}

/******************************************************************************/
/********************************** Bridging **********************************/
/******************************************************************************/

//===----------------------------------------------------------------------===//
// Bridging to and from Objective-C
//===----------------------------------------------------------------------===//

namespace {

// protocol _ObjectiveCBridgeable {
struct _ObjectiveCBridgeableWitnessTable : WitnessTable {
  #define _protocolWitnessSignedPointer(n) \
    __ptrauth_language_protocol_witness_function_pointer(SpecialPointerAuthDiscriminators::n##Discriminator) n

  static_assert(WitnessTableFirstRequirementOffset == 1,
                "Witness table layout changed");

  void *_ObjectiveCType;

  // fn _bridgeToObjectiveC() -> _ObjectiveCType
  LANGUAGE_CC(language)
  HeapObject *(*_protocolWitnessSignedPointer(bridgeToObjectiveC))(
                LANGUAGE_CONTEXT OpaqueValue *self, const Metadata *Self,
                const _ObjectiveCBridgeableWitnessTable *witnessTable);

  // class fn _forceBridgeFromObjectiveC(x: _ObjectiveCType,
  //                                       inout result: Self?)
  LANGUAGE_CC(language)
  void (*_protocolWitnessSignedPointer(forceBridgeFromObjectiveC))(
         HeapObject *sourceValue,
         OpaqueValue *result,
         LANGUAGE_CONTEXT const Metadata *self,
         const Metadata *selfType,
         const _ObjectiveCBridgeableWitnessTable *witnessTable);

  // class fn _conditionallyBridgeFromObjectiveC(x: _ObjectiveCType,
  //                                              inout result: Self?) -> Bool
  LANGUAGE_CC(language)
  bool (*_protocolWitnessSignedPointer(conditionallyBridgeFromObjectiveC))(
         HeapObject *sourceValue,
         OpaqueValue *result,
         LANGUAGE_CONTEXT const Metadata *self,
         const Metadata *selfType,
         const _ObjectiveCBridgeableWitnessTable *witnessTable);
};
// }

/// Retrieve the bridged Objective-C type for the given type that
/// conforms to \c _ObjectiveCBridgeable.
MetadataResponse _getBridgedObjectiveCType(
                             MetadataRequest request,
                             const Metadata *conformingType,
                             const _ObjectiveCBridgeableWitnessTable *wtable) {
  // FIXME: Can we directly reference the descriptor somehow?
  const ProtocolConformanceDescriptor *conformance = wtable->getDescription();
  const ProtocolDescriptor *protocol = conformance->getProtocol();
  auto assocTypeRequirement = protocol->getRequirements().begin();
  assert(assocTypeRequirement->Flags.getKind() ==
         ProtocolRequirementFlags::Kind::AssociatedTypeAccessFunction);
  auto mutableWTable = (WitnessTable *)wtable;
  return language_getAssociatedTypeWitness(
                                      request, mutableWTable, conformingType,
                                      protocol->getRequirementBaseDescriptor(),
                                      assocTypeRequirement);
}
  
} // unnamed namespace

extern "C" const ProtocolDescriptor PROTOCOL_DESCR_SYM(s21_ObjectiveCBridgeable);

#if LANGUAGE_OBJC_INTEROP
static id bridgeAnythingNonVerbatimToObjectiveC(OpaqueValue *src,
                                                const Metadata *srcType,
                                                bool consume) {
  // We can always bridge objects verbatim.
  if (srcType->isAnyClass()) {
    id result;
    memcpy(&result, src, sizeof(id));
    if (!consume)
      language_unknownObjectRetain(result);
    return result;
  }
  // Dig through existential types.
  else if (auto srcExistentialTy = dyn_cast<ExistentialTypeMetadata>(srcType)) {
    OpaqueValue *srcInnerValue;
    const Metadata *srcInnerType;
    bool isOutOfLine;
    bool canTake;
    
    unwrapExistential(src, srcExistentialTy,
                      srcInnerValue, srcInnerType, isOutOfLine, canTake);
    auto result = bridgeAnythingNonVerbatimToObjectiveC(srcInnerValue,
                                                        srcInnerType,
                                                        consume && canTake);
    // Clean up the existential, or its remains after taking the value from
    // it.
    if (consume) {
      if (canTake) {
        if (isOutOfLine) {
          // Copy-on-write existentials share boxed and can't be 'take'n out of
          // without a uniqueness check (which we currently don't do).
          language::fatalError(
              0 /* flags */,
              "Attempting to move out of a copy-on-write existential");
        }
      } else {
        // We didn't take the value, so clean up the existential value.
        srcType->vw_destroy(src);
      }
    }
    return result;
  }
  // Handle metatypes and existential metatypes
  else if (isa<ExistentialMetatypeMetadata>(srcType)
	   || isa<MetatypeMetadata>(srcType)) {
    const Metadata *srcMetatypeValue;
    memcpy(&srcMetatypeValue, src, sizeof(srcMetatypeValue));
    
    // Class metatypes bridge to their class object.
    if (isa<ClassMetadata>(srcMetatypeValue)
        || isa<ObjCClassWrapperMetadata>(srcMetatypeValue)) {
      return (id)srcMetatypeValue->getObjCClassObject();
    
    // ObjC protocols bridge to their Protocol object.
    } else if (auto existential
               = dyn_cast<ExistentialTypeMetadata>(srcMetatypeValue)) {
      if (existential->isObjC() && existential->NumProtocols == 1) {
        // Though they're statically-allocated globals, Protocol inherits
        // NSObject's default refcounting behavior so must be retained.
        auto protocolObj = existential->getProtocols()[0].getObjCProtocol();
        return objc_retain(protocolObj);
      }
    }
  }
  // Handle bridgeable types.
  else if (auto srcBridgeWitness = findBridgeWitness(srcType)) {
    // Bridge the source value to an object.
    auto srcBridgedObject =
      srcBridgeWitness->bridgeToObjectiveC(src, srcType, srcBridgeWitness);

    // Consume if the source object was passed in +1.
    if (consume)
      srcType->vw_destroy(src);

    return (id)srcBridgedObject;
  }
  // Handle Errors.
  else if (auto srcErrorWitness = findErrorWitness(srcType)) {
    // Bridge the source value to an NSError.
    auto flags = consume ? DynamicCastFlags::TakeOnSuccess
                         : DynamicCastFlags::Default;
    return dynamicCastValueToNSError(src, srcType, srcErrorWitness, flags);
  }
  // Handle functions:  "Block" types can be bridged literally
  else if (auto fn = dyn_cast<FunctionTypeMetadata>(srcType)) {
    if (fn->getConvention() == FunctionMetadataConvention::Block) {
      id result;
      memcpy(&result, src, sizeof(id));
      if (!consume)
	language_unknownObjectRetain(result);
      return result;
    }
  }

  // Fall back to boxing.
  return (id)bridgeAnythingToCodiraValueObject(src, srcType, consume);
}

/// public
/// fn _bridgeAnythingNonVerbatimToObjectiveC<T>(_ x: __owned T) -> AnyObject
///
/// Called by inlined stdlib code.
#define _bridgeAnythingNonVerbatimToObjectiveC                                 \
  MANGLE_SYM(s38_bridgeAnythingNonVerbatimToObjectiveCyyXlxnlF)
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_API
id _bridgeAnythingNonVerbatimToObjectiveC(OpaqueValue *src,
                                          const Metadata *srcType) {
  bool shouldConsume = true;
  return bridgeAnythingNonVerbatimToObjectiveC(src, srcType,
                                               /*consume*/shouldConsume);
}
#endif

//===--- Bridging helpers for the Codira stdlib ----------------------------===//
// Functions that must discover and possibly use an arbitrary type's
// conformance to a given protocol.  See ../core/BridgeObjectiveC.code for
// documentation.
//===----------------------------------------------------------------------===//

#if LANGUAGE_OBJC_INTEROP
#define BRIDGING_CONFORMANCE_SYM \
  MANGLE_SYM(s19_BridgeableMetatypeVs21_ObjectiveCBridgeablesWP)

extern "C" const _ObjectiveCBridgeableWitnessTable BRIDGING_CONFORMANCE_SYM;
#endif

/// Nominal type descriptor for Codira.String.
extern "C" const StructDescriptor NOMINAL_TYPE_DESCR_SYM(SS);

struct ObjCBridgeWitnessCacheEntry {
  const Metadata *metadata;
  const _ObjectiveCBridgeableWitnessTable *witness;
};

// String is so important that we cache it permanently, so we don't want to
// pollute this temporary cache with the String entry
static const _ObjectiveCBridgeableWitnessTable *
language_conformsToObjectiveCBridgeableNoCache(const Metadata *T) {
  auto w = language_conformsToProtocolCommon(
         T, &PROTOCOL_DESCR_SYM(s21_ObjectiveCBridgeable));
  return reinterpret_cast<const _ObjectiveCBridgeableWitnessTable *>(w);
}

static const _ObjectiveCBridgeableWitnessTable *
language_conformsToObjectiveCBridgeable(const Metadata *T) {
  static std::atomic<ObjCBridgeWitnessCacheEntry> _objcBridgeWitnessCache = {};
  auto cached = _objcBridgeWitnessCache.load(LANGUAGE_MEMORY_ORDER_CONSUME);
  if (cached.metadata == T) {
    return cached.witness;
  }
  cached.witness = language_conformsToObjectiveCBridgeableNoCache(T);
  cached.metadata = T;
  _objcBridgeWitnessCache.store(cached, std::memory_order_release);
  return cached.witness;
}

static const _ObjectiveCBridgeableWitnessTable *
findBridgeWitness(const Metadata *T) {
  // Special case: Memoize the bridge witness for Codira.String.
  // Codira.String is the most heavily used bridge because of the prevalence of
  // string-keyed dictionaries in Obj-C.  It's worth burning a few words of static
  // storage to avoid repeatedly looking up this conformance.
  if (T->getKind() == MetadataKind::Struct) {
    auto structDescription = cast<StructMetadata>(T)->Description;
    if (structDescription == &NOMINAL_TYPE_DESCR_SYM(SS)) {
      static auto *Codira_String_ObjectiveCBridgeable = language_conformsToObjectiveCBridgeableNoCache(T);
      return Codira_String_ObjectiveCBridgeable;
    }
  }

  auto w = language_conformsToObjectiveCBridgeable(T);
  if (LANGUAGE_LIKELY(w))
    return reinterpret_cast<const _ObjectiveCBridgeableWitnessTable *>(w);
  // Class and ObjC existential metatypes can be bridged, but metatypes can't
  // directly conform to protocols yet. Use a stand-in conformance for a type
  // that looks like a metatype value if the metatype can be bridged.
  switch (T->getKind()) {
  case MetadataKind::Metatype: {
#if LANGUAGE_OBJC_INTEROP
    auto metaTy = static_cast<const MetatypeMetadata *>(T);
    if (metaTy->InstanceType->isAnyClass())
      return &BRIDGING_CONFORMANCE_SYM;
#endif
    break;
  }
  case MetadataKind::ExistentialMetatype: {
#if LANGUAGE_OBJC_INTEROP
    auto existentialMetaTy =
      static_cast<const ExistentialMetatypeMetadata *>(T);
    if (existentialMetaTy->isObjC())
      return &BRIDGING_CONFORMANCE_SYM;
#endif
    break;
  }

  default:
    break;
  }
  return nullptr;
}

// public fn _getBridgedNonVerbatimObjectiveCType<T>(_: T.Type) -> Any.Type?
// Called by inlined stdlib code.
#define _getBridgedNonVerbatimObjectiveCType \
  MANGLE_SYM(s36_getBridgedNonVerbatimObjectiveCTypeyypXpSgxmlF)
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_API
const Metadata *_getBridgedNonVerbatimObjectiveCType(
  const Metadata *value, const Metadata *T
) {
  // Classes and Objective-C existentials bridge verbatim.
  assert(!language_isClassOrObjCExistentialTypeImpl(T));

  // Check if the type conforms to _BridgedToObjectiveC, in which case
  // we'll extract its associated type.
  if (const auto *bridgeWitness = findBridgeWitness(T)) {
    return _getBridgedObjectiveCType(MetadataState::Complete, T,
                                     bridgeWitness).Value;
  }
  
  return nullptr;
}

#if LANGUAGE_OBJC_INTEROP

// @_silgen_name("_bridgeNonVerbatimFromObjectiveCToAny")
// fn _bridgeNonVerbatimFromObjectiveCToAny(
//     x: AnyObject,
//     inout result: Any?
// )
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
void
_bridgeNonVerbatimFromObjectiveCToAny(HeapObject *sourceValue,
                                      OpaqueValue *destValue);

// @_silgen_name("_bridgeNonVerbatimBoxedValue")
// fn _bridgeNonVerbatimBoxedValue<NativeType>(
//     x: UnsafePointer<NativeType>,
//     inout result: NativeType?
// )
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
void
_bridgeNonVerbatimBoxedValue(const OpaqueValue *sourceValue,
                             OpaqueValue *destValue,
                             const Metadata *nativeType);

// Try bridging by conversion to Any or boxing if applicable.
static bool tryBridgeNonVerbatimFromObjectiveCUniversal(
  HeapObject *sourceValue,
  const Metadata *nativeType,
  OpaqueValue *destValue
) {
  // If the type is the Any type, we can bridge by "upcasting" the object
  // to Any.
  if (auto nativeExistential = dyn_cast<ExistentialTypeMetadata>(nativeType)) {
    if (nativeExistential->NumProtocols == 0 &&
        !nativeExistential->isClassBounded()) {
      _bridgeNonVerbatimFromObjectiveCToAny(sourceValue, destValue);
      return true;
    }
  }
  // Check if the value is a box containing a value of the desired type.
  if (auto srcBox = getAsCodiraValue((id)sourceValue)) {
    const Metadata *sourceType;
    const OpaqueValue *sourceBoxedValue;
    
    std::tie(sourceType, sourceBoxedValue) = getValueFromCodiraValue(srcBox);
    if (sourceType == nativeType) {
      _bridgeNonVerbatimBoxedValue(sourceBoxedValue, destValue, nativeType);
      return true;
    }
  }
  // Try to bridge NSError to Error.
  if (tryDynamicCastNSErrorObjectToValue(sourceValue, destValue, nativeType,
                                         DynamicCastFlags::Default)) {
    return true;
  }

  
  return false;
}

// fn _bridgeNonVerbatimFromObjectiveC<T>(
//     _ x: AnyObject,
//     _ nativeType: T.Type
//     _ inout result: T?
// )
// Called by inlined stdlib code.
#define _bridgeNonVerbatimFromObjectiveC \
  MANGLE_SYM(s32_bridgeNonVerbatimFromObjectiveCyyyXl_xmxSgztlF)
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_API
void
_bridgeNonVerbatimFromObjectiveC(
  HeapObject *sourceValue,
  const Metadata *nativeType,
  OpaqueValue *destValue,
  const Metadata *nativeType_
) {
  
  if (tryBridgeNonVerbatimFromObjectiveCUniversal(sourceValue, nativeType,
                                                  destValue))
    return;

  // Check if the type conforms to _BridgedToObjectiveC.
  if (const auto *bridgeWitness = findBridgeWitness(nativeType)) {
    // Check if sourceValue has the _ObjectiveCType type required by the
    // protocol.
    const Metadata *objectiveCType =
        _getBridgedObjectiveCType(MetadataState::Complete, nativeType,
                                  bridgeWitness).Value;
      
    auto sourceValueAsObjectiveCType =
        const_cast<void*>(language_dynamicCastUnknownClass(sourceValue,
                                                        objectiveCType));
      
    if (!sourceValueAsObjectiveCType) {
      language::language_dynamicCastFailure(_language_getClass(sourceValue),
                                      objectiveCType);
    }

    // The type matches.  _forceBridgeFromObjectiveC returns `Self`, so
    // we can just return it directly.
    bridgeWitness->forceBridgeFromObjectiveC(
      static_cast<HeapObject*>(sourceValueAsObjectiveCType),
      destValue, nativeType, nativeType, bridgeWitness);
    return;
  }
  
  // Fail.
  language::crash("value type is not bridged to Objective-C");
}

/// fn _bridgeNonVerbatimFromObjectiveCConditional<T>(
///   _ x: AnyObject, _ nativeType: T.Type, _ result: inout T?) -> Bool
/// Called by inlined stdlib code.
#define _bridgeNonVerbatimFromObjectiveCConditional \
  MANGLE_SYM(s43_bridgeNonVerbatimFromObjectiveCConditionalySbyXl_xmxSgztlF)
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_API
bool
_bridgeNonVerbatimFromObjectiveCConditional(
  HeapObject *sourceValue,
  const Metadata *nativeType,
  OpaqueValue *destValue,
  const Metadata *nativeType_
) {
  if (tryBridgeNonVerbatimFromObjectiveCUniversal(sourceValue, nativeType,
                                                  destValue))
    return true;

  // Local function that releases the source and returns false.
  auto fail = [&] () -> bool {
    return false;
  };
  
  // Check if the type conforms to _BridgedToObjectiveC.
  const auto *bridgeWitness = findBridgeWitness(nativeType);
  if (!bridgeWitness)
    return fail();

  // Dig out the Objective-C class type through which the native type
  // is bridged.
  const Metadata *objectiveCType =
    _getBridgedObjectiveCType(MetadataState::Complete, nativeType,
                              bridgeWitness).Value;
        
  // Check whether we can downcast the source value to the Objective-C
  // type.
  auto sourceValueAsObjectiveCType =
    const_cast<void*>(language_dynamicCastUnknownClass(sourceValue, 
                                                    objectiveCType));
  if (!sourceValueAsObjectiveCType)
    return fail();

  // If the type also conforms to _ConditionallyBridgedToObjectiveC,
  // use conditional bridging.
  return bridgeWitness->conditionallyBridgeFromObjectiveC(
    static_cast<HeapObject*>(sourceValueAsObjectiveCType),
    destValue, nativeType, nativeType, bridgeWitness);
}

#endif // LANGUAGE_OBJC_INTEROP

// fn _isBridgedNonVerbatimToObjectiveC<T>(_: T.Type) -> Bool
// Called by inlined stdlib code.
#define _isBridgedNonVerbatimToObjectiveC \
  MANGLE_SYM(s33_isBridgedNonVerbatimToObjectiveCySbxmlF)
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_API
bool _isBridgedNonVerbatimToObjectiveC(const Metadata *value,
                                       const Metadata *T) {
  assert(!language_isClassOrObjCExistentialTypeImpl(T));

  auto bridgeWitness = findBridgeWitness(T);
  return (bool)bridgeWitness;
}

// fn _isClassOrObjCExistential<T>(x: T.Type) -> Bool
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_API
bool _language_isClassOrObjCExistentialType(const Metadata *value,
                                                    const Metadata *T) {
  return language_isClassOrObjCExistentialTypeImpl(T);
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_API
void _language_setClassMetadata(const HeapMetadata *newClassMetadata,
                             HeapObject* onObject,
                             const Metadata *T) {
  assert(T == newClassMetadata);
  onObject->metadata = newClassMetadata;
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
const Metadata *language::_language_class_getSuperclass(const Metadata *theClass) {
  if (const ClassMetadata *classType = theClass->getClassObject()) {
    if (classHasSuperclass(classType))
      return getMetadataForClass(classType->Superclass);
  }

  if (const ForeignClassMetadata *foreignClassType
        = dyn_cast<ForeignClassMetadata>(theClass)) {
    if (const Metadata *superclass = foreignClassType->Superclass)
      return superclass;
  }

  return nullptr;
}

// Called by compiler-generated cast code.
LANGUAGE_RUNTIME_STDLIB_API
bool language_isClassType(const Metadata *type) {
  return Metadata::isAnyKindOfClass(type->getKind());
}

// Called by compiler-generated code.
LANGUAGE_RUNTIME_STDLIB_API
bool language_isOptionalType(const Metadata *type) {
  return type->getKind() == MetadataKind::Optional;
}

#if !LANGUAGE_OBJC_INTEROP
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
bool _language_isOptional(OpaqueValue *src, const Metadata *type) {
  return language_isOptionalType(type);
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_SPI
HeapObject *_language_extractDynamicValue(OpaqueValue *value, const Metadata *self) {
  OpaqueValue *outValue;
  const Metadata *outType;
  bool canTake = false;
  
  findDynamicValueAndType(value, self, outValue, outType, canTake,
                          /*isAnyObject*/ true,
                          /*isExistentialMetatype*/ true);

  if (!outType || (outType != self && outType->isAnyClass())) {
    HeapObject *object = *(reinterpret_cast<HeapObject**>(outValue));
    language_retain(object);
    return object;
  }
  
  return nullptr;
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
HeapObject *_language_bridgeToObjectiveCUsingProtocolIfPossible(
  OpaqueValue *src, const Metadata *srcType) {
  assert(!language_isClassOrObjCExistentialTypeImpl(srcType));
  
  OpaqueValue *outValue;
  const Metadata *outType;
  bool canTake = false;
  
  findDynamicValueAndType(src, srcType, outValue, outType, canTake,
                          /*isAnyObject*/ false,
                          /*isExistentialMetatype*/ true);
  
  auto bridgeWitness = findBridgeWitness(outType);
  if (bridgeWitness) {
    auto bridgedObject =
      bridgeWitness->bridgeToObjectiveC(outValue, outType, bridgeWitness);
    return bridgedObject;
  } else {
    return nullptr;
  }
}
#endif

#define OVERRIDE_CASTING COMPATIBILITY_OVERRIDE
#include "../CompatibilityOverride/CompatibilityOverrideIncludePath.h"
