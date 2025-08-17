//===-- Address.h - An aligned address -------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// This class provides a simple wrapper for a pair of a pointer and an
// alignment.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CODEGEN_ADDRESS_H
#define LANGUAGE_CORE_LIB_CODEGEN_ADDRESS_H

#include "CGPointerAuthInfo.h"
#include "language/Core/AST/CharUnits.h"
#include "language/Core/AST/Type.h"
#include "toolchain/ADT/PointerIntPair.h"
#include "toolchain/IR/Constants.h"
#include "toolchain/Support/MathExtras.h"

namespace language::Core {
namespace CodeGen {

class Address;
class CGBuilderTy;
class CodeGenFunction;
class CodeGenModule;

// Indicates whether a pointer is known not to be null.
enum KnownNonNull_t { NotKnownNonNull, KnownNonNull };

/// An abstract representation of an aligned address. This is designed to be an
/// IR-level abstraction, carrying just the information necessary to perform IR
/// operations on an address like loads and stores.  In particular, it doesn't
/// carry C type information or allow the representation of things like
/// bit-fields; clients working at that level should generally be using
/// `LValue`.
/// The pointer contained in this class is known to be unsigned.
class RawAddress {
  toolchain::PointerIntPair<toolchain::Value *, 1, bool> PointerAndKnownNonNull;
  toolchain::Type *ElementType;
  CharUnits Alignment;

protected:
  RawAddress(std::nullptr_t) : ElementType(nullptr) {}

public:
  RawAddress(toolchain::Value *Pointer, toolchain::Type *ElementType, CharUnits Alignment,
             KnownNonNull_t IsKnownNonNull = NotKnownNonNull)
      : PointerAndKnownNonNull(Pointer, IsKnownNonNull),
        ElementType(ElementType), Alignment(Alignment) {
    assert(Pointer != nullptr && "Pointer cannot be null");
    assert(ElementType != nullptr && "Element type cannot be null");
  }

  inline RawAddress(Address Addr);

  static RawAddress invalid() { return RawAddress(nullptr); }
  bool isValid() const {
    return PointerAndKnownNonNull.getPointer() != nullptr;
  }

  toolchain::Value *getPointer() const {
    assert(isValid());
    return PointerAndKnownNonNull.getPointer();
  }

  /// Return the type of the pointer value.
  toolchain::PointerType *getType() const {
    return toolchain::cast<toolchain::PointerType>(getPointer()->getType());
  }

  /// Return the type of the values stored in this address.
  toolchain::Type *getElementType() const {
    assert(isValid());
    return ElementType;
  }

  /// Return the address space that this address resides in.
  unsigned getAddressSpace() const {
    return getType()->getAddressSpace();
  }

  /// Return the IR name of the pointer value.
  toolchain::StringRef getName() const {
    return getPointer()->getName();
  }

  /// Return the alignment of this pointer.
  CharUnits getAlignment() const {
    assert(isValid());
    return Alignment;
  }

  /// Return address with different element type, but same pointer and
  /// alignment.
  RawAddress withElementType(toolchain::Type *ElemTy) const {
    return RawAddress(getPointer(), ElemTy, getAlignment(), isKnownNonNull());
  }

  KnownNonNull_t isKnownNonNull() const {
    assert(isValid());
    return (KnownNonNull_t)PointerAndKnownNonNull.getInt();
  }
};

/// Like RawAddress, an abstract representation of an aligned address, but the
/// pointer contained in this class is possibly signed.
///
/// This is designed to be an IR-level abstraction, carrying just the
/// information necessary to perform IR operations on an address like loads and
/// stores.  In particular, it doesn't carry C type information or allow the
/// representation of things like bit-fields; clients working at that level
/// should generally be using `LValue`.
///
/// An address may be either *raw*, meaning that it's an ordinary machine
/// pointer, or *signed*, meaning that the pointer carries an embedded
/// pointer-authentication signature. Representing signed pointers directly in
/// this abstraction allows the authentication to be delayed as long as possible
/// without forcing IRGen to use totally different code paths for signed and
/// unsigned values or to separately propagate signature information through
/// every API that manipulates addresses. Pointer arithmetic on signed addresses
/// (e.g. drilling down to a struct field) is accumulated into a separate offset
/// which is applied when the address is finally accessed.
class Address {
  friend class CGBuilderTy;

  // The boolean flag indicates whether the pointer is known to be non-null.
  toolchain::PointerIntPair<toolchain::Value *, 1, bool> Pointer;

  /// The expected IR type of the pointer. Carrying accurate element type
  /// information in Address makes it more convenient to work with Address
  /// values and allows frontend assertions to catch simple mistakes.
  toolchain::Type *ElementType = nullptr;

  CharUnits Alignment;

  /// The ptrauth information needed to authenticate the base pointer.
  CGPointerAuthInfo PtrAuthInfo;

  /// Offset from the base pointer. This is non-null only when the base
  /// pointer is signed.
  toolchain::Value *Offset = nullptr;

  toolchain::Value *emitRawPointerSlow(CodeGenFunction &CGF) const;

protected:
  Address(std::nullptr_t) : ElementType(nullptr) {}

public:
  Address(toolchain::Value *pointer, toolchain::Type *elementType, CharUnits alignment,
          KnownNonNull_t IsKnownNonNull = NotKnownNonNull)
      : Pointer(pointer, IsKnownNonNull), ElementType(elementType),
        Alignment(alignment) {
    assert(pointer != nullptr && "Pointer cannot be null");
    assert(elementType != nullptr && "Element type cannot be null");
    assert(!alignment.isZero() && "Alignment cannot be zero");
  }

  Address(toolchain::Value *BasePtr, toolchain::Type *ElementType, CharUnits Alignment,
          CGPointerAuthInfo PtrAuthInfo, toolchain::Value *Offset,
          KnownNonNull_t IsKnownNonNull = NotKnownNonNull)
      : Pointer(BasePtr, IsKnownNonNull), ElementType(ElementType),
        Alignment(Alignment), PtrAuthInfo(PtrAuthInfo), Offset(Offset) {}

  Address(RawAddress RawAddr)
      : Pointer(RawAddr.isValid() ? RawAddr.getPointer() : nullptr,
                RawAddr.isValid() ? RawAddr.isKnownNonNull() : NotKnownNonNull),
        ElementType(RawAddr.isValid() ? RawAddr.getElementType() : nullptr),
        Alignment(RawAddr.isValid() ? RawAddr.getAlignment()
                                    : CharUnits::Zero()) {}

  static Address invalid() { return Address(nullptr); }
  bool isValid() const { return Pointer.getPointer() != nullptr; }

  toolchain::Value *getPointerIfNotSigned() const {
    assert(isValid() && "pointer isn't valid");
    return !isSigned() ? Pointer.getPointer() : nullptr;
  }

  /// This function is used in situations where the caller is doing some sort of
  /// opaque "laundering" of the pointer.
  void replaceBasePointer(toolchain::Value *P) {
    assert(isValid() && "pointer isn't valid");
    assert(P->getType() == Pointer.getPointer()->getType() &&
           "Pointer's type changed");
    Pointer.setPointer(P);
    assert(isValid() && "pointer is invalid after replacement");
  }

  CharUnits getAlignment() const { return Alignment; }

  void setAlignment(CharUnits Value) { Alignment = Value; }

  toolchain::Value *getBasePointer() const {
    assert(isValid() && "pointer isn't valid");
    return Pointer.getPointer();
  }

  /// Return the type of the pointer value.
  toolchain::PointerType *getType() const {
    return toolchain::cast<toolchain::PointerType>(Pointer.getPointer()->getType());
  }

  /// Return the type of the values stored in this address.
  toolchain::Type *getElementType() const {
    assert(isValid());
    return ElementType;
  }

  /// Return the address space that this address resides in.
  unsigned getAddressSpace() const { return getType()->getAddressSpace(); }

  /// Return the IR name of the pointer value.
  toolchain::StringRef getName() const { return Pointer.getPointer()->getName(); }

  const CGPointerAuthInfo &getPointerAuthInfo() const { return PtrAuthInfo; }
  void setPointerAuthInfo(const CGPointerAuthInfo &Info) { PtrAuthInfo = Info; }

  // This function is called only in CGBuilderBaseTy::CreateElementBitCast.
  void setElementType(toolchain::Type *Ty) {
    assert(hasOffset() &&
           "this funcion shouldn't be called when there is no offset");
    ElementType = Ty;
  }

  bool isSigned() const { return PtrAuthInfo.isSigned(); }

  /// Whether the pointer is known not to be null.
  KnownNonNull_t isKnownNonNull() const {
    assert(isValid());
    return (KnownNonNull_t)Pointer.getInt();
  }

  Address setKnownNonNull() {
    assert(isValid());
    Pointer.setInt(KnownNonNull);
    return *this;
  }

  bool hasOffset() const { return Offset; }

  toolchain::Value *getOffset() const { return Offset; }

  Address getResignedAddress(const CGPointerAuthInfo &NewInfo,
                             CodeGenFunction &CGF) const;

  /// Return the pointer contained in this class after authenticating it and
  /// adding offset to it if necessary.
  toolchain::Value *emitRawPointer(CodeGenFunction &CGF) const {
    if (!isSigned())
      return getBasePointer();
    return emitRawPointerSlow(CGF);
  }

  /// Return address with different pointer, but same element type and
  /// alignment.
  Address withPointer(toolchain::Value *NewPointer,
                      KnownNonNull_t IsKnownNonNull) const {
    return Address(NewPointer, getElementType(), getAlignment(),
                   IsKnownNonNull);
  }

  /// Return address with different alignment, but same pointer and element
  /// type.
  Address withAlignment(CharUnits NewAlignment) const {
    return Address(Pointer.getPointer(), getElementType(), NewAlignment,
                   isKnownNonNull());
  }

  /// Return address with different element type, but same pointer and
  /// alignment.
  Address withElementType(toolchain::Type *ElemTy) const {
    if (!hasOffset())
      return Address(getBasePointer(), ElemTy, getAlignment(),
                     getPointerAuthInfo(), /*Offset=*/nullptr,
                     isKnownNonNull());
    Address A(*this);
    A.ElementType = ElemTy;
    return A;
  }
};

inline RawAddress::RawAddress(Address Addr)
    : PointerAndKnownNonNull(Addr.isValid() ? Addr.getBasePointer() : nullptr,
                             Addr.isValid() ? Addr.isKnownNonNull()
                                            : NotKnownNonNull),
      ElementType(Addr.isValid() ? Addr.getElementType() : nullptr),
      Alignment(Addr.isValid() ? Addr.getAlignment() : CharUnits::Zero()) {}

/// A specialization of Address that requires the address to be an
/// LLVM Constant.
class ConstantAddress : public RawAddress {
  ConstantAddress(std::nullptr_t) : RawAddress(nullptr) {}

public:
  ConstantAddress(toolchain::Constant *pointer, toolchain::Type *elementType,
                  CharUnits alignment)
      : RawAddress(pointer, elementType, alignment) {}

  static ConstantAddress invalid() {
    return ConstantAddress(nullptr);
  }

  toolchain::Constant *getPointer() const {
    return toolchain::cast<toolchain::Constant>(RawAddress::getPointer());
  }

  ConstantAddress withElementType(toolchain::Type *ElemTy) const {
    return ConstantAddress(getPointer(), ElemTy, getAlignment());
  }

  static bool isaImpl(RawAddress addr) {
    return toolchain::isa<toolchain::Constant>(addr.getPointer());
  }
  static ConstantAddress castImpl(RawAddress addr) {
    return ConstantAddress(toolchain::cast<toolchain::Constant>(addr.getPointer()),
                           addr.getElementType(), addr.getAlignment());
  }
};
}

// Present a minimal LLVM-like casting interface.
template <class U> inline U cast(CodeGen::Address addr) {
  return U::castImpl(addr);
}
template <class U> inline bool isa(CodeGen::Address addr) {
  return U::isaImpl(addr);
}

}

#endif
