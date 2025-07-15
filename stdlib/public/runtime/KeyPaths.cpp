//===--- KeyPaths.cpp - Key path helper symbols ---------------------------===//
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

#include "language/Runtime/HeapObject.h"
#include "language/Runtime/Metadata.h"
#include <cstdint>
#include <cstring>
#include <new>

using namespace language;

LANGUAGE_RUNTIME_EXPORT LANGUAGE_CC(language)
void language_copyKeyPathTrivialIndices(const void *src, void *dest, size_t bytes) {
  memcpy(dest, src, bytes);
}

LANGUAGE_CC(language)
static bool equateGenericArguments(const void *a, const void *b, size_t bytes) {
  // Generic arguments can't affect equality, since an equivalent key path may
  // have been formed in a fully concrete context without capturing generic
  // arguments.
  return true;
}

LANGUAGE_CC(language)
static intptr_t hashGenericArguments(const void *src, size_t bytes) {
  // Generic arguments can't affect equality, since an equivalent key path may
  // have been formed in a fully concrete context without capturing generic
  // arguments. The implementation recognizes a hash value return of '0' as
  // "no effect on the hash".
  return 0;
}

struct KeyPathGenericWitnessTable {
  void *destroy;
  LANGUAGE_CC(language) void (* __ptrauth_language_runtime_function_entry_with_key(language::SpecialPointerAuthDiscriminators::KeyPathCopy) copy)(const void *src, void *dest, size_t bytes);
  LANGUAGE_CC(language) bool (* __ptrauth_language_runtime_function_entry_with_key(language::SpecialPointerAuthDiscriminators::KeyPathEquals) equals)(const void *, const void *, size_t);
  LANGUAGE_CC(language) intptr_t (* __ptrauth_language_runtime_function_entry_with_key(language::SpecialPointerAuthDiscriminators::KeyPathHash) hash)(const void *src, size_t bytes);
};

/// A prefab witness table for computed key path components that only include
/// captured generic arguments.
LANGUAGE_RUNTIME_EXPORT
KeyPathGenericWitnessTable language_keyPathGenericWitnessTable = {
  nullptr,
  language_copyKeyPathTrivialIndices,
  equateGenericArguments,
  hashGenericArguments,
};

/****************************************************************************/
/** Projection functions ****************************************************/
/****************************************************************************/

namespace {
  struct AddrAndOwner {
    OpaqueValue *Addr;
    HeapObject *Owner;
  };
}

// These functions are all implemented in the stdlib.  Their type
// parameters are passed implicitly in the isa of the key path.

extern "C"
LANGUAGE_CC(language) void
language_getAtKeyPath(LANGUAGE_INDIRECT_RESULT void *result,
                   const OpaqueValue *root, void *keyPath);

extern "C"
LANGUAGE_CC(language) AddrAndOwner
_language_modifyAtWritableKeyPath_impl(OpaqueValue *root, void *keyPath);

extern "C"
LANGUAGE_CC(language) AddrAndOwner
_language_modifyAtReferenceWritableKeyPath_impl(const OpaqueValue *root,
                                             void *keyPath);

namespace {
  struct YieldOnceTemporary {
    const Metadata *Type;

    // Yield-once buffers can't be memcpy'ed, so it doesn't matter that
    // isValueInline() returns false for non-bitwise-takable types --- but
    // it doesn't hurt, either.
    ValueBuffer Buffer;

    YieldOnceTemporary(const Metadata *type) : Type(type) {}

    static OpaqueValue *allocateIn(const Metadata *type,
                                   YieldOnceBuffer *buffer) {
      auto *temp =
        ::new (reinterpret_cast<void*>(buffer)) YieldOnceTemporary(type);
      return type->allocateBufferIn(&temp->Buffer);
    }

    static void destroyAndDeallocateIn(YieldOnceBuffer *buffer) {
      auto *temp = reinterpret_cast<YieldOnceTemporary*>(buffer);
      temp->Type->vw_destroy(temp->Type->projectBufferFrom(&temp->Buffer));
      temp->Type->deallocateBufferIn(&temp->Buffer);
    }
  };

  static_assert(sizeof(YieldOnceTemporary) <= sizeof(YieldOnceBuffer) &&
                alignof(YieldOnceTemporary) <= alignof(YieldOnceBuffer),
                "temporary doesn't fit in a YieldOnceBuffer");
}

static LANGUAGE_CC(language)
void _destroy_temporary_continuation(YieldOnceBuffer *buffer, bool forUnwind) {
  YieldOnceTemporary::destroyAndDeallocateIn(buffer);
}

YieldOnceResult<const OpaqueValue*>
language::language_readAtKeyPath(YieldOnceBuffer *buffer,
                           const OpaqueValue *root, void *keyPath) {
  // The Value type parameter is passed in the class of the key path object.
  // KeyPath is a native class, so we can just load its metadata directly
  // even on ObjC-interop targets.
  const Metadata *keyPathType = static_cast<HeapObject*>(keyPath)->metadata;
  auto keyPathGenericArgs = keyPathType->getGenericArgs();
  const Metadata *valueTy = keyPathGenericArgs[1];

  // Allocate the buffer.
  auto result = YieldOnceTemporary::allocateIn(valueTy, buffer);

  // Read into the buffer.
  language_getAtKeyPath(result, root, keyPath);

  // Return a continuation that destroys the value in the buffer
  // and deallocates it.
  return { language_ptrauth_sign_opaque_read_resume_function(
             &_destroy_temporary_continuation, buffer),
           result };
}

static LANGUAGE_CC(language)
void _release_owner_continuation(YieldOnceBuffer *buffer, bool forUnwind) {
  language_unknownObjectRelease(buffer->Data[0]);
}

YieldOnceResult<OpaqueValue*>
language::language_modifyAtWritableKeyPath(YieldOnceBuffer *buffer,
                                     OpaqueValue *root, void *keyPath) {
  auto addrAndOwner =
    _language_modifyAtWritableKeyPath_impl(root, keyPath);
  buffer->Data[0] = addrAndOwner.Owner;

  return { language_ptrauth_sign_opaque_modify_resume_function(
             &_release_owner_continuation, buffer),
           addrAndOwner.Addr };
}

YieldOnceResult<OpaqueValue*>
language::language_modifyAtReferenceWritableKeyPath(YieldOnceBuffer *buffer,
                                              const OpaqueValue *root,
                                              void *keyPath) {
  auto addrAndOwner =
    _language_modifyAtReferenceWritableKeyPath_impl(root, keyPath);
  buffer->Data[0] = addrAndOwner.Owner;

  return { language_ptrauth_sign_opaque_modify_resume_function(
             &_release_owner_continuation, buffer),
           addrAndOwner.Addr };
}

namespace {
template <typename>
struct YieldOnceCoroutine;

/// A template which generates the type of the ramp function of a yield-once
/// coroutine.
template <typename ResultType, typename... ArgumentTypes>
struct YieldOnceCoroutine<ResultType(ArgumentTypes...)> {
  using type =
      LANGUAGE_CC(language) YieldOnceResult<ResultType>(YieldOnceBuffer *,
                                                  ArgumentTypes...);
};

static_assert(std::is_same_v<decltype(language_readAtKeyPath),
                             YieldOnceCoroutine<const OpaqueValue * (const OpaqueValue *, void *)>::type>);
static_assert(std::is_same_v<decltype(language_modifyAtWritableKeyPath),
                             YieldOnceCoroutine<OpaqueValue * (OpaqueValue *, void *)>::type>);
static_assert(std::is_same_v<decltype(language_modifyAtReferenceWritableKeyPath),
                             YieldOnceCoroutine<OpaqueValue * (const OpaqueValue *, void *)>::type>);
}
