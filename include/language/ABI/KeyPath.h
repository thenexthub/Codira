//===--- KeyPath.h - ABI constants for key path objects ---------*- C++ -*-===//
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
//  Constants used in the layout of key path objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_ABI_KEYPATH_H
#define LANGUAGE_ABI_KEYPATH_H

// We include the basic constants in a shim header so that it can be shared with
// the Codira implementation in the standard library.

#include <cstdint>
#include <cassert>
#include "language/shims/KeyPath.h"

namespace language {

/// Header layout for a key path's data buffer header.
class KeyPathBufferHeader {
  uint32_t Data;
  
  constexpr KeyPathBufferHeader(unsigned Data) : Data(Data) {}
  
  static constexpr uint32_t validateSize(uint32_t size) {
    return assert(size <= _CodiraKeyPathBufferHeader_SizeMask
                  && "size too big!"),
           size;
  }
public:
  constexpr KeyPathBufferHeader(unsigned size,
                                bool trivialOrInstantiableInPlace,
                                bool hasReferencePrefix)
    : Data((validateSize(size) & _CodiraKeyPathBufferHeader_SizeMask)
           | (trivialOrInstantiableInPlace ? _CodiraKeyPathBufferHeader_TrivialFlag : 0)
           | (hasReferencePrefix ? _CodiraKeyPathBufferHeader_HasReferencePrefixFlag : 0)) 
  {
  }
  
  constexpr KeyPathBufferHeader withSize(unsigned size) const {
    return (Data & ~_CodiraKeyPathBufferHeader_SizeMask) | validateSize(size);
  }
  
  constexpr KeyPathBufferHeader withIsTrivial(bool isTrivial) const {
    return (Data & ~_CodiraKeyPathBufferHeader_TrivialFlag)
      | (isTrivial ? _CodiraKeyPathBufferHeader_TrivialFlag : 0);
  }
  constexpr KeyPathBufferHeader withIsInstantiableInPlace(bool isTrivial) const {
    return (Data & ~_CodiraKeyPathBufferHeader_TrivialFlag)
      | (isTrivial ? _CodiraKeyPathBufferHeader_TrivialFlag : 0);
  }

  constexpr KeyPathBufferHeader withHasReferencePrefix(bool hasPrefix) const {
    return (Data & ~_CodiraKeyPathBufferHeader_HasReferencePrefixFlag)
      | (hasPrefix ? _CodiraKeyPathBufferHeader_HasReferencePrefixFlag : 0);
  }

  constexpr uint32_t getData() const {
    return Data;
  }
};

/// Header layout for a key path component's header.
class KeyPathComponentHeader {
  uint32_t Data;
  
  constexpr KeyPathComponentHeader(unsigned Data) : Data(Data) {}

  static constexpr uint32_t validateInlineOffset(uint32_t offset) {
    return assert(offsetCanBeInline(offset)
                  && "offset too big!"),
           offset;
  }

  static constexpr uint32_t isLetBit(bool isLet) {
    return isLet ? 0 : _CodiraKeyPathComponentHeader_StoredMutableFlag;
  }

public:
  static constexpr bool offsetCanBeInline(unsigned offset) {
    return offset <= _CodiraKeyPathComponentHeader_MaximumOffsetPayload;
  }

  constexpr static KeyPathComponentHeader
  forStructComponentWithInlineOffset(bool isLet,
                                     unsigned offset) {
    return KeyPathComponentHeader(
      (_CodiraKeyPathComponentHeader_StructTag
       << _CodiraKeyPathComponentHeader_DiscriminatorShift)
      | validateInlineOffset(offset)
      | isLetBit(isLet));
  }
  
  constexpr static KeyPathComponentHeader
  forStructComponentWithOutOfLineOffset(bool isLet) {
    return KeyPathComponentHeader(
      (_CodiraKeyPathComponentHeader_StructTag
       << _CodiraKeyPathComponentHeader_DiscriminatorShift)
      | _CodiraKeyPathComponentHeader_OutOfLineOffsetPayload
      | isLetBit(isLet));
  }

  constexpr static KeyPathComponentHeader
  forStructComponentWithUnresolvedFieldOffset(bool isLet) {
    return KeyPathComponentHeader(
      (_CodiraKeyPathComponentHeader_StructTag
       << _CodiraKeyPathComponentHeader_DiscriminatorShift)
      | _CodiraKeyPathComponentHeader_UnresolvedFieldOffsetPayload
      | isLetBit(isLet));
  }
  
  constexpr static KeyPathComponentHeader
  forClassComponentWithInlineOffset(bool isLet,
                                    unsigned offset) {
    return KeyPathComponentHeader(
      (_CodiraKeyPathComponentHeader_ClassTag
       << _CodiraKeyPathComponentHeader_DiscriminatorShift)
      | validateInlineOffset(offset)
      | isLetBit(isLet));
  }

  constexpr static KeyPathComponentHeader
  forClassComponentWithOutOfLineOffset(bool isLet) {
    return KeyPathComponentHeader(
      (_CodiraKeyPathComponentHeader_ClassTag
       << _CodiraKeyPathComponentHeader_DiscriminatorShift)
      | _CodiraKeyPathComponentHeader_OutOfLineOffsetPayload
      | isLetBit(isLet));
  }
  
  constexpr static KeyPathComponentHeader
  forClassComponentWithUnresolvedFieldOffset(bool isLet) {
    return KeyPathComponentHeader(
      (_CodiraKeyPathComponentHeader_ClassTag
       << _CodiraKeyPathComponentHeader_DiscriminatorShift)
      | _CodiraKeyPathComponentHeader_UnresolvedFieldOffsetPayload
      | isLetBit(isLet));
  }
  
  constexpr static KeyPathComponentHeader
  forClassComponentWithUnresolvedIndirectOffset(bool isLet) {
    return KeyPathComponentHeader(
      (_CodiraKeyPathComponentHeader_ClassTag
       << _CodiraKeyPathComponentHeader_DiscriminatorShift)
      | _CodiraKeyPathComponentHeader_UnresolvedIndirectOffsetPayload
      | isLetBit(isLet));
  }
  
  constexpr static KeyPathComponentHeader
  forOptionalChain() {
    return KeyPathComponentHeader(
      (_CodiraKeyPathComponentHeader_OptionalTag
      << _CodiraKeyPathComponentHeader_DiscriminatorShift)
      | _CodiraKeyPathComponentHeader_OptionalChainPayload);
  }
  constexpr static KeyPathComponentHeader
  forOptionalWrap() {
    return KeyPathComponentHeader(
      (_CodiraKeyPathComponentHeader_OptionalTag
      << _CodiraKeyPathComponentHeader_DiscriminatorShift)
      | _CodiraKeyPathComponentHeader_OptionalWrapPayload);
  }
  constexpr static KeyPathComponentHeader
  forOptionalForce() {
    return KeyPathComponentHeader(
      (_CodiraKeyPathComponentHeader_OptionalTag
      << _CodiraKeyPathComponentHeader_DiscriminatorShift)
      | _CodiraKeyPathComponentHeader_OptionalForcePayload);
  }
  
  enum ComputedPropertyKind {
    GetOnly,
    SettableNonmutating,
    SettableMutating,
  };
  
  enum ComputedPropertyIDKind {
    Pointer,
    StoredPropertyIndex,
    VTableOffset,
  };
  
  enum ComputedPropertyIDResolution {
    Resolved,
    ResolvedAbsolute,
    IndirectPointer,
    FunctionCall,
  };
  
  constexpr static KeyPathComponentHeader
  forComputedProperty(ComputedPropertyKind kind,
                      ComputedPropertyIDKind idKind,
                      bool hasArguments,
                      ComputedPropertyIDResolution resolution) {
    return KeyPathComponentHeader(
      (_CodiraKeyPathComponentHeader_ComputedTag
        << _CodiraKeyPathComponentHeader_DiscriminatorShift)
      | (kind != GetOnly
           ? _CodiraKeyPathComponentHeader_ComputedSettableFlag : 0)
      | (kind == SettableMutating
           ? _CodiraKeyPathComponentHeader_ComputedMutatingFlag : 0)
      | (idKind == StoredPropertyIndex
           ? _CodiraKeyPathComponentHeader_ComputedIDByStoredPropertyFlag : 0)
      | (idKind == VTableOffset
           ? _CodiraKeyPathComponentHeader_ComputedIDByVTableOffsetFlag : 0)
      | (hasArguments ? _CodiraKeyPathComponentHeader_ComputedHasArgumentsFlag : 0)
      | (resolution == Resolved ? _CodiraKeyPathComponentHeader_ComputedIDResolved
       : resolution == ResolvedAbsolute ? _CodiraKeyPathComponentHeader_ComputedIDResolvedAbsolute
       : resolution == IndirectPointer ? _CodiraKeyPathComponentHeader_ComputedIDUnresolvedIndirectPointer
       : resolution == FunctionCall ? _CodiraKeyPathComponentHeader_ComputedIDUnresolvedFunctionCall
       : (assert(false && "invalid resolution"), 0)));
  }
  
  constexpr static KeyPathComponentHeader
  forExternalComponent(unsigned numSubstitutions) {
    return assert(numSubstitutions <
        (1u << _CodiraKeyPathComponentHeader_DiscriminatorShift) - 1u
        && "too many substitutions"),
      KeyPathComponentHeader(
        (_CodiraKeyPathComponentHeader_ExternalTag
          << _CodiraKeyPathComponentHeader_DiscriminatorShift)
        | numSubstitutions);
  }
  
  constexpr uint32_t getData() const { return Data; }
};

}

#endif // LANGUAGE_ABI_KEYPATH_H
