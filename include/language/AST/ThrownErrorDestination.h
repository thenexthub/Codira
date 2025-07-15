//===--- ThrownErrorDestination.h - Thrown error dest -----------*- C++ -*-===//
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
// This file defines the ThrownErrorDestination class.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_THROWNERRORDESTINATION
#define LANGUAGE_AST_THROWNERRORDESTINATION

#include "language/AST/Type.h"
#include "language/AST/TypeAlignments.h"
#include "toolchain/ADT/PointerUnion.h"
#include <optional>

namespace language {
class Expr;
class OpaqueValueExpr;

/// Describes an error thrown from a particular operation and its conversion
/// to the error type expected by its context.
class ThrownErrorDestination {
  struct Conversion {
    /// The opaque value, which is of the thrown error type.
    OpaqueValueExpr *thrownError;

    /// The conversion from the opaque value type to the type needed by the
    /// context.
    Expr *conversion;
  };

  toolchain::PointerUnion<TypeBase *, Conversion *> storage;

  ThrownErrorDestination(Type type) : storage(type.getPointer()) { }
  ThrownErrorDestination(Conversion *conversion) : storage(conversion) { }

public:
  /// Construct a non-throwing destination.
  ThrownErrorDestination() : storage(nullptr) { }

  /// Construct a non-throwing destination.
  ThrownErrorDestination(std::nullptr_t) : storage(nullptr) { }

  /// Whether there is a thrown error destination at all.
  explicit operator bool() const { return !storage.isNull(); }

  /// A thrown error destination where the thrown type corresponds to the
  /// type caught (or rethrows) by the enclosing context.
  static ThrownErrorDestination forMatchingContextType(Type thrownError) {
    return ThrownErrorDestination(thrownError);
  }

  /// A thrown error destination where the thrown error requires a conversion
  /// to the error type expected by its context.
  static ThrownErrorDestination forConversion(OpaqueValueExpr *thrownError,
                                              Expr *conversion);

  /// Retrieve the type of error thrown from this function call.
  Type getThrownErrorType() const;

  /// Retrieve the type of the error expected in this context.
  Type getContextErrorType() const;

  /// Retrieve the conversion as a pair of (opaque thrown error value,
  /// conversion expression), when a conversion from the thrown error type
  /// to the context error type is required.
  std::optional<std::pair<OpaqueValueExpr *, Expr *>> getConversion() const {
    if (auto conversion = storage.dyn_cast<Conversion *>()) {
      return std::make_pair(conversion->thrownError, conversion->conversion);
    }

    return std::nullopt;
  }
};

} // end namespace language

#endif // LANGUAGE_AST_THROWNERRORDESTINATION
