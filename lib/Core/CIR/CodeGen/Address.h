//===----------------------------------------------------------------------===//
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

#ifndef CLANG_LIB_CIR_ADDRESS_H
#define CLANG_LIB_CIR_ADDRESS_H

#include "mlir/IR/Value.h"
#include "language/Core/AST/CharUnits.h"
#include "language/Core/CIR/Dialect/IR/CIRTypes.h"
#include "toolchain/ADT/PointerIntPair.h"

namespace language::Core::CIRGen {

// Forward declaration to avoid a circular dependency
class CIRGenBuilderTy;

class Address {

  // The boolean flag indicates whether the pointer is known to be non-null.
  toolchain::PointerIntPair<mlir::Value, 1, bool> pointerAndKnownNonNull;

  /// The expected CIR type of the pointer. Carrying accurate element type
  /// information in Address makes it more convenient to work with Address
  /// values and allows frontend assertions to catch simple mistakes.
  mlir::Type elementType;

  language::Core::CharUnits alignment;

protected:
  Address(std::nullptr_t) : elementType(nullptr) {}

public:
  Address(mlir::Value pointer, mlir::Type elementType,
          language::Core::CharUnits alignment)
      : pointerAndKnownNonNull(pointer, false), elementType(elementType),
        alignment(alignment) {
    assert(mlir::isa<cir::PointerType>(pointer.getType()) &&
           "Expected cir.ptr type");

    assert(pointer && "Pointer cannot be null");
    assert(elementType && "Element type cannot be null");
    assert(!alignment.isZero() && "Alignment cannot be zero");

    assert(mlir::cast<cir::PointerType>(pointer.getType()).getPointee() ==
           elementType);
  }

  Address(mlir::Value pointer, language::Core::CharUnits alignment)
      : Address(pointer,
                mlir::cast<cir::PointerType>(pointer.getType()).getPointee(),
                alignment) {
    assert((!alignment.isZero() || pointer == nullptr) &&
           "creating valid address with invalid alignment");
  }

  static Address invalid() { return Address(nullptr); }
  bool isValid() const {
    return pointerAndKnownNonNull.getPointer() != nullptr;
  }

  /// Return address with different element type, a bitcast pointer, and
  /// the same alignment.
  Address withElementType(CIRGenBuilderTy &builder, mlir::Type ElemTy) const;

  mlir::Value getPointer() const {
    assert(isValid());
    return pointerAndKnownNonNull.getPointer();
  }

  mlir::Value getBasePointer() const {
    // TODO(cir): Remove the version above when we catchup with OG codegen on
    // ptr auth.
    assert(isValid() && "pointer isn't valid");
    return getPointer();
  }

  mlir::Type getType() const {
    assert(mlir::cast<cir::PointerType>(
               pointerAndKnownNonNull.getPointer().getType())
               .getPointee() == elementType);

    return mlir::cast<cir::PointerType>(getPointer().getType());
  }

  mlir::Type getElementType() const {
    assert(isValid());
    assert(mlir::cast<cir::PointerType>(
               pointerAndKnownNonNull.getPointer().getType())
               .getPointee() == elementType);
    return elementType;
  }

  language::Core::CharUnits getAlignment() const { return alignment; }

  /// Get the operation which defines this address.
  mlir::Operation *getDefiningOp() const {
    if (!isValid())
      return nullptr;
    return getPointer().getDefiningOp();
  }

  template <typename OpTy> OpTy getDefiningOp() const {
    return mlir::dyn_cast_or_null<OpTy>(getDefiningOp());
  }
};

} // namespace language::Core::CIRGen

#endif // CLANG_LIB_CIR_ADDRESS_H
