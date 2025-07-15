//===--- Builtins.h - Codira Builtin Functions -------------------*- C++ -*-===//
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
// This file defines the interface to builtin functions.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_BUILTINS_H
#define LANGUAGE_AST_BUILTINS_H

#include "language/AST/Type.h"
#include "language/AST/Types.h"
#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/IR/Attributes.h"
#include "toolchain/IR/Intrinsics.h"
#include "toolchain/Support/ErrorHandling.h"

namespace toolchain {
enum class AtomicOrdering : unsigned;
}

namespace language {

class ASTContext;
class Identifier;
class ValueDecl;

enum class BuiltinTypeKind : std::underlying_type<TypeKind>::type {
#define TYPE(id, parent)
#define BUILTIN_TYPE(id, parent)                                               \
  id = std::underlying_type<TypeKind>::type(TypeKind::id),
#include "language/AST/TypeNodes.def"
};

/// Get the builtin type for the given name.
///
/// Returns a null type if the name is not a known builtin type name.
Type getBuiltinType(ASTContext &Context, StringRef Name);

/// OverloadedBuiltinKind - Whether and how a builtin is overloaded.
enum class OverloadedBuiltinKind : uint8_t {
  /// The builtin is not overloaded.
  None,

  /// The builtin is overloaded over all integer types.
  Integer,

  /// The builtin is overloaded over all integer types and vectors of integers.
  IntegerOrVector,

  /// The builtin is overloaded over all integer types and the raw pointer type.
  IntegerOrRawPointer,

  /// The builtin is overloaded over all integer types, the raw pointer type,
  /// and vectors of integers.
  IntegerOrRawPointerOrVector,

  /// The builtin is overloaded over all floating-point types.
  Float,

  /// The builtin is overloaded over all floating-point types and vectors of
  /// floating-point types.
  FloatOrVector,

  /// The builtin has custom processing.
  Special
};

/// BuiltinValueKind - The set of (possibly overloaded) builtin functions.
enum class BuiltinValueKind {
  None = 0,
#define BUILTIN(Id, Name, Attrs) Id,
#include "language/AST/Builtins.def"
};

/// Returns true if this is a polymorphic builtin that is only valid
/// in raw sil and thus must be resolved to have concrete types by the
/// time we are in canonical SIL.
bool isPolymorphicBuiltin(BuiltinValueKind Id);

/// Decode the type list of a builtin (e.g. mul_Int32) and return the base
/// name (e.g. "mul").
StringRef getBuiltinBaseName(ASTContext &C, StringRef Name,
                             SmallVectorImpl<Type> &Types);

/// Given an LLVM IR intrinsic name with argument types remove (e.g. like
/// "bswap") return the LLVM IR IntrinsicID for the intrinsic or not_intrinsic
/// (0) if the intrinsic name doesn't match anything.
toolchain::Intrinsic::ID getLLVMIntrinsicID(StringRef Name);

/// Get the LLVM intrinsic ID that corresponds to the given builtin with
/// overflow.
toolchain::Intrinsic::ID
getLLVMIntrinsicIDForBuiltinWithOverflow(BuiltinValueKind ID);

/// Create a ValueDecl for the builtin with the given name.
///
/// Returns null if the name does not identifier a known builtin value.
ValueDecl *getBuiltinValueDecl(ASTContext &Context, Identifier Name);
  
/// Returns the name of a builtin declaration given a builtin ID.
StringRef getBuiltinName(BuiltinValueKind ID);
  
/// The information identifying the builtin - its kind and types.
class BuiltinInfo {
public:
  BuiltinValueKind ID;
  SmallVector<Type, 4> Types;
  bool isReadNone() const;
};

/// The information identifying the toolchain intrinsic - its id and types.
class IntrinsicInfo {
  mutable toolchain::AttributeList Attrs =
      toolchain::DenseMapInfo<toolchain::AttributeList>::getEmptyKey();
public:
  toolchain::Intrinsic::ID ID;
  SmallVector<Type, 4> Types;
  const toolchain::AttributeList &getOrCreateAttributes(ASTContext &Ctx) const;
};

/// Turn a string like "release" into the LLVM enum.
toolchain::AtomicOrdering decodeLLVMAtomicOrdering(StringRef O);

/// Returns true if the builtin with ID \p ID has a defined static overload for
/// the type \p Ty.
bool canBuiltinBeOverloadedForType(BuiltinValueKind ID, Type Ty);

/// Retrieve the AST-level AsyncTaskAndContext type, used for the
/// createAsyncTask* builtins.
Type getAsyncTaskAndContextType(ASTContext &ctx);

}

#endif
